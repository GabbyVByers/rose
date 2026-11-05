
/*
 *   Header File [map_generator.h]
 */

#ifndef MAP_HEADER_GUARD
#define MAP_HEADER_GUARD

#include "rose.h"

typedef struct MapInfo {
	uint32_t map_width;
	float    map_sea_level;
	uint32_t perlin_seed;
	uint32_t perlin_divisions;
	uint32_t perlin_layers;
	float    perlin_attenuation;
	uint32_t province_spacing;
} MapInfo;

typedef struct Province {
	uint32_t  index;
	ROSE_RGBA color;
	uint32_t* neighbours;
	uint32_t  neighbour_count;
	ROSE_Mesh border;
	ROSE_Mesh triangles;
} Province;

typedef struct Provinces {
	Province* array;
	uint32_t count;
	uint32_t capacity;
} Provinces;

ROSE_Image MAP_GenerateImage(MapInfo* map_info);
Provinces  MAP_GenerateProvinces(ROSE_Image* image);

#endif /* MAP_HEADER_GUARD */

