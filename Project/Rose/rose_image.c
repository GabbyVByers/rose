
/*
 *   Source File [rose_image.c]
 */

#include "rose.h"

typedef struct ROSE_Image {
	i32 width;
	i32 height;
	u8* pixels;
} ROSE_Image;

ROSE_Image* ROSE_ImageCreate(i32 width, i32 height) {
	if ((width <= 0) || (height <= 0)) {
		fprintf(stderr, "ROSE_ImageCreate() Failed\n");
		fprintf(stderr, "Invalid Dimensions: (%d, %d)\n", width, height);
		exit(EXIT_FAILURE);
	}

	const bool too_large = (usize)width > SIZE_MAX / (usize)height / 4;
	if (too_large) {
		fprintf(stderr, "ROSE_ImageCreate() Failed\n");
		fprintf(stderr, "Image Too Large\n");
		exit(EXIT_FAILURE);
	}

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	if (!image) {
		fprintf(stderr, "ROSE_ImageCreate() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	if (!pixels) {
		fprintf(stderr, "ROSE_ImageCreate() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	} memset(pixels, 255, buffer_size);

	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageDestroy(ROSE_Image* image) {
	if (!image) { return; }
	free(image->pixels);
	free(image);
}

ROSE_Image* ROSE_ImageLoadFromFile(const char* path) {
	if (!path) {
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "NULL Path\n");
		exit(EXIT_FAILURE);
	}

	int width, height, n;
	stbi_set_flip_vertically_on_load(true);
	u8* stbi_image = stbi_load(path, &width, &height, &n, 4);
	if (!stbi_image) {
		const char* reason = stbi_failure_reason();
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "STBI Couldn't Load: %s\n", path);
		fprintf(stderr, "STBI Failure Reason: %s \n", reason ? reason : "Unknown");
		exit(EXIT_FAILURE);
	}

	if ((width <= 0) || (height <= 0)) {
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "Invalid Dimensions: (%d, %d)\n", width, height);
		exit(EXIT_FAILURE);
	}

	const bool too_large = (usize)width > SIZE_MAX / (usize)height / 4;
	if (too_large) {
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "Image Too Large\n");
		exit(EXIT_FAILURE);
	}

	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	if (!pixels) {
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

	memcpy(pixels, stbi_image, buffer_size);
	stbi_image_free(stbi_image);
	
	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	if (!image) {
		fprintf(stderr, "ROSE_ImageLoadFromFile() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageSaveAsPNG(ROSE_Image* image, const char* name) {
	if (!image) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "NULL Image\n");
		exit(EXIT_FAILURE);
	}

	if (!image->pixels) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "NULL Pixels\n");
		exit(EXIT_FAILURE);
	}

	if (!name) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "NULL Name\n");
		exit(EXIT_FAILURE);
	}

	if ((image->width <= 0) || (image->height <= 0)) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "Invalid Dimensions: (%d, %d)\n", image->width, image->height);
		exit(EXIT_FAILURE);
	}

	const bool too_large = image->width > INT_MAX / 4;
	if (too_large) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "Stride Too Large\n");
		exit(EXIT_FAILURE);
	}

	i32 width = image->width;
	i32 height = image->height;
	i32 stride = image->width * 4;
	stbi_flip_vertically_on_write(true);
	i32 success = stbi_write_png(name, width, height, 4, image->pixels, stride);
	if (!success) {
		fprintf(stderr, "ROSE_ImageSaveAsPNG() Failed\n");
		fprintf(stderr, "STBI Couldn't Write: %s\n", name);
		exit(EXIT_FAILURE);
	}
}

void ROSE_ImageResize(ROSE_Image* image, i32 width, i32 height) {
	if (!image) {
		fprintf(stderr, "ROSE_ImageResize() Failed\n");
		fprintf(stderr, "NULL Image\n");
		exit(EXIT_FAILURE);
	}

	if (!image->pixels) {
		fprintf(stderr, "ROSE_ImageResize() Failed\n");
		fprintf(stderr, "NULL Pixels\n");
		exit(EXIT_FAILURE);
	}

	if ((width <= 0) || (height <= 0)) {
		fprintf(stderr, "ROSE_ImageResize() Failed\n");
		fprintf(stderr, "Invalid Dimensions: (%d, %d)\n", width, height);
		exit(EXIT_FAILURE);
	}

	const bool too_large = (usize)width > SIZE_MAX / (usize)height / 4;
	if (too_large) {
		fprintf(stderr, "ROSE_ImageResize() Failed\n");
		fprintf(stderr, "Image Too Large\n");
		exit(EXIT_FAILURE);
	}

	usize new_buffer_size = (usize)width * (usize)height * 4;
	u8* new_pixels = malloc(new_buffer_size);
	if (!new_pixels) {
		fprintf(stderr, "ROSE_ImageResize() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	} memset(new_pixels, 255, new_buffer_size);

	i32 copy_width = (width < image->width) ? width : image->width;
	i32 copy_height = (height < image->height) ? height : image->height;

	i32 src_offset = image->height - copy_height;
	i32 dst_offset = height - copy_height;

	for (i32 y = 0; y < copy_height; y++) {
		usize src_index = (usize)(src_offset + y) * (usize)image->width * 4;
		usize dst_index = (usize)(dst_offset + y) * (usize)width * 4;
		memcpy(new_pixels + dst_index, image->pixels + src_index, (usize)copy_width * 4);
	}

	free(image->pixels);
	image->width = width;
	image->height = height;
	image->pixels = new_pixels;
}

void ROSE_ImageGetDimensions(ROSE_Image* image, i32* width, i32* height) {
	if (!image) {
		fprintf(stderr, "ROSE_ImageGetDimensions() Failed\n");
		fprintf(stderr, "NULL Image\n");
		exit(EXIT_FAILURE);
	}

	if ((!width) || (!height)) {
		fprintf(stderr, "ROSE_ImageGetDimensions() Failed\n");
		fprintf(stderr, "NULL Result Storage\n");
		exit(EXIT_FAILURE);
	}

	*width = image->width;
	*height = image->height;
}


ROSE_Color ROSE_ImageGetPixel(ROSE_Image* image, i32 x, i32 y) {
	if (!image) {
		fprintf(stderr, "ROSE_ImageGetPixel() Failed\n");
		fprintf(stderr, "NULL Image\n");
		exit(EXIT_FAILURE);
	}

	if (!image->pixels) {
		fprintf(stderr, "ROSE_ImageGetPixel() Failed\n");
		fprintf(stderr, "NULL Pixels\n");
		exit(EXIT_FAILURE);
	}

	bool bad_bounds = (x < 0) || (x >= image->width);
	bad_bounds = bad_bounds || ((y < 0) || (y >= image->height));
	if (bad_bounds) {
		fprintf(stderr, "ROSE_ImageGetPixel() Failed\n");
		fprintf(stderr, "Pixel Position: (%d, %d)\n", x, y);
		fprintf(stderr, "Image Dimensions: (%d, %d)\n", image->width, image->height);
		exit(EXIT_FAILURE);
	}

	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	u8 r = image->pixels[index + 0];
	u8 g = image->pixels[index + 1];
	u8 b = image->pixels[index + 2];
	u8 a = image->pixels[index + 3];

	return (ROSE_Color) {
		.r = ((float)r / 255.0f),
		.g = ((float)g / 255.0f),
		.b = ((float)b / 255.0f),
		.a = ((float)a / 255.0f),
	};
}

void ROSE_ImageSetPixel(ROSE_Image* image, i32 x, i32 y, ROSE_Color color) {
	if (!image) {
		fprintf(stderr, "ROSE_ImageSetPixel() Failed\n");
		fprintf(stderr, "NULL Image\n");
		exit(EXIT_FAILURE);
	}

	if (!image->pixels) {
		fprintf(stderr, "ROSE_ImageSetPixel() Failed\n");
		fprintf(stderr, "NULL Pixels\n");
		exit(EXIT_FAILURE);
	}

	float r = color.r;
	float g = color.g;
	float b = color.b;
	float a = color.a;

	bool bad_color         = (r < 0.0f) || (r > 1.0f);
	bad_color = bad_color || (g < 0.0f) || (g > 1.0f);
	bad_color = bad_color || (b < 0.0f) || (b > 1.0f);
	bad_color = bad_color || (a < 0.0f) || (a > 1.0f);
	if (bad_color) {
		fprintf(stderr, "ROSE_ImageSetPixel() Failed\n");
		fprintf(stderr, "Invalid Color Range [0..1]: (%f, %f, %f, %f)\n", r, g, b, a);
		exit(EXIT_FAILURE);
	}

	bool bad_bounds = (x < 0) || (x >= image->width);
	bad_bounds = bad_bounds || ((y < 0) || (y >= image->height));
	if (bad_bounds) {
		fprintf(stderr, "ROSE_ImageSetPixel() Failed\n");
		fprintf(stderr, "Pixel Position: (%d, %d)\n", x, y);
		fprintf(stderr, "Image Dimensions: (%d, %d)\n", image->width, image->height);
		exit(EXIT_FAILURE);
	}

	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	image->pixels[index + 0] = (u8)(r * 255.0f);
	image->pixels[index + 1] = (u8)(g * 255.0f);
	image->pixels[index + 2] = (u8)(b * 255.0f);
	image->pixels[index + 3] = (u8)(a * 255.0f);
}

