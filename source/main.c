
/*
 *   Source File [main.c]
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef struct {
    f32 r, g, b, a;
} Color4f;

typedef struct {
    f32 x, y;
} Vec2f;

typedef struct {
    f32 x, y, z;
} Vec3f;

typedef struct {
    Color4f color;
    Vec3f pos;
    Vec2f uv;
} Vertex;

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

// On in debug builds, off in release builds (which define NDEBUG).
#ifdef NDEBUG
#define ENABLE_VALIDATION false
#else
#define ENABLE_VALIDATION true
#endif

#define MAX_INSTANCE_LAYERS 64
#define MAX_PHYSICAL_DEVICES 8
#define MAX_QUEUE_FAMILIES 32
#define MAX_SURFACE_FORMATS 64
#define MAX_SWAPCHAIN_IMAGES 8

static const wchar_t WINDOW_CLASS_NAME[] = L"WINDOW CLASS NAME";
static const wchar_t WINDOW_TITLE[] = L"WINDOW TITLE";
static const int SCREEN_WIDTH = 1200;
static const int SCREEN_HEIGHT = 700;

// Positions are in Vulkan clip space (x right, y down), centered in the window.
static const Vertex TRIANGLE_VERTICES[3] = {
    { .color = { 1.0f, 0.0f, 0.0f, 1.0f }, .pos = {  0.0f, -0.5f, 0.0f }, .uv = { 0.5f, 0.0f } },  // top, red
    { .color = { 0.0f, 1.0f, 0.0f, 1.0f }, .pos = {  0.5f,  0.5f, 0.0f }, .uv = { 1.0f, 1.0f } },  // bottom right, green
    { .color = { 0.0f, 0.0f, 1.0f, 1.0f }, .pos = { -0.5f,  0.5f, 0.0f }, .uv = { 0.0f, 1.0f } },  // bottom left, blue
};

// Stand-in texture: a single white RGBA pixel, so the vertex colors show through
// unchanged. Real image data goes through the same create_mesh() call.
static const u8 WHITE_PIXEL[4] = { 255, 255, 255, 255 };

static HWND hwnd = NULL;
static HINSTANCE hinstance = NULL;

// Everything that exists once for the whole run of the program: the device, the
// window's swapchain, the one pipeline every mesh is drawn with, and the
// per-frame command buffer and sync objects.
typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;  // NULL unless validation is on
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    u32 queue_family;

    VkSurfaceFormatKHR surface_format;
    VkRenderPass render_pass;
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

    VkSwapchainKHR swapchain;
    VkExtent2D swapchain_extent;
    u32 swapchain_image_count;
    VkImageView swapchain_views[MAX_SWAPCHAIN_IMAGES];
    VkFramebuffer swapchain_framebuffers[MAX_SWAPCHAIN_IMAGES];
    VkSemaphore render_finished[MAX_SWAPCHAIN_IMAGES];
    bool swapchain_dirty;

    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
    VkFence frame_fence;

    // Set by clear_screen() for the frame being drawn.
    u32 frame_image_index;
    bool frame_active;
} State;

// One drawable thing: its vertices, its texture, and the descriptor set that
// points the shader at that texture. There can be any number of these.
typedef struct {
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    u32 vertex_count;

    VkImage texture_image;
    VkDeviceMemory texture_memory;
    VkImageView texture_view;
    VkSampler texture_sampler;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
} Mesh;

static State state = { .swapchain_dirty = true };

// The validation layer ships with the Vulkan SDK, not with GPU drivers, so it
// is missing on machines that only have a driver installed.
static bool validation_layer_available(void)
{
    VkLayerProperties layers[MAX_INSTANCE_LAYERS];
    u32 layer_count = ARRAY_COUNT(layers);

    VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, layers);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        fprintf(stderr, "vkEnumerateInstanceLayerProperties() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    for (u32 i = 0; i < layer_count; i++)
    {
        if (strcmp(layers[i].layerName, VALIDATION_LAYER_NAME) == 0)
            return true;
    }

    return false;
}

// Called by the validation layer for every warning or error it finds.
static VKAPI_ATTR VkBool32 VKAPI_CALL validation_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT types,
                                                          const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                          void *user_data)
{
    (void)types;
    (void)user_data;

    const char *label = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "error" : "warning";
    fprintf(stderr, "Vulkan validation %s: %s\n", label, data->pMessage);

    // VK_FALSE lets the call that triggered the message carry on.
    return VK_FALSE;
}

// With vkdebug, Vulkan checks every call for misuse and reports
// problems on stderr. It is slow, so it is meant for development builds.
static void create_instance(bool vkdebug)
{
    if (vkdebug && !validation_layer_available())
    {
        fprintf(stderr, "Validation is off: %s is not installed (it comes with the Vulkan SDK)\n", VALIDATION_LAYER_NAME);
        vkdebug = false;
    }

    const char *extensions[3] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

    u32 extension_count = 2;
    if (vkdebug)
        extensions[extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    static const char *const layers[] = {
        VALIDATION_LAYER_NAME,
    };

    VkDebugUtilsMessengerCreateInfoEXT messenger_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = validation_callback,
    };

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_0,
    };

    // Chaining messenger_info here also covers vkCreateInstance and
    // vkDestroyInstance themselves, which the messenger below can't see.
    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = vkdebug ? &messenger_info : NULL,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = vkdebug ? ARRAY_COUNT(layers) : 0,
        .ppEnabledLayerNames = layers,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
    };

    VkResult result = vkCreateInstance(&info, NULL, &state.instance);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateInstance() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    if (!vkdebug)
        return;

    // Extension functions aren't exported by the loader, so look this one up.
    PFN_vkCreateDebugUtilsMessengerEXT create_messenger =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state.instance, "vkCreateDebugUtilsMessengerEXT");

    result = create_messenger(state.instance, &messenger_info, NULL, &state.debug_messenger);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateDebugUtilsMessengerEXT() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

static void create_surface(void)
{
    VkWin32SurfaceCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = hinstance,
        .hwnd = hwnd,
    };

    VkResult result = vkCreateWin32SurfaceKHR(state.instance, &info, NULL, &state.surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateWin32SurfaceKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

// Picks a GPU with a queue family that can both draw and present to the window,
// preferring a discrete GPU when there is one.
static void pick_physical_device(void)
{
    VkPhysicalDevice devices[MAX_PHYSICAL_DEVICES];
    u32 device_count = ARRAY_COUNT(devices);

    VkResult result = vkEnumeratePhysicalDevices(state.instance, &device_count, devices);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        fprintf(stderr, "vkEnumeratePhysicalDevices() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    for (u32 i = 0; i < device_count; i++)
    {
        VkQueueFamilyProperties families[MAX_QUEUE_FAMILIES];
        u32 family_count = ARRAY_COUNT(families);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &family_count, families);

        for (u32 j = 0; j < family_count; j++)
        {
            VkBool32 can_present = VK_FALSE;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, state.surface, &can_present);
            if (result != VK_SUCCESS)
            {
                fprintf(stderr, "vkGetPhysicalDeviceSurfaceSupportKHR() Failed: %d\n", result);
                exit(EXIT_FAILURE);
            }

            if (!(families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !can_present)
                continue;

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(devices[i], &properties);

            if (state.physical_device == NULL || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                state.physical_device = devices[i];
                state.queue_family = j;
            }

            break;
        }
    }

    if (state.physical_device == NULL)
    {
        fprintf(stderr, "No Vulkan device can render to the window\n");
        exit(EXIT_FAILURE);
    }
}

static void create_device(void)
{
    static const char *const extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    f32 priority = 1.0f;

    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = state.queue_family,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    VkDeviceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = ARRAY_COUNT(extensions),
        .ppEnabledExtensionNames = extensions,
    };

    VkResult result = vkCreateDevice(state.physical_device, &info, NULL, &state.device);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateDevice() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(state.device, state.queue_family, 0, &state.queue);
}

static void pick_surface_format(void)
{
    VkSurfaceFormatKHR formats[MAX_SURFACE_FORMATS];
    u32 format_count = ARRAY_COUNT(formats);

    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical_device, state.surface, &format_count, formats);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    if (format_count == 0)
    {
        fprintf(stderr, "The window surface has no formats\n");
        exit(EXIT_FAILURE);
    }

    state.surface_format = formats[0];

    for (u32 i = 0; i < format_count; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            state.surface_format = formats[i];
            break;
        }
    }
}

static void create_render_pass(void)
{
    VkAttachmentDescription attachment = {
        .format = state.surface_format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
    };

    // Holds the layout transition until the acquired image is actually ours to
    // write (the submit waits on image_available at this same stage).
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkResult result = vkCreateRenderPass(state.device, &info, NULL, &state.render_pass);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateRenderPass() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

// Describes what shaders/shader.frag expects at set 0: one texture + sampler.
static void create_descriptor_set_layout(void)
{
    VkDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &binding,
    };

    VkResult result = vkCreateDescriptorSetLayout(state.device, &info, NULL, &state.descriptor_set_layout);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateDescriptorSetLayout() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

// The .spv files live in shaders/ next to the .exe (see the Makefile).
static VkShaderModule load_shader_module(const wchar_t *file_name)
{
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);

    if (length == 0 || length == MAX_PATH)
    {
        fprintf(stderr, "GetModuleFileNameW() Failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    wchar_t *slash = wcsrchr(path, L'\\');
    size_t directory_length = slash ? (size_t)(slash - path) + 1 : 0;

    if (directory_length + wcslen(file_name) >= MAX_PATH)
    {
        fprintf(stderr, "Shader path is too long: %ls\n", file_name);
        exit(EXIT_FAILURE);
    }

    wcscpy(path + directory_length, file_name);

    FILE *file = _wfopen(path, L"rb");

    if (!file)
    {
        fprintf(stderr, "Failed to open shader: %ls\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // SPIR-V is a stream of 32-bit words.
    if (size <= 0 || size % 4 != 0)
    {
        fprintf(stderr, "Shader is not valid SPIR-V: %ls\n", path);
        exit(EXIT_FAILURE);
    }

    u32 *code = malloc((size_t)size);

    if (!code || fread(code, 1, (size_t)size, file) != (size_t)size)
    {
        fprintf(stderr, "Failed to read shader: %ls\n", path);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = (size_t)size,
        .pCode = code,
    };

    VkShaderModule module;
    VkResult result = vkCreateShaderModule(state.device, &info, NULL, &module);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateShaderModule() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    free(code);
    return module;
}

static void create_pipeline(void)
{
    VkShaderModule vertex_module = load_shader_module(L"shaders\\vertex.spv");
    VkShaderModule fragment_module = load_shader_module(L"shaders\\fragment.spv");

    VkPipelineShaderStageCreateInfo stages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_module,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_module,
            .pName = "main",
        },
    };

    VkVertexInputBindingDescription binding = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    // Locations match the inputs of shaders/shader.vert.
    VkVertexInputAttributeDescription attributes[] = {
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(Vertex, color) },
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
        { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv) },
    };

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = ARRAY_COUNT(attributes),
        .pVertexAttributeDescriptions = attributes,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    // Viewport and scissor are dynamic, so resizing the window only needs a new
    // swapchain, not a new pipeline.
    VkPipelineViewportStateCreateInfo viewport = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterization = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisample = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blend = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blend_attachment,
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ARRAY_COUNT(dynamic_states),
        .pDynamicStates = dynamic_states,
    };

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &state.descriptor_set_layout,
    };

    VkResult result = vkCreatePipelineLayout(state.device, &layout_info, NULL, &state.pipeline_layout);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreatePipelineLayout() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkGraphicsPipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = ARRAY_COUNT(stages),
        .pStages = stages,
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport,
        .pRasterizationState = &rasterization,
        .pMultisampleState = &multisample,
        .pColorBlendState = &color_blend,
        .pDynamicState = &dynamic,
        .layout = state.pipeline_layout,
        .renderPass = state.render_pass,
        .subpass = 0,
    };

    result = vkCreateGraphicsPipelines(state.device, NULL, 1, &info, NULL, &state.pipeline);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateGraphicsPipelines() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(state.device, vertex_module, NULL);
    vkDestroyShaderModule(state.device, fragment_module, NULL);
}

static void create_frame_resources(void)
{
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = state.queue_family,
    };

    VkResult result = vkCreateCommandPool(state.device, &pool_info, NULL, &state.command_pool);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateCommandPool() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = state.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    result = vkAllocateCommandBuffers(state.device, &allocate_info, &state.command_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkAllocateCommandBuffers() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    result = vkCreateSemaphore(state.device, &semaphore_info, NULL, &state.image_available);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateSemaphore() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    // Starts signaled so the first frame doesn't wait on a frame that never ran.
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    result = vkCreateFence(state.device, &fence_info, NULL, &state.frame_fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateFence() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

static VkDeviceMemory allocate_memory(VkMemoryRequirements requirements, VkMemoryPropertyFlags wanted)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(state.physical_device, &memory_properties);

    u32 memory_type = UINT32_MAX;

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        bool allowed = requirements.memoryTypeBits & (1u << i);
        bool suitable = (memory_properties.memoryTypes[i].propertyFlags & wanted) == wanted;

        if (allowed && suitable)
        {
            memory_type = i;
            break;
        }
    }

    if (memory_type == UINT32_MAX)
    {
        fprintf(stderr, "No suitable memory type (wanted flags 0x%x)\n", (unsigned)wanted);
        exit(EXIT_FAILURE);
    }

    VkMemoryAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memory_type,
    };

    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory(state.device, &info, NULL, &memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkAllocateMemory() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    return memory;
}

// Creates a buffer in host-visible memory and fills it with a copy of data.
static void create_filled_buffer(const void *data, size_t size, VkBufferUsageFlags usage, VkBuffer *buffer, VkDeviceMemory *memory)
{
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkResult result = vkCreateBuffer(state.device, &info, NULL, buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateBuffer() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(state.device, *buffer, &requirements);

    *memory = allocate_memory(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    result = vkBindBufferMemory(state.device, *buffer, *memory, 0);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkBindBufferMemory() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    void *mapped;
    result = vkMapMemory(state.device, *memory, 0, size, 0, &mapped);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkMapMemory() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    memcpy(mapped, data, size);
    vkUnmapMemory(state.device, *memory);
}

static void transition_image_layout(VkCommandBuffer commands, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
                                    VkAccessFlags src_access, VkAccessFlags dst_access,
                                    VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage)
{
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier(commands, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

// Creates the texture the fragment shader samples from tightly packed 8-bit
// RGBA pixels (width * height * 4 bytes, rows top to bottom).
static void create_mesh_texture(Mesh *mesh, const u8 *pixels, u32 width, u32 height)
{
    // Color images (like PNGs) are sRGB encoded; sampling converts to linear.
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult result = vkCreateImage(state.device, &image_info, NULL, &mesh->texture_image);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateImage() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(state.device, mesh->texture_image, &requirements);

    mesh->texture_memory = allocate_memory(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkBindImageMemory(state.device, mesh->texture_image, mesh->texture_memory, 0);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkBindImageMemory() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    // Images can't be written directly: the pixels go into a staging buffer,
    // then the GPU copies that buffer into the image.
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    create_filled_buffer(pixels, (size_t)width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &staging_buffer, &staging_memory);

    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = state.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commands;
    result = vkAllocateCommandBuffers(state.device, &allocate_info, &commands);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkAllocateCommandBuffers() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    result = vkBeginCommandBuffer(commands, &begin_info);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkBeginCommandBuffer() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    transition_image_layout(commands, mesh->texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            0, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy region = {
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .imageExtent = { width, height, 1 },
    };

    vkCmdCopyBufferToImage(commands, staging_buffer, mesh->texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    transition_image_layout(commands, mesh->texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    result = vkEndCommandBuffer(commands);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkEndCommandBuffer() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commands,
    };

    result = vkQueueSubmit(state.queue, 1, &submit_info, NULL);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkQueueSubmit() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = vkQueueWaitIdle(state.queue);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkQueueWaitIdle() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    vkFreeCommandBuffers(state.device, state.command_pool, 1, &commands);
    vkDestroyBuffer(state.device, staging_buffer, NULL);
    vkFreeMemory(state.device, staging_memory, NULL);

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = mesh->texture_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    result = vkCreateImageView(state.device, &view_info, NULL, &mesh->texture_view);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateImageView() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    };

    result = vkCreateSampler(state.device, &sampler_info, NULL, &mesh->texture_sampler);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateSampler() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

// Points the fragment shader's sampler at the mesh's texture. Each mesh has its
// own single-set pool, so meshes can be created and destroyed independently.
static void create_mesh_descriptor_set(Mesh *mesh)
{
    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };

    VkResult result = vkCreateDescriptorPool(state.device, &pool_info, NULL, &mesh->descriptor_pool);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateDescriptorPool() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkDescriptorSetAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mesh->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &state.descriptor_set_layout,
    };

    result = vkAllocateDescriptorSets(state.device, &allocate_info, &mesh->descriptor_set);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkAllocateDescriptorSets() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkDescriptorImageInfo image_info = {
        .sampler = mesh->texture_sampler,
        .imageView = mesh->texture_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mesh->descriptor_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
    };

    vkUpdateDescriptorSets(state.device, 1, &write, 0, NULL);
}

// Pixels are tightly packed 8-bit RGBA, as for create_mesh_texture().
static void create_mesh(Mesh *mesh, const Vertex *vertices, u32 vertex_count, const u8 *pixels, u32 width, u32 height)
{
    mesh->vertex_count = vertex_count;
    create_filled_buffer(vertices, vertex_count * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &mesh->vertex_buffer, &mesh->vertex_memory);

    create_mesh_texture(mesh, pixels, width, height);
    create_mesh_descriptor_set(mesh);
}

// The GPU must be done with the mesh (see vkDeviceWaitIdle) before this is called.
static void destroy_mesh(Mesh *mesh)
{
    vkDestroyDescriptorPool(state.device, mesh->descriptor_pool, NULL);
    vkDestroySampler(state.device, mesh->texture_sampler, NULL);
    vkDestroyImageView(state.device, mesh->texture_view, NULL);
    vkDestroyImage(state.device, mesh->texture_image, NULL);
    vkFreeMemory(state.device, mesh->texture_memory, NULL);

    vkDestroyBuffer(state.device, mesh->vertex_buffer, NULL);
    vkFreeMemory(state.device, mesh->vertex_memory, NULL);

    *mesh = (Mesh){ 0 };
}

static void destroy_swapchain_resources(void)
{
    for (u32 i = 0; i < state.swapchain_image_count; i++)
    {
        vkDestroyFramebuffer(state.device, state.swapchain_framebuffers[i], NULL);
        vkDestroyImageView(state.device, state.swapchain_views[i], NULL);
        vkDestroySemaphore(state.device, state.render_finished[i], NULL);
    }

    state.swapchain_image_count = 0;
}

// (Re)creates the swapchain at the window's current size. Returns false when
// there is nothing to draw to (the window is minimized).
static bool create_swapchain(void)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physical_device, state.surface, &capabilities);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkExtent2D extent = capabilities.currentExtent;

    if (extent.width == UINT32_MAX)
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        extent.width = (u32)(rect.right - rect.left);
        extent.height = (u32)(rect.bottom - rect.top);
    }

    if (extent.width == 0 || extent.height == 0)
        return false;

    u32 image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 && image_count > capabilities.maxImageCount)
        image_count = capabilities.maxImageCount;

    result = vkDeviceWaitIdle(state.device);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkDeviceWaitIdle() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    destroy_swapchain_resources();

    VkSwapchainKHR old_swapchain = state.swapchain;

    VkSwapchainCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = state.surface,
        .minImageCount = image_count,
        .imageFormat = state.surface_format.format,
        .imageColorSpace = state.surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    };

    result = vkCreateSwapchainKHR(state.device, &info, NULL, &state.swapchain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateSwapchainKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    vkDestroySwapchainKHR(state.device, old_swapchain, NULL);

    VkImage images[MAX_SWAPCHAIN_IMAGES];
    state.swapchain_image_count = ARRAY_COUNT(images);
    result = vkGetSwapchainImagesKHR(state.device, state.swapchain, &state.swapchain_image_count, images);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkGetSwapchainImagesKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    state.swapchain_extent = extent;

    for (u32 i = 0; i < state.swapchain_image_count; i++)
    {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = state.surface_format.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        };

        result = vkCreateImageView(state.device, &view_info, NULL, &state.swapchain_views[i]);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateImageView() Failed: %d\n", result);
            exit(EXIT_FAILURE);
        }

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = state.render_pass,
            .attachmentCount = 1,
            .pAttachments = &state.swapchain_views[i],
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };

        result = vkCreateFramebuffer(state.device, &framebuffer_info, NULL, &state.swapchain_framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateFramebuffer() Failed: %d\n", result);
            exit(EXIT_FAILURE);
        }

        // One per image: a present can still hold its semaphore after the
        // frame's fence has signaled, so a single shared one isn't safe to reuse.
        VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        result = vkCreateSemaphore(state.device, &semaphore_info, NULL, &state.render_finished[i]);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "vkCreateSemaphore() Failed: %d\n", result);
            exit(EXIT_FAILURE);
        }
    }

    state.swapchain_dirty = false;
    return true;
}

// Starts a frame: acquires the next swapchain image and fills it with
// clear_color. Call once per frame, then draw_mesh() any number of times, then
// end_frame(). When there is nothing to draw to (the window is minimized, or the
// swapchain went out of date) no frame is started, and draw_mesh() and
// end_frame() do nothing until the next clear_screen().
static void clear_screen(Color4f clear_color)
{
    if (state.swapchain_dirty && !create_swapchain())
    {
        // Minimized: nothing to draw, so don't spin the CPU.
        Sleep(10);
        return;
    }

    VkResult result = vkWaitForFences(state.device, 1, &state.frame_fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkWaitForFences() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = vkAcquireNextImageKHR(state.device, state.swapchain, UINT64_MAX, state.image_available, NULL, &state.frame_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        state.swapchain_dirty = true;
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        fprintf(stderr, "vkAcquireNextImageKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = vkResetFences(state.device, 1, &state.frame_fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkResetFences() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    result = vkBeginCommandBuffer(state.command_buffer, &begin_info);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkBeginCommandBuffer() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    // The render pass clears its attachment on load, so the clear happens here.
    VkClearValue clear = { .color = { .float32 = { clear_color.r, clear_color.g, clear_color.b, clear_color.a } } };

    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = state.render_pass,
        .framebuffer = state.swapchain_framebuffers[state.frame_image_index],
        .renderArea = { .extent = state.swapchain_extent },
        .clearValueCount = 1,
        .pClearValues = &clear,
    };

    vkCmdBeginRenderPass(state.command_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(state.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);

    VkViewport viewport = {
        .width = (f32)state.swapchain_extent.width,
        .height = (f32)state.swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = { .extent = state.swapchain_extent };

    vkCmdSetViewport(state.command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(state.command_buffer, 0, 1, &scissor);

    state.frame_active = true;
}

// Draws one mesh into the frame started by clear_screen().
static void draw_mesh(const Mesh *mesh)
{
    if (!state.frame_active)
        return;

    vkCmdBindDescriptorSets(state.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline_layout, 0, 1, &mesh->descriptor_set, 0, NULL);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(state.command_buffer, 0, 1, &mesh->vertex_buffer, &offset);
    vkCmdDraw(state.command_buffer, mesh->vertex_count, 1, 0, 0);
}

// Finishes the frame started by clear_screen() and shows it in the window.
static void end_frame(void)
{
    if (!state.frame_active)
        return;

    state.frame_active = false;

    vkCmdEndRenderPass(state.command_buffer);
    VkResult result = vkEndCommandBuffer(state.command_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkEndCommandBuffer() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &state.image_available,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &state.command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &state.render_finished[state.frame_image_index],
    };

    result = vkQueueSubmit(state.queue, 1, &submit_info, state.frame_fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkQueueSubmit() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &state.render_finished[state.frame_image_index],
        .swapchainCount = 1,
        .pSwapchains = &state.swapchain,
        .pImageIndices = &state.frame_image_index,
    };

    result = vkQueuePresentKHR(state.queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        state.swapchain_dirty = true;
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkQueuePresentKHR() Failed: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

static void init_vulkan(bool vkdebug)
{
    create_instance(vkdebug);
    create_surface();
    pick_physical_device();
    create_device();
    pick_surface_format();
    create_render_pass();
    create_descriptor_set_layout();
    create_pipeline();
    create_frame_resources();
}

static void shutdown_vulkan(void)
{
    vkDeviceWaitIdle(state.device);

    destroy_swapchain_resources();
    vkDestroySwapchainKHR(state.device, state.swapchain, NULL);

    vkDestroyFence(state.device, state.frame_fence, NULL);
    vkDestroySemaphore(state.device, state.image_available, NULL);
    vkDestroyCommandPool(state.device, state.command_pool, NULL);

    vkDestroyPipeline(state.device, state.pipeline, NULL);
    vkDestroyPipelineLayout(state.device, state.pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(state.device, state.descriptor_set_layout, NULL);
    vkDestroyRenderPass(state.device, state.render_pass, NULL);

    vkDestroyDevice(state.device, NULL);
    vkDestroySurfaceKHR(state.instance, state.surface, NULL);

    if (state.debug_messenger != NULL)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state.instance, "vkDestroyDebugUtilsMessengerEXT");

        destroy_messenger(state.instance, state.debug_messenger, NULL);
    }

    vkDestroyInstance(state.instance, NULL);
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // The window has to outlive the Vulkan surface made from it, so closing only
    // ends the main loop; main() destroys the window after Vulkan shuts down.
    if (msg == WM_CLOSE)
    {
        PostQuitMessage(EXIT_SUCCESS);
        return 0;
    }

    if (msg == WM_SIZE)
    {
        state.swapchain_dirty = true;
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int main(void)
{
    //return __main__();

    hinstance = GetModuleHandleW(NULL);

    WNDCLASSEXW wc = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = window_proc,
        .hInstance = hinstance,
        .hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION),
        .hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW),
        .hbrBackground = NULL,
        .lpszClassName = WINDOW_CLASS_NAME,
    };

    if (!RegisterClassExW(&wc))
    {
        fprintf(stderr, "RegisterClassExW() Failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    DWORD dwStyle = WS_OVERLAPPEDWINDOW;

    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    hwnd = CreateWindowExW(0, WINDOW_CLASS_NAME, WINDOW_TITLE, dwStyle, x, y, width, height, NULL, NULL, hinstance, NULL);

    if (!hwnd)
    {
        fprintf(stderr, "CreateWindowExW() Failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    init_vulkan(ENABLE_VALIDATION);

    Mesh triangle;
    create_mesh(&triangle, TRIANGLE_VERTICES, ARRAY_COUNT(TRIANGLE_VERTICES), WHITE_PIXEL, 1, 1);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    bool running = true;

    Color4f background_color = { 0.5f, 0.0f, 0.5f, 1.0f };

    while (running)
    {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                running = false;

            DispatchMessageW(&msg);
        }

        if (running)
        {
            clear_screen(background_color);
            draw_mesh(&triangle);
            end_frame();
        }
    }

    // Meshes are made from the device, so they have to go before it does.
    vkDeviceWaitIdle(state.device);
    destroy_mesh(&triangle);

    shutdown_vulkan();
    DestroyWindow(hwnd);

    return EXIT_SUCCESS;
}

int __main__(void)
{

    return EXIT_SUCCESS;
}

