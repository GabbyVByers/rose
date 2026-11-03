
/*
 *   Source File [main.c]
 */

#include "rose.h"

#define MAP_GEOMETRY_CONSTRUCTOR
#if defined(MAP_GEOMETRY_CONSTRUCTOR)

#define INIT_CAPACITY 256

typedef struct Vec2 {
	int32_t x;
	int32_t y;
} Vec2;

typedef struct Province {
	ROSE_RGBA color;
	
	uint32_t*      neighbours;
	uint32_t       neighbour_count;
	uint32_t       neighbour_capacity;

	Vec2*          pixels;
	uint32_t       pixel_count;
	uint32_t       pixel_capacity;

	ROSE_Vertex2D* vert_points;
	uint32_t       vert_point_count;
	uint32_t       vert_point_capacity;

	ROSE_Vertex2D* vert_lines;
	uint32_t       vert_line_count;
	uint32_t       vert_line_capacity;
	
	ROSE_Vertex2D* vert_triangles;
	uint32_t       vert_triangle_count;
	uint32_t       vert_triangle_capacity;
	
	ROSE_Mesh      mesh_points;
	ROSE_Mesh      mesh_lines;
	ROSE_Mesh      mesh_triangles;

} Province;

typedef struct Provinces {
	Province* array;
	uint32_t count;
	uint32_t capacity;
} Provinces;

static bool ColorEqual(ROSE_RGBA colorA, ROSE_RGBA colorB)
{
	bool same =
		(colorA.r == colorB.r) &&
		(colorA.g == colorB.g) &&
		(colorA.b == colorB.b);
	return same;
}

static ROSE_RGBA MapGetPixel(ROSE_Image* image, int32_t x, int32_t y)
{
	bool bad_bounds =
		(x < 0) ||
		(y < 0) ||
		((uint32_t)x >= image->_width) ||
		((uint32_t)y >= image->_height);

	if (bad_bounds)
		return (ROSE_RGBA) { 0, 0, 255, 255 };
	
	return ROSE_ImageGetPixelRGBA(image, (uint32_t)x, (uint32_t)y);
}

static bool CornerIsChamfered(ROSE_Image* image, int32_t vx, int32_t vy, ROSE_RGBA color)
{
	const ROSE_RGBA around[4] = {
		MapGetPixel(image, vx - 1, vy - 1),
		MapGetPixel(image, vx,     vy - 1),
		MapGetPixel(image, vx - 1, vy    ),
		MapGetPixel(image, vx,     vy    ),
	};

	uint32_t  same_count = 0;
	bool      have_other = false;
	ROSE_RGBA other = { 0 };
	for (uint32_t k = 0; k < 4; k++) {
		if (ColorEqual(around[k], color)) {
			same_count++;
		} else if (!have_other) {
			other = around[k];
			have_other = true;
		} else if (!ColorEqual(around[k], other)) {
			return false;
		}
	}

	return (same_count == 1) || (same_count == 3);
}

Provinces GenerateMapGeometry(ROSE_Image* image)
{
	// here
}

int main(void)
{
	ROSE_Init("Van Gogh's Epic Odyssey", 1200, 700);
	ROSE_WindowVSync(true);

	ROSE_Image image = ROSE_ImageLoadFromFile("map.png");
	const uint32_t width  = image._width;
	const uint32_t height = image._height;

	const ROSE_RGBA color_blue  = (ROSE_RGBA) { 0, 0, 255, 255 };

	uint32_t  province_count = 0;
	uint32_t  province_capacity = INIT_CAPACITY;
	Province* provinces = calloc(INIT_CAPACITY, sizeof(Province));

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			ROSE_RGBA curr_color = ROSE_ImageGetPixelRGBA(&image, x, y);
			const bool ocean =
				(curr_color.r == color_blue.r) &&
				(curr_color.g == color_blue.g) &&
				(curr_color.b == color_blue.b);
			if (ocean)
				continue;

			bool seen_color = false;
			for (uint32_t i = 0; i < province_count; i++) {
				Province* other = &provinces[i];
				ROSE_RGBA other_color = other->color;
				const bool seen =
					(curr_color.r == other_color.r) &&
					(curr_color.g == other_color.g) &&
					(curr_color.b == other_color.b);
				if (seen) {
					seen_color = true;
					break;
				}
			}

			if (seen_color)
				continue;

			Province province = (Province) {
				.color                  = curr_color,
				.pixels                 = NULL,
				.pixel_count            = 0,
				.pixel_capacity         = INIT_CAPACITY,
				.neighbours             = NULL,
				.neighbour_count        = 0,
				.neighbour_capacity     = INIT_CAPACITY,
				.vert_points            = NULL,
				.vert_point_count       = 0,
				.vert_point_capacity    = INIT_CAPACITY,
				.vert_triangles        = NULL,
				.vert_triangle_count    = 0,
				.vert_triangle_capacity = INIT_CAPACITY,
				.vert_lines             = NULL,
				.vert_line_count        = 0,
				.vert_line_capacity     = INIT_CAPACITY,
				.mesh_points    = (ROSE_Mesh) { 0 },
				.mesh_lines     = (ROSE_Mesh) { 0 },
				.mesh_triangles = (ROSE_Mesh) { 0 },
			};

			province.pixels         = calloc(INIT_CAPACITY, sizeof(Vec2));
			province.neighbours     = calloc(INIT_CAPACITY, sizeof(uint32_t));
			province.vert_points    = calloc(INIT_CAPACITY, sizeof(ROSE_Vertex2D));
			province.vert_triangles = calloc(INIT_CAPACITY, sizeof(ROSE_Vertex2D));
			province.vert_lines     = calloc(INIT_CAPACITY, sizeof(ROSE_Vertex2D));

			if (province_count >= province_capacity) {
				province_capacity *= 2;
				provinces = realloc(provinces, province_capacity * sizeof(Province));
			} provinces[province_count++] = province;
		}
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			ROSE_RGBA curr_color = ROSE_ImageGetPixelRGBA(&image, x, y);
			const bool ocean =
				(curr_color.r == color_blue.r) &&
				(curr_color.g == color_blue.g) &&
				(curr_color.b == color_blue.b);
			if (ocean)
				continue;

			Province* province = NULL;
			for (uint32_t i = 0; i < province_count; i++) {
				Province* other = &provinces[i];
				const bool color_match =
					(curr_color.r == other->color.r) &&
					(curr_color.g == other->color.g) &&
					(curr_color.b == other->color.b);
				if (color_match) {
					province = other;
					break;
				}
			}

			Vec2 pixel = (Vec2){ x, y };
			if (province->pixel_count >= province->pixel_capacity) {
				province->pixel_capacity *= 2;
				province->pixels = realloc(province->pixels, province->pixel_capacity * sizeof(Vec2));
			} province->pixels[province->pixel_count++] = pixel;
		}
	}

	{
		const int32_t direction_steps[4][2] = { { 1,  0 }, { 0, 1 }, { -1,  0 }, {  0, -1 } };
		const int32_t side_neighbours[4][2] = { { 0, -1 }, { 1, 0 }, {  0,  1 }, { -1,  0 } };
		const int32_t side_starts[4][2]     = { { 0,  0 }, { 1, 0 }, {  1,  1 }, {  0,  1 } };

		const uint32_t vertex_columns = width + 1;
		const uint32_t vertex_rows    = height + 1;
		uint8_t* edge_masks = calloc((size_t)vertex_columns * vertex_rows, sizeof(uint8_t));

		for (uint32_t i = 0; i < province_count; i++) {
			Province* province = &provinces[i];

			for (uint32_t j = 0; j < province->pixel_count; j++) {
				const Vec2 pixel = province->pixels[j];
				for (uint32_t s = 0; s < 4; s++) {
					const ROSE_RGBA neighbour = MapGetPixel(&image, pixel.x + side_neighbours[s][0], pixel.y + side_neighbours[s][1]);
					if (ColorEqual(neighbour, province->color))
						continue;
					const uint32_t v = ((uint32_t)(pixel.y + side_starts[s][1]) * vertex_columns) + (uint32_t)(pixel.x + side_starts[s][0]);
					edge_masks[v] |= (uint8_t)(1u << s);
				}
			}

			const int32_t start_x = province->pixels[0].x;
			const int32_t start_y = province->pixels[0].y;
			int32_t  vx  = start_x;
			int32_t  vy  = start_y;
			uint32_t dir = 0;

			for (;;) {
				edge_masks[((uint32_t)vy * vertex_columns) + (uint32_t)vx] &= (uint8_t)~(1u << dir);

				if ((province->vert_point_count + 2) > province->vert_point_capacity) {
					province->vert_point_capacity *= 2;
					province->vert_points = realloc(province->vert_points, province->vert_point_capacity * sizeof(ROSE_Vertex2D));
				}

				const ROSE_Vertex2D midpoint = { {
					(float)vx + (0.5f * (float)direction_steps[dir][0]),
					(float)vy + (0.5f * (float)direction_steps[dir][1]),
				} };
				province->vert_points[province->vert_point_count++] = midpoint;

				vx += direction_steps[dir][0];
				vy += direction_steps[dir][1];

				const bool closed = (vx == start_x) && (vy == start_y);
				uint32_t next = 0;
				if (!closed) {
					const uint8_t mask = edge_masks[((uint32_t)vy * vertex_columns) + (uint32_t)vx];
					while (!(mask & (1u << next))) next++;
				}

				if ((next != dir) && !CornerIsChamfered(&image, vx, vy, province->color)) {
					const ROSE_Vertex2D corner = { { (float)vx, (float)vy } };
					province->vert_points[province->vert_point_count++] = corner;
				}

				dir = next;
				if (closed)
					break;
			}
		}

		free(edge_masks);
	}

	for (uint32_t i = 0; i < province_count; i++) {
		Province* province = &provinces[i];
		const uint32_t point_count = province->vert_point_count;
		if (point_count == 0)
			continue;

		ROSE_Vertex2D* original = malloc(point_count * sizeof(ROSE_Vertex2D));
		memcpy(original, province->vert_points, point_count * sizeof(ROSE_Vertex2D));

		uint32_t kept_count = 0;
		for (uint32_t j = 0; j < point_count; j++) {
			const ROSE_Vertex2D prev = original[(j + point_count - 1) % point_count];
			const ROSE_Vertex2D curr = original[j];
			const ROSE_Vertex2D next = original[(j + 1) % point_count];

			const float in_x  = curr.position[0] - prev.position[0];
			const float in_y  = curr.position[1] - prev.position[1];
			const float out_x = next.position[0] - curr.position[0];
			const float out_y = next.position[1] - curr.position[1];

			const float cross = (in_x * out_y) - (in_y * out_x);
			const float dot   = (in_x * out_x) + (in_y * out_y);

			const bool inline_point = (cross == 0.0f) && (dot > 0.0f);
			if (!inline_point) {
				province->vert_points[kept_count++] = curr;
			}
		}

		province->vert_point_count = kept_count;
		free(original);
	}

	for (uint32_t i = 0; i < province_count; i++) {
		Province* province = &provinces[i];
		const uint32_t point_count = province->vert_point_count;
		for (uint32_t j = 0; j < point_count; j++) {
			ROSE_Vertex2D pointA = province->vert_points[j];
			ROSE_Vertex2D pointB = province->vert_points[(j + 1) % point_count];

			if ((province->vert_line_count + 2) > province->vert_line_capacity) {
				province->vert_line_capacity *= 2;
				province->vert_lines = realloc(province->vert_lines, province->vert_line_capacity * sizeof(ROSE_Vertex2D));
			}

			province->vert_lines[province->vert_line_count++] = pointA;
			province->vert_lines[province->vert_line_count++] = pointB;
		}
	}

	//// --- dump --------------
	//
	//fprintf(stdout,
	//	"PROVINCE COUNT: %d\n",
	//	province_count
	//);
	//
	//for (uint32_t i = 0; i < province_count; i++) {
	//	Province* province = &provinces[i];
	//	fprintf(stdout, "COLOR: %d, %d, %d\t PIXEL COUNT: %d\n",
	//		province->color.r,
	//		province->color.g,
	//		province->color.b,
	//		province->pixel_count
	//	);
	//}

	// --- visual ------------

	for (uint32_t i = 0; i < province_count; i++) {
		Province* province = &provinces[i];
		
		if (province->vert_point_count != 0) {
			province->mesh_points = ROSE_MeshCreate(province->vert_points, province->vert_point_count);
			province->mesh_points.shape = ROSE_POINTS;
			province->mesh_points.diameter = 4;
		}
		
		if (province->vert_line_count != 0) {
			province->mesh_lines = ROSE_MeshCreate(province->vert_lines, province->vert_line_count);
			province->mesh_lines.shape = ROSE_LINES;
		}
		
		if (province->vert_triangle_count != 0) {
			province->mesh_triangles = ROSE_MeshCreate(province->vert_triangles, province->vert_triangle_count);
			province->mesh_triangles.shape = ROSE_TRIANGLES;
		}
	}

	uint32_t map_scale = 1;
	float map_xpos = 0.0f;
	float map_ypos = 0.0f;

	ROSE_Sprite sprite = ROSE_SpriteCreate(&image);

	while (ROSE_WindowPollEvents())
	{
		int32_t scroll = 0;
		if (rose_mouse.scroll > 0.0f) scroll = 1;
		if (rose_mouse.scroll < 0.0f) scroll = -1;

		if (scroll != 0) {
			if (!((scroll == -1) && (map_scale <= 1))) {
				map_xpos = rose_mouse.xpos + (map_xpos - rose_mouse.xpos) * ((float)(map_scale + scroll) / map_scale);
				map_ypos = rose_mouse.ypos + (map_ypos - rose_mouse.ypos) * ((float)(map_scale + scroll) / map_scale);
				map_scale += scroll;
			}
		}

		if (ROSE_MousePressing(ROSE_BUTTON_MIDDLE)) {
			map_xpos += rose_mouse.xvel;
			map_ypos += rose_mouse.yvel;
		}

		sprite.scale = map_scale;
		sprite.xpos = (int32_t)map_xpos;
		sprite.ypos = (int32_t)map_ypos;

		ROSE_WindowBeginFrame(ROSE_COLOR_BLUE);
		ROSE_SpriteDraw(&sprite);

		for (uint32_t i = 0; i < province_count; i++) {
			Province* province = &provinces[i];
			province->mesh_points.scale = (float)map_scale;
			province->mesh_lines.scale = (float)map_scale;
			province->mesh_triangles.scale = (float)map_scale;
			province->mesh_points.xpos = (float)sprite.xpos;
			province->mesh_points.ypos = (float)sprite.ypos;
			province->mesh_lines.xpos = (float)sprite.xpos;
			province->mesh_lines.ypos = (float)sprite.ypos;
			province->mesh_triangles.xpos = (float)sprite.xpos;
			province->mesh_triangles.ypos = (float)sprite.ypos;
			ROSE_MeshDraw(&province->mesh_points);
			ROSE_MeshDraw(&province->mesh_lines);
			ROSE_MeshDraw(&province->mesh_triangles);
		}

		ROSE_WindowEndFrame();
	}

	for (uint32_t i = 0; i < province_count; i++) {
		Province* province = &provinces[i];
		free(province->neighbours);
		free(province->pixels);
		free(province->vert_points);
		free(province->vert_lines);
		free(province->vert_triangles);
		ROSE_MeshDestroy(&province->mesh_points);
		ROSE_MeshDestroy(&province->mesh_lines);
		ROSE_MeshDestroy(&province->mesh_triangles);
	} free(provinces);

	ROSE_ImageDestroy(&image);
	ROSE_SpriteDestroy(&sprite);
	ROSE_Quit();
}

#endif /* MAP_GEOMETRY_CONSTRUCTOR */

//#define MAP_GENERATOR_REFACTOR
#if defined(MAP_GENERATOR_REFACTOR)

typedef enum MapStage {
	FULL_IMAGE = 0,
	PERLIN_NOISE,
	PERLIN_CENTERED,
	MAP_SEA_LEVEL,
	MAP_DEBURRING,
	MAP_FLOOD_FILL,
	MAP_NO_LAKES,
} MapStage;

typedef struct MapInfo {
	uint32_t map_width;
	MapStage map_stage;
	float    map_sea_level;
	uint32_t perlin_seed;
	uint32_t perlin_divisions;
	uint32_t perlin_layers;
	float    perlin_attenuation;
	uint32_t province_spacing;
} MapInfo;

ROSE_Image ImageMapGenerate(MapInfo* map_info);

#define MAP_WIDTH 350
#define MAP_SCALE 2
#define MAP_PADDING 50
#define GUI_SCALE 3
#define GUI_PADDING 20
#define SLIDER_WIDTH 48

#define BUTTON_SMALL_WIDTH (ROSE_ASCII_WIDTH * 3)
#define BUTTON_LARGE_WIDTH (ROSE_ASCII_WIDTH * 7)
#define GUI_HEIGHT ((ROSE_ASCII_HEIGHT + 4) * GUI_SCALE)
#define GUI_HORIZONTAL_POSITION(x) (GUI_PADDING + (((BUTTON_SMALL_WIDTH * GUI_SCALE) + GUI_PADDING) * x))
#define GUI_VERTICAL_POSITION(x) (GUI_PADDING + ((GUI_HEIGHT + GUI_PADDING) * x))
#define GUI_LARGE_HORIZONTAL_POSITION(x) (GUI_PADDING + (((BUTTON_LARGE_WIDTH * GUI_SCALE) + GUI_PADDING) * x))

#define STRING_SAVE_MAP_PATH "map.png"
#define STRING_SAVE_MAP      "SAVE AS PNG"
#define STRING_RANDOM_SEED   "RANDOM SEED"

#define STRING_SCALE     "SCALE:     %d"
#define STRING_LAYERS    "LAYERS:    %d"
#define STRING_DETAIL    "DETAIL:    %.2f"
#define STRING_SEA_LEVEL "SEA LEVEL: %.2f"
#define STRING_INC       "+"
#define STRING_DEC       "-"

int main(void)
{
	ROSE_Init("Pearl", 1600, 900);
	ROSE_WindowVSync(false);

	MapInfo info = (MapInfo){
		.map_width              = MAP_WIDTH,
		.map_stage              = FULL_IMAGE,
		.map_sea_level          = 0.5f,
		.perlin_divisions       = 5,
		.perlin_layers          = 5,
		.perlin_attenuation     = 1.0f,
		.province_spacing       = 10,
	};

	ROSE_Image image = (ROSE_Image) { 0 };
	ROSE_Sprite map = ROSE_SpriteCreateEmpty(MAP_WIDTH, MAP_WIDTH);
	map.scale = MAP_SCALE;

	// [  SAVE MAP   ]
	// [ RANDOM SEED ]
	ROSE_Button button_save_map      = ROSE_ButtonCreate(STRING_SAVE_MAP);
	ROSE_Button button_random_seed   = ROSE_ButtonCreate(STRING_RANDOM_SEED);
	button_save_map.scale            = GUI_SCALE;
	button_random_seed.scale         = GUI_SCALE;
	button_save_map.xpos             = GUI_HORIZONTAL_POSITION(0);
	button_random_seed.xpos          = GUI_HORIZONTAL_POSITION(0);
	button_save_map.ypos             = GUI_VERTICAL_POSITION(0);
	button_random_seed.ypos          = GUI_VERTICAL_POSITION(1);
	
	// [-] [+] SCALE: #
	ROSE_Button button_init_divs_dec = ROSE_ButtonCreate(STRING_DEC);
	ROSE_Button button_init_divs_inc = ROSE_ButtonCreate(STRING_INC);
	ROSE_Text   text_init_divs       = ROSE_TextCreate(STRING_SCALE, info.perlin_divisions);
	button_init_divs_dec.scale       = GUI_SCALE;
	button_init_divs_inc.scale       = GUI_SCALE;
	text_init_divs.scale             = GUI_SCALE;
	button_init_divs_dec.xpos        = GUI_HORIZONTAL_POSITION(0);
	button_init_divs_inc.xpos        = GUI_HORIZONTAL_POSITION(1);
	text_init_divs.xpos              = GUI_HORIZONTAL_POSITION(2);
	button_init_divs_dec.ypos        = GUI_VERTICAL_POSITION(2);
	button_init_divs_inc.ypos        = GUI_VERTICAL_POSITION(2);
	text_init_divs.ypos              = GUI_VERTICAL_POSITION(2) + (2 * GUI_SCALE);
	button_init_divs_dec.width       = BUTTON_SMALL_WIDTH;
	button_init_divs_inc.width       = BUTTON_SMALL_WIDTH;
	
	// [-] [+] LAYERS: #
	ROSE_Button button_layers_dec    = ROSE_ButtonCreate(STRING_DEC);
	ROSE_Button button_layers_inc    = ROSE_ButtonCreate(STRING_INC);
	ROSE_Text   text_layers          = ROSE_TextCreate(STRING_LAYERS, info.perlin_layers);
	button_layers_dec.scale          = GUI_SCALE;
	button_layers_inc.scale          = GUI_SCALE;
	text_layers.scale                = GUI_SCALE;
	button_layers_dec.xpos           = GUI_HORIZONTAL_POSITION(0);
	button_layers_inc.xpos           = GUI_HORIZONTAL_POSITION(1);
	text_layers.xpos                 = GUI_HORIZONTAL_POSITION(2);
	button_layers_dec.ypos           = GUI_VERTICAL_POSITION(3);
	button_layers_inc.ypos           = GUI_VERTICAL_POSITION(3);
	text_layers.ypos                 = GUI_VERTICAL_POSITION(3) + (2 * GUI_SCALE);
	button_layers_dec.width          = BUTTON_SMALL_WIDTH;
	button_layers_inc.width          = BUTTON_SMALL_WIDTH;
	
	// --[]--- DETAIL: #
	ROSE_Slider slider_attenuation   = ROSE_SliderCreate();
	ROSE_Text   text_attenuation     = ROSE_TextCreate(STRING_DETAIL, info.perlin_attenuation);
	slider_attenuation.subject       = &info.perlin_attenuation;
	slider_attenuation.scale         = GUI_SCALE;
	text_attenuation.scale           = GUI_SCALE;
	slider_attenuation.xpos          = GUI_HORIZONTAL_POSITION(0);
	text_attenuation.xpos            = GUI_HORIZONTAL_POSITION(2);
	slider_attenuation.ypos          = GUI_VERTICAL_POSITION(4);
	text_attenuation.ypos            = GUI_VERTICAL_POSITION(4) + (2 * GUI_SCALE);
	slider_attenuation.width         = SLIDER_WIDTH;
	
	// --[]--- SEA LEVEL: #
	ROSE_Slider slider_sea_level     = ROSE_SliderCreate();
	ROSE_Text   text_sea_level       = ROSE_TextCreate(STRING_SEA_LEVEL, info.map_sea_level);
	slider_sea_level.subject         = &info.map_sea_level;
	slider_sea_level.scale           = GUI_SCALE;
	text_sea_level.scale             = GUI_SCALE;
	slider_sea_level.xpos            = GUI_HORIZONTAL_POSITION(0);
	text_sea_level.xpos              = GUI_HORIZONTAL_POSITION(2);
	slider_sea_level.ypos            = GUI_VERTICAL_POSITION(5);
	text_sea_level.ypos              = GUI_VERTICAL_POSITION(5) + (2 * GUI_SCALE);
	slider_sea_level.width           = SLIDER_WIDTH;
	slider_sea_level.min             = 0.2f;
	slider_sea_level.max             = 0.7f; // sometimes unsafe - use responsibly :)
	
	// [ RAW ] [ CLIP ] [ LAND ] [ DEBURR ]
	ROSE_Button button_show_raw       = ROSE_ButtonCreate("RAW");
	ROSE_Button button_show_clipped   = ROSE_ButtonCreate("CLIP");
	ROSE_Button button_show_land      = ROSE_ButtonCreate("LAND");
	ROSE_Button button_show_deburred  = ROSE_ButtonCreate("DEBURR");
	ROSE_Button button_show_flood     = ROSE_ButtonCreate("FLOOD");
	ROSE_Button button_show_no_lakes  = ROSE_ButtonCreate("LAKES");
	ROSE_Button button_show_provinces = ROSE_ButtonCreate("PROVS");
	button_show_raw.scale            = GUI_SCALE;
	button_show_clipped.scale        = GUI_SCALE;
	button_show_land.scale           = GUI_SCALE;
	button_show_deburred.scale       = GUI_SCALE;
	button_show_flood.scale          = GUI_SCALE;
	button_show_no_lakes.scale       = GUI_SCALE;
	button_show_provinces.scale      = GUI_SCALE;
	button_show_raw.width            = BUTTON_LARGE_WIDTH;
	button_show_clipped.width        = BUTTON_LARGE_WIDTH;
	button_show_land.width           = BUTTON_LARGE_WIDTH;
	button_show_deburred.width       = BUTTON_LARGE_WIDTH;
	button_show_flood.width          = BUTTON_LARGE_WIDTH;
	button_show_no_lakes.width       = BUTTON_LARGE_WIDTH;
	button_show_provinces.width      = BUTTON_LARGE_WIDTH;

	while (ROSE_WindowPollEvents())
	{
		const bool pressed_button_save_map       = ROSE_ButtonPressed(&button_save_map);
		const bool pressed_button_random_seed    = ROSE_ButtonPressed(&button_random_seed);
		const bool pressed_button_init_divs_dec  = ROSE_ButtonPressed(&button_init_divs_dec);
		const bool pressed_button_init_divs_inc  = ROSE_ButtonPressed(&button_init_divs_inc);
		const bool pressed_button_layers_dec     = ROSE_ButtonPressed(&button_layers_dec);
		const bool pressed_button_layers_inc     = ROSE_ButtonPressed(&button_layers_inc);
		const bool pressed_button_show_raw       = ROSE_ButtonPressed(&button_show_raw);
		const bool pressed_button_show_clipped   = ROSE_ButtonPressed(&button_show_clipped);
		const bool pressed_button_show_land      = ROSE_ButtonPressed(&button_show_land);
		const bool pressed_button_show_deburred  = ROSE_ButtonPressed(&button_show_deburred);
		const bool pressed_button_show_flood     = ROSE_ButtonPressed(&button_show_flood);
		const bool pressed_button_show_no_lakes  = ROSE_ButtonPressed(&button_show_no_lakes);
		const bool pressed_button_show_provinces = ROSE_ButtonPressed(&button_show_provinces);

		if (pressed_button_save_map) {
			if (&image._pixels) {
				ROSE_ImageSavePNG(&image, STRING_SAVE_MAP_PATH);
			}
		}

		if (pressed_button_random_seed) {
			info.perlin_seed = rand();
		}

		if (pressed_button_init_divs_dec) {
			if (info.perlin_divisions > 1) {
				info.perlin_divisions--;
				ROSE_TextUpdateString(&text_init_divs, STRING_SCALE, info.perlin_divisions);
			}
		}

		if (pressed_button_init_divs_inc) {
			if (info.perlin_divisions < 20) {
				info.perlin_divisions++;
				ROSE_TextUpdateString(&text_init_divs, STRING_SCALE, info.perlin_divisions);
			}
		}

		if (pressed_button_layers_dec) {
			if (info.perlin_layers > 1) {
				info.perlin_layers--;
				ROSE_TextUpdateString(&text_layers, STRING_LAYERS, info.perlin_layers);
			}
		}

		if (pressed_button_layers_inc) {
			if (info.perlin_layers < 8) {
				info.perlin_layers++;
				ROSE_TextUpdateString(&text_layers, STRING_LAYERS, info.perlin_layers);
			}
		}

		if (pressed_button_show_raw)       { info.map_stage = PERLIN_NOISE;    }
		if (pressed_button_show_clipped)   { info.map_stage = PERLIN_CENTERED; }
		if (pressed_button_show_land)      { info.map_stage = MAP_SEA_LEVEL;   }
		if (pressed_button_show_deburred)  { info.map_stage = MAP_DEBURRING;   }
		if (pressed_button_show_flood)     { info.map_stage = MAP_FLOOD_FILL;  }
		if (pressed_button_show_no_lakes)  { info.map_stage = MAP_NO_LAKES;    }
		if (pressed_button_show_provinces) { info.map_stage = FULL_IMAGE;      }

		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		{
			ROSE_ImageDestroy(&image);
			image = ImageMapGenerate(&info);
			ROSE_SpriteUpdateTexture(&map, &image);

			map.xpos = rose_screen.width - (MAP_PADDING + (MAP_WIDTH * map.scale));
			map.ypos = MAP_PADDING;

			button_show_raw.xpos       = GUI_LARGE_HORIZONTAL_POSITION(0) + map.xpos;
			button_show_clipped.xpos   = GUI_LARGE_HORIZONTAL_POSITION(1) + map.xpos;
			button_show_land.xpos      = GUI_LARGE_HORIZONTAL_POSITION(2) + map.xpos;
			button_show_deburred.xpos  = GUI_LARGE_HORIZONTAL_POSITION(3) + map.xpos;
			button_show_flood.xpos     = GUI_LARGE_HORIZONTAL_POSITION(0) + map.xpos;
			button_show_no_lakes.xpos  = GUI_LARGE_HORIZONTAL_POSITION(1) + map.xpos;
			button_show_provinces.xpos = GUI_LARGE_HORIZONTAL_POSITION(2) + map.xpos;

			button_show_raw.ypos       = GUI_VERTICAL_POSITION(0) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_clipped.ypos   = GUI_VERTICAL_POSITION(0) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_land.ypos      = GUI_VERTICAL_POSITION(0) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_deburred.ypos  = GUI_VERTICAL_POSITION(0) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_flood.ypos     = GUI_VERTICAL_POSITION(1) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_no_lakes.ypos  = GUI_VERTICAL_POSITION(1) + map.ypos + (MAP_WIDTH * MAP_SCALE);
			button_show_provinces.ypos = GUI_VERTICAL_POSITION(1) + map.ypos + (MAP_WIDTH * MAP_SCALE);

			ROSE_ButtonDraw(&button_save_map);
			ROSE_ButtonDraw(&button_random_seed);

			ROSE_ButtonDraw(&button_init_divs_dec);
			ROSE_ButtonDraw(&button_init_divs_inc);
			ROSE_TextDraw(&text_init_divs);

			ROSE_ButtonDraw(&button_layers_dec);
			ROSE_ButtonDraw(&button_layers_inc);
			ROSE_TextDraw(&text_layers);

			ROSE_SliderDraw(&slider_attenuation);
			ROSE_TextUpdateString(&text_attenuation, STRING_DETAIL, info.perlin_attenuation);
			ROSE_TextDraw(&text_attenuation);

			ROSE_SliderDraw(&slider_sea_level);
			ROSE_TextUpdateString(&text_sea_level, STRING_SEA_LEVEL, info.map_sea_level);
			ROSE_TextDraw(&text_sea_level);

			ROSE_SpriteDraw(&map);
			ROSE_ButtonDraw(&button_show_raw);
			ROSE_ButtonDraw(&button_show_clipped);
			ROSE_ButtonDraw(&button_show_land);
			ROSE_ButtonDraw(&button_show_deburred);
			ROSE_ButtonDraw(&button_show_flood);
			ROSE_ButtonDraw(&button_show_no_lakes);
			ROSE_ButtonDraw(&button_show_provinces);
		}
		ROSE_WindowEndFrame();
	}

	ROSE_ImageDestroy(&image);
	ROSE_SpriteDestroy(&map);

	ROSE_ButtonDestroy(&button_save_map);
	ROSE_ButtonDestroy(&button_random_seed);

	ROSE_ButtonDestroy(&button_init_divs_dec);
	ROSE_ButtonDestroy(&button_init_divs_inc);
	ROSE_TextDestroy(&text_init_divs);

	ROSE_ButtonDestroy(&button_layers_dec);
	ROSE_ButtonDestroy(&button_layers_inc);
	ROSE_TextDestroy(&text_layers);

	ROSE_SliderDestroy(&slider_attenuation);
	ROSE_TextDestroy(&text_attenuation);

	ROSE_SliderDestroy(&slider_sea_level);
	ROSE_TextDestroy(&text_sea_level);

	ROSE_ButtonDestroy(&button_show_raw);
	ROSE_ButtonDestroy(&button_show_clipped);
	ROSE_ButtonDestroy(&button_show_land);
	ROSE_ButtonDestroy(&button_show_deburred);
	ROSE_ButtonDestroy(&button_show_flood);
	ROSE_ButtonDestroy(&button_show_no_lakes);
	ROSE_ButtonDestroy(&button_show_provinces);

	ROSE_Quit();
	return EXIT_SUCCESS;
}

ROSE_Image ImageMapGenerate(MapInfo* map_info)
{
	const uint32_t width     = map_info->map_width;
	const MapStage stage     = map_info->map_stage;
	const float    sea_level = map_info->map_sea_level;

	const uint32_t seed             = map_info->perlin_seed;
	const uint32_t divisions        = map_info->perlin_divisions;
	const uint32_t layers           = map_info->perlin_layers;
	const float    attenuation      = map_info->perlin_attenuation;
	const uint32_t province_spacing = map_info->province_spacing;

	const ROSE_RGBA color_red   = (ROSE_RGBA) { 255, 0, 0, 255 };
	const ROSE_RGBA color_green = (ROSE_RGBA) { 0, 255, 0, 255 };
	const ROSE_RGBA color_blue  = (ROSE_RGBA) { 0, 0, 255, 255 };
	
	// PERLIN_NOISE

	ROSE_Perlin perlin = ROSE_PerlinCreate(width, width);
	ROSE_Image  image = (ROSE_Image) { 0 };

	perlin.seed        = seed;
	perlin.divisions   = divisions;
	perlin.layers      = layers;
	perlin.attenuation = attenuation;
	ROSE_PerlinGenerate(&perlin);

	if (stage == PERLIN_NOISE) {
		image = ROSE_ImageCreateFromPerlin(&perlin);
		ROSE_PerlinDestroy(&perlin);
		return image;
	}

	// PERLIN_CENTERED

	{
		const int32_t radius = ((int32_t)width / 2);
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				int32_t dx = x - radius;
				int32_t dy = y - radius;
				int32_t dist2 = (dx * dx) + (dy * dy);
				int32_t radius2 = radius * radius;

				if (dist2 > radius2) {
					ROSE_PerlinSetIntensity(&perlin, x, y, 0.0f);
				} else {
					float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
					float s = 1.0f - ((float)dist2 / radius2);
					float smooth_strength = (((3.0f * s) + 2.0f - (s * s * s)) / 2.0f) - 1.0f;
					intensity *= smooth_strength;
					ROSE_PerlinSetIntensity(&perlin, x, y, intensity);
				}
			}
		}

		if (stage == PERLIN_CENTERED) {
			image = ROSE_ImageCreateFromPerlin(&perlin);
			ROSE_PerlinDestroy(&perlin);
			return image;
		}
	}

	// MAP_SEA_LEVEL

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
				if (intensity < sea_level) {
					ROSE_PerlinSetIntensity(&perlin, x, y, 0.0f);
				} else {
					ROSE_PerlinSetIntensity(&perlin, x, y, 1.0f);
				}
			}
		}

		image = ROSE_ImageCreateFromPerlin(&perlin);

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
				if (intensity >= 0.95f) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_green);
				} else {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
				}
			}
		}

		ROSE_PerlinDestroy(&perlin);

		if (stage == MAP_SEA_LEVEL) {
			return image;
		}
	}

	// MAP_DEBURRING

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool water = (color.b == 255);
				if (water) continue;

				ROSE_RGBA top    = ROSE_ImageGetPixelRGBA(&image, x, y - 1);
				ROSE_RGBA bottom = ROSE_ImageGetPixelRGBA(&image, x, y + 1);
				ROSE_RGBA left   = ROSE_ImageGetPixelRGBA(&image, x - 1, y);
				ROSE_RGBA right  = ROSE_ImageGetPixelRGBA(&image, x + 1, y);
				ROSE_RGBA top_right    = ROSE_ImageGetPixelRGBA(&image, x + 1, y - 1);
				ROSE_RGBA top_left     = ROSE_ImageGetPixelRGBA(&image, x - 1, y - 1);
				ROSE_RGBA bottom_right = ROSE_ImageGetPixelRGBA(&image, x + 1, y + 1);
				ROSE_RGBA bottom_left  = ROSE_ImageGetPixelRGBA(&image, x - 1, y + 1);

				bool water_top    = (top.b    == 255);
				bool water_bottom = (bottom.b == 255);
				bool water_left   = (left.b   == 255);
				bool water_right  = (right.b  == 255);
				bool land_top_right    = (top_right.g    == 255);
				bool land_top_left     = (top_left.g     == 255);
				bool land_bottom_right = (bottom_right.g == 255);
				bool land_bottom_left  = (bottom_left.g  == 255);

				bool tiny_island = water_top
					&& water_bottom
					&& water_left
					&& water_right
					&& !land_top_right
					&& !land_top_left
					&& !land_bottom_right
					&& !land_bottom_left;

				bool deburr = tiny_island
					|| (water_top    && water_right && land_top_right   )
					|| (water_top    && water_left  && land_top_left    )
					|| (water_bottom && water_right && land_bottom_right)
					|| (water_bottom && water_left  && land_bottom_left );

				if (deburr) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
					continue;
				}
			}
		}

		if (stage == MAP_DEBURRING) {
			return image;
		}
	}

	// MAP_FLOOD_FILL

	{
		typedef struct {
			uint32_t x;
			uint32_t y;
		} Vec2;

		const int32_t neighbour_offsets[4][2] = {
			{  0, -1 },
			{  0,  1 },
			{ -1,  0 },
			{  1,  0 }
		};

		uint32_t position_queue_head = 0;
		uint32_t position_queue_size = 0;
		uint32_t position_queue_capacity = 1024;
		Vec2* position_queue = malloc(position_queue_capacity * sizeof(Vec2));

		ROSE_ImageSetPixelRGBA(&image, 0, 0, color_red);
		position_queue[position_queue_size++] = (Vec2) { 0, 0 };

		while (position_queue_head < position_queue_size) {
			Vec2 position = position_queue[position_queue_head++];

			for (int32_t i = 0; i < 4; i++) {
				uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
				uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
				if ((next_xpos >= width) || (next_ypos >= width)) continue;

				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, next_xpos, next_ypos);
				bool is_water = (color.b == 255);
				if (!is_water) continue;

				if (position_queue_size == position_queue_capacity) {
					position_queue_capacity *= 2;
					Vec2* new_queue = realloc(position_queue, position_queue_capacity * sizeof(Vec2));
					position_queue = new_queue;
				}

				ROSE_ImageSetPixelRGBA(&image, next_xpos, next_ypos, color_red);
				position_queue[position_queue_size++] = (Vec2){ next_xpos, next_ypos };
			}
		}

		free(position_queue);

		if (stage == MAP_FLOOD_FILL) {
			return image;
		}
	}

	// MAP_NO_LAKES

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_lake = (color.b == 255);
				if (is_lake) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_green);
				}
			}
		}

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_blood = (color.r == 255);
				if (is_blood) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
				}
			}
		}

		if (stage == MAP_NO_LAKES) {
			return image;
		}
	}

	// PROVINCES

	{
		typedef struct {
			uint32_t x;
			uint32_t y;
		} Vec2;

		typedef struct {
			ROSE_RGBA color;
			Vec2*     frontier;
			uint32_t  pixel_count;
			uint32_t  frontier_size;
			uint32_t  frontier_capacity;
		} PixelProv;

		const uint32_t seeds_per_row =
			(width + province_spacing - 1)
			/ province_spacing;

		uint32_t province_count = 0;
		uint32_t province_capacity = seeds_per_row * seeds_per_row;
		PixelProv* provinces = malloc(province_capacity * sizeof(PixelProv));

		for (uint32_t y = province_spacing; y < (width - province_spacing); y += province_spacing) {
			for (uint32_t x = province_spacing; x < (width - province_spacing); x += province_spacing) {
				if ((y / province_spacing) % 2 == 0)
					x += (province_spacing / 2);

				if (x >= image._width) __debugbreak();
				if (y >= image._height) __debugbreak();

				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_land = (color.g == 255);
				if (!is_land) continue;

				uint8_t r = 0;
				uint8_t g = 0;
				uint8_t b = 0;

				for (;;) {
					r = (uint8_t)(55 + (rand() % 200));
					g = (uint8_t)(55 + (rand() % 200));
					b = (uint8_t)(55 + (rand() % 200));
					bool color_available = true;

					for (uint32_t i = 0; i < province_count; i++) {
						PixelProv* province = &provinces[i];
						ROSE_RGBA curr = province->color;
						if ((r == curr.r) && (g == curr.g) && (b == curr.b)) {
							color_available = false;
							break;
						}
					}

					if (color_available)
						break;
				}

				PixelProv province = (PixelProv){
					.color = (ROSE_RGBA) { r, g, b, 255 },
					.pixel_count = 1,
					.frontier_size = 0,
					.frontier_capacity = 32,
				};

				ROSE_ImageSetPixelRGBA(&image, x, y, province.color);
				province.frontier = malloc(province.frontier_capacity * sizeof(Vec2));
				province.frontier[province.frontier_size++] = (Vec2){ x, y };
				provinces[province_count++] = province;
			}
		}

		const int32_t neighbour_offsets[4][2] = {
			{  0, -1 },
			{  0,  1 },
			{ -1,  0 },
			{  1,  0 }
		};

		uint32_t active_count = province_count;
		while (active_count > 0) {
			active_count = 0;

			for (uint32_t p = 0; p < province_count; p++) {
				PixelProv* province = &provinces[p];

				while (province->frontier_size > 0) {
					uint32_t index = rand() % province->frontier_size;
					Vec2 position = province->frontier[index];

					uint32_t open_directions[4];
					uint32_t open_count = 0;
					for (uint32_t i = 0; i < 4; i++) {
						uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
						uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
						if ((next_xpos >= width) || (next_ypos >= width))
							continue;

						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, next_xpos, next_ypos);
						bool is_land = (color.g == 255);
						if (is_land) open_directions[open_count++] = i;
					}

					if (open_count == 0) {
						province->frontier[index] = province->frontier[--province->frontier_size];
						continue;
					}

					uint32_t direction = open_directions[(uint32_t)rand() % open_count];
					uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[direction][0];
					uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[direction][1];

					if (province->frontier_size == province->frontier_capacity) {
						province->frontier_capacity *= 2;
						Vec2* new_frontier = realloc(province->frontier, province->frontier_capacity * sizeof(Vec2));
						province->frontier = new_frontier;
					}

					ROSE_ImageSetPixelRGBA(&image, next_xpos, next_ypos, province->color);
					province->pixel_count++;
					province->frontier[province->frontier_size++] = (Vec2){ next_xpos, next_ypos };
					break;
				}

				if (province->frontier_size > 0)
					active_count++;
			}
		}

		uint32_t pixel_sum = 0;
		for (uint32_t p = 0; p < province_count; p++) {
			pixel_sum += provinces[p].pixel_count;
		}

		uint32_t mean_pixels = pixel_sum / province_count;
		for (uint32_t p = 0; p < province_count; p++) {
			PixelProv* province = &provinces[p];
			if (province->pixel_count < (mean_pixels / 2)) {
				for (uint32_t y = 0; y < width; y++) {
					for (uint32_t x = 0; x < width; x++) {
						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
						ROSE_RGBA pcolor = province->color;
						bool is_province = (color.r == pcolor.r) && (color.g == pcolor.g) && (color.b == pcolor.b);
						if (!is_province) continue;
						ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
					}
				}
			}
		}

		for (uint32_t p = 0; p < province_count; p++) {
			free(provinces[p].frontier);
		} free(provinces);

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_land = (color.g == 255);
				if (!is_land) continue;
				ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
			}
		}
	}

	return image;
}

#endif /* MAP_GENERATOR_REFACTOR */

