
/*
 *   Source File [main.c]
 */

#define __MAIN__
#include "rose.h"
#include "map.h"

static float scale = 2.0f;
static float xpos = 0.0f;
static float ypos = 0.0f;

static ROSE_Image image = { 0 };
static Provinces provinces = { 0 };

static Province* SelectProvince(void);
static void DrawProvinces(void);
static void DestroyProvinces(void);

int main(void)
{
	ROSE_Init("Van Gogh's Epic Odyssey", 1600, 900);
	ROSE_WindowVSync(true);

	MapInfo info = {
		.map_width          = 500,
		.map_sea_level      = 0.4f,
		.perlin_seed        = 0,
		.perlin_divisions   = 1,
		.perlin_layers      = 8,
		.perlin_attenuation = 0.8f,
		.province_spacing   = 15,
		.province_drift     = 100,
	};

	image = MAP_GenerateImage(&info);
	provinces = MAP_GenerateProvinces(&image);

	while (ROSE_WindowPollEvents())
	{
		if (ROSE_MousePressing(ROSE_BUTTON_MIDDLE)) {
			xpos += rose_mouse.xvel;
			ypos += rose_mouse.yvel;
		}

		float scroll = rose_mouse.scroll;
		if (scroll != 0.0f) {
			scroll = (scroll > 0.0f) ?
				(1.0f) :
				(-1.0f);
		}

		float prev_scale = scale;
		scale += scroll;
		if (scale <= 1.0f)
			scale = 1.0f;

		if (scale != prev_scale) {
			float ratio = scale / prev_scale;
			xpos = rose_mouse.xpos - (rose_mouse.xpos - xpos) * ratio;
			ypos = rose_mouse.ypos - (rose_mouse.ypos - ypos) * ratio;
		}

		Province* selection = SelectProvince();
		if (selection) {
			selection->triangles.color = ROSE_COLOR_RED;
		}
		

		ROSE_WindowBeginFrame(ROSE_COLOR_BLUE);
		DrawProvinces();
		ROSE_WindowEndFrame();
	}

	ROSE_ImageDestroy(&image);
	DestroyProvinces();
	ROSE_Quit();
	return EXIT_SUCCESS;
}

Province* SelectProvince(void)
{
	float x = (rose_mouse.xpos - xpos) / scale;
	float y = (rose_mouse.ypos - ypos) / scale;

	if ((x < 0.0f) || (y < 0.0f))
		return NULL;

	float width = (float)image._width;
	float height = (float)image._height;

	if ((x >= width) || (y >= height))
		return NULL;

	ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, (uint32_t)x, (uint32_t)y);

	const bool ocean =
		(color.r == 255) &&
		(color.g == 0) &&
		(color.b == 0);

	if (ocean)
		return NULL;

	Province* result = NULL;
	for (uint32_t i = 0; i < provinces.count; i++) {
		Province* province = &provinces.array[i];
		const bool found =
			(province->color.r == color.r) &&
			(province->color.g == color.g) &&
			(province->color.b == color.b);
		if (found) {
			result = province;
			break;
		}
	}

	return result;
}

void DrawProvinces(void)
{
	for (uint32_t i = 0; i < provinces.count; i++) {
		Province* province = &provinces.array[i];
		province->triangles.scale = scale;
		province->triangles.xpos = xpos;
		province->triangles.ypos = ypos;
		ROSE_MeshDraw(&province->triangles);
	}

	for (uint32_t i = 0; i < provinces.count; i++) {
		Province* province = &provinces.array[i];
		province->border.scale = scale;
		province->border.xpos = xpos;
		province->border.ypos = ypos;
		ROSE_MeshDraw(&province->border);
	}
}

void DestroyProvinces(void)
{
	for (uint32_t i = 0; i < provinces.count; i++) {
		Province* province = &provinces.array[i];
		free(province->neighbours);
		ROSE_MeshDestroy(&province->border);
		ROSE_MeshDestroy(&province->triangles);
	}

	free(provinces.array);
	provinces = (Provinces){ 0 };
}

