
/*
 *   Source File [main.c]
 */

#include "rose.h"
#include "map.h"

Province* ProvincesSelect(Provinces* provinces, ROSE_Image* image, float xpos, float ypos, float scale)
{
	float x = (rose_mouse.xpos - xpos) / scale;
	float y = (rose_mouse.ypos - ypos) / scale;
	
	if ((x < 0.0f) || (y < 0.0f))
		return NULL;
	
	float width = (float)image->_width;
	float height = (float)image->_height;
	
	if ((x >= width) || (y >= height))
		return NULL;

	ROSE_RGBA color = ROSE_ImageGetPixelRGBA(image, (uint32_t)x, (uint32_t)y);
	Province* result = NULL;

	for (uint32_t i = 0; i < provinces->count; i++) {
		Province* province = &provinces->array[i];
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

void ProvincesDraw(Provinces* provinces, float xpos, float ypos, float scale)
{
	for (uint32_t i = 0; i < provinces->count; i++) {
		Province* province = &provinces->array[i];
		province->triangles.scale = scale;
		province->triangles.xpos = xpos;
		province->triangles.ypos = ypos;
		ROSE_MeshDraw(&province->triangles);
	}

	for (uint32_t i = 0; i < provinces->count; i++) {
		Province* province = &provinces->array[i];
		province->border.scale = scale;
		province->border.xpos = xpos;
		province->border.ypos = ypos;
		ROSE_MeshDraw(&province->border);
	}
}

void ProvincesDestroy(Provinces* provinces)
{
	if (provinces) {
		for (uint32_t i = 0; i < provinces->count; i++) {
			Province* province = &provinces->array[i];
			free(province->neighbours);
			ROSE_MeshDestroy(&province->border);
			ROSE_MeshDestroy(&province->triangles);
		}

		free(provinces->array);
		*provinces = (Provinces) { 0 };
	}
}

int main(void)
{
	ROSE_Init("Van Gogh's Epic Odyssey", 1600, 900);
	ROSE_WindowVSync(true);

	float scale = 2.0f;
	float xpos = 0.0f;
	float ypos = 0.0f;

	MapInfo info = {
		.map_width          = 500,
		.map_sea_level      = 0.5f,
		.perlin_seed        = 0,
		.perlin_divisions   = 1,
		.perlin_layers      = 5,
		.perlin_attenuation = 0.5f,
		.province_spacing   = 10,
	};

	ROSE_Image image = MAP_GenerateImage(&info);
	Provinces provinces = MAP_GenerateProvinces(&image);

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

		Province* selection = ProvincesSelect(&provinces, &image, xpos, ypos, scale);
		if (selection) {
			selection->triangles.color = ROSE_COLOR_RED;
		}
		

		ROSE_WindowBeginFrame(ROSE_COLOR_BLUE);
		ProvincesDraw(&provinces, xpos, ypos, scale);
		ROSE_WindowEndFrame();
	}

	ROSE_ImageDestroy(&image);
	ProvincesDestroy(&provinces);
	ROSE_Quit();
	return EXIT_SUCCESS;
}

