
/*
 *   Source [png.c]
 */

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PNG_HUFFMAN_FAST_BITS 9
#define PNG_HUFFMAN_MAX_BITS 15

#define PNG_ENCODE_HASH_BITS   15
#define PNG_ENCODE_HASH_SIZE   (1 << PNG_ENCODE_HASH_BITS)
#define PNG_ENCODE_WINDOW_SIZE 32768
#define PNG_ENCODE_MIN_MATCH   3
#define PNG_ENCODE_MAX_MATCH   258
#define PNG_ENCODE_IDAT_SIZE   (1u << 20)

enum {
	PNG_SUCCESS = 0,
	PNG_ERROR_OUT_OF_MEMORY,
	PNG_ERROR_INVALID_ARGUMENTS,
	PNG_ERROR_FILE_CANT_OPEN,
	PNG_ERROR_FILE_CANT_READ,
	PNG_ERROR_BAD_SIGNATURE,
	PNG_ERROR_UNSUPPORTED,
	PNG_ERROR_TOO_LARGE,
	PNG_ERROR_CORRUPT,
	PNG_ERROR_BAD_CRC,
	PNG_ERROR_BAD_ADLER,
	PNG_ERROR_FILE_CANT_WRITE,
	PNG_ERROR_ZERO_DIMENSIONS,
};

typedef struct {
	uint16_t fast[1 << PNG_HUFFMAN_FAST_BITS];
	uint16_t count[PNG_HUFFMAN_MAX_BITS + 1];
	uint16_t symbol[288];
} PNG_Huffman;

typedef struct {
	const uint8_t* data;
	size_t size;
	size_t pos;
	uint32_t bitbuf;
	int bitcount;
} PNG_BitReader;

typedef struct {
	uint8_t* data;
	size_t size;
	size_t capacity;
	bool failed;
} PNG_ByteBuffer;

typedef struct {
	PNG_ByteBuffer* buffer;
	uint32_t bitbuf;
	int bitcount;
} PNG_BitWriter;

static const uint8_t  png_decode_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
static const uint16_t png_decode_len_base[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
static const uint8_t  png_decode_len_extra[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
static const uint16_t png_decode_dist_base[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
static const uint8_t  png_decode_dist_extra[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

static uint32_t png_decode_read_be32(const uint8_t*);
static void     png_decode_crc_table_init(uint32_t[256]);
static uint32_t png_decode_crc32(const uint32_t[256], const uint8_t*, size_t);
static uint32_t png_decode_adler32(const uint8_t*, size_t);
static void     png_decode_br_refill(PNG_BitReader*);
static uint32_t png_decode_br_bits(PNG_BitReader*, int);
static bool     png_decode_br_overrun(const PNG_BitReader*);
static bool     png_decode_huff_build(PNG_Huffman*, const uint8_t*, int);
static int      png_decode_huff_decode(PNG_BitReader*, const PNG_Huffman*);
static int      png_decode_inflate_codes(PNG_BitReader*, const PNG_Huffman*, const PNG_Huffman*, uint8_t*, size_t, size_t*);
static int      png_decode_inflate_dynamic_tables(PNG_BitReader*, PNG_Huffman*, PNG_Huffman*);
static int      png_decode_zlib_decompress(const uint8_t*, size_t, uint8_t*, size_t);
static uint8_t  png_decode_paeth(int, int, int);
static int      png_decode_unfilter(uint8_t*, uint32_t, size_t, size_t);
static int      png_decode_read_file(const char*, uint8_t**, size_t*);

static void     png_encode_push(PNG_ByteBuffer*, const uint8_t*, size_t);
static void     png_encode_push_byte(PNG_ByteBuffer*, uint8_t);
static void     png_encode_push_be32(PNG_ByteBuffer*, uint32_t);
static void     png_encode_chunk(PNG_ByteBuffer*, const uint32_t[256], const char*, const uint8_t*, uint32_t);
static void     png_encode_put_bits(PNG_BitWriter*, uint32_t, int);
static void     png_encode_flush_bits(PNG_BitWriter*);
static uint32_t png_encode_reverse(uint32_t, int);
static void     png_encode_put_symbol(PNG_BitWriter*, int);
static void     png_encode_put_match(PNG_BitWriter*, size_t, size_t);
static uint32_t png_encode_hash(const uint8_t*);
static void     png_encode_deflate(PNG_BitWriter*, const uint8_t*, size_t, size_t*);
static uint8_t  png_encode_filter_byte(int, int, int, int, int);
static void     png_encode_filter_row(uint8_t*, const uint8_t*, const uint8_t*, size_t);

/*
 *   API
 */

int png_decode(const char* path, uint32_t** pixels_result, uint32_t* width_result, uint32_t* height_result)
{
	if (!path || !pixels_result || !width_result || !height_result) {
		return PNG_ERROR_INVALID_ARGUMENTS;
	}

	uint8_t* data = NULL;
	uint8_t* idat = NULL;
	uint8_t* raw = NULL;
	uint32_t* pixels = NULL;
	size_t size = 0;

	int status = png_decode_read_file(path, &data, &size);
	if (status != PNG_SUCCESS) {
		return status;
	}

	if (size < 8 || memcmp(data, png_decode_signature, 8) != 0) {
		status = PNG_ERROR_BAD_SIGNATURE;
		goto cleanup;
	}

	uint32_t crc_table[256];
	png_decode_crc_table_init(crc_table);
	idat = (uint8_t*)malloc(size);
	if (!idat) {
		status = PNG_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}

	uint32_t width = 0, height = 0, channels = 0;
	size_t idat_size = 0;
	bool seen_ihdr = false, seen_idat = false, idat_ended = false, seen_iend = false;
	size_t pos = 8;

	while (!seen_iend) {
		if (size - pos < 12) {
			status = PNG_ERROR_CORRUPT;
			goto cleanup;
		}

		uint32_t len = png_decode_read_be32(data + pos);
		const uint8_t* type = data + pos + 4;
		if (len > 0x7FFFFFFFu || len > size - pos - 12) {
			status = PNG_ERROR_CORRUPT;
			goto cleanup;
		}

		const uint8_t* chunk = data + pos + 8;
		for (int i = 0; i < 4; i++) {
			uint8_t ch = type[i];
			if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))) {
				status = PNG_ERROR_CORRUPT;
				goto cleanup;
			}
		}

		if (png_decode_crc32(crc_table, type, (size_t)len + 4) != png_decode_read_be32(chunk + len)) {
			status = PNG_ERROR_BAD_CRC;
			goto cleanup;
		}

		bool is_idat = memcmp(type, "IDAT", 4) == 0;
		if (seen_idat && !is_idat) {
			idat_ended = true;
		}

		if (!seen_ihdr && memcmp(type, "IHDR", 4) != 0) {
			status = PNG_ERROR_CORRUPT;
			goto cleanup;
		}

		if (memcmp(type, "IHDR", 4) == 0) {
			if (seen_ihdr || len != 13) {
				status = PNG_ERROR_CORRUPT;
				goto cleanup;
			}

			seen_ihdr = true;
			width = png_decode_read_be32(chunk);
			height = png_decode_read_be32(chunk + 4);
			uint8_t bit_depth = chunk[8];
			uint8_t color_type = chunk[9];
			uint8_t compression = chunk[10];
			uint8_t filter = chunk[11];
			uint8_t interlace = chunk[12];

			if (width == 0 || height == 0 || width > 0x7FFFFFFFu || height > 0x7FFFFFFFu) {
				status = PNG_ERROR_CORRUPT;
				goto cleanup;
			}

			if (bit_depth != 8 || compression != 0 || filter != 0 || interlace != 0) {
				status = PNG_ERROR_UNSUPPORTED;
				goto cleanup;
			}

			if (color_type == 6) {
				channels = 4;
			}
			else if (color_type == 2) {
				channels = 3;
			}
			else {
				status = PNG_ERROR_UNSUPPORTED;
				goto cleanup;
			}
		}
		else if (is_idat) {
			if (idat_ended) {
				status = PNG_ERROR_CORRUPT;
				goto cleanup;
			}

			seen_idat = true;
			memcpy(idat + idat_size, chunk, len);
			idat_size += len;
		}
		else if (memcmp(type, "IEND", 4) == 0) {
			if (len != 0) {
				status = PNG_ERROR_CORRUPT;
				goto cleanup;
			} seen_iend = true;
		}
		else if (memcmp(type, "PLTE", 4) == 0) {}
		else if (!(type[0] & 0x20)) {
			status = PNG_ERROR_UNSUPPORTED;
			goto cleanup;
		}

		pos += (size_t)len + 12;
	}

	if (!seen_idat) {
		status = PNG_ERROR_CORRUPT;
		goto cleanup;
	}

	uint64_t stride64 = (uint64_t)width * channels;
	uint64_t raw_size64 = (stride64 + 1) * height;
	uint64_t pixel_count = (uint64_t)width * height;
	if (raw_size64 > SIZE_MAX || pixel_count > SIZE_MAX / sizeof(uint32_t)) {
		status = PNG_ERROR_TOO_LARGE;
		goto cleanup;
	}

	if (raw_size64 / 1032 > idat_size) {
		status = PNG_ERROR_CORRUPT;
		goto cleanup;
	}

	size_t stride = (size_t)stride64;
	size_t raw_size = (size_t)raw_size64;

	raw = (uint8_t*)malloc(raw_size);
	pixels = (uint32_t*)malloc((size_t)pixel_count * sizeof(uint32_t));
	if (!raw || !pixels) {
		status = PNG_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}

	status = png_decode_zlib_decompress(idat, idat_size, raw, raw_size);
	if (status != PNG_SUCCESS) {
		goto cleanup;
	}

	status = png_decode_unfilter(raw, height, stride, channels);
	if (status != PNG_SUCCESS) {
		goto cleanup;
	}

	for (uint32_t y = 0; y < height; y++) {
		const uint8_t* src = raw + (size_t)y * (stride + 1) + 1;
		uint32_t* dst = pixels + (size_t)y * width;
		if (channels == 4) {
			for (uint32_t x = 0; x < width; x++, src += 4) {
				dst[x] = ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) | ((uint32_t)src[2] << 8) | (uint32_t)src[3];
			}
		}
		else {
			for (uint32_t x = 0; x < width; x++, src += 3) {
				dst[x] = ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) | ((uint32_t)src[2] << 8) | 0xFFu;
			}
		}
	}

	*pixels_result = pixels;
	*width_result = width;
	*height_result = height;
	pixels = NULL;

cleanup:
	free(data);
	free(idat);
	free(raw);
	free(pixels);
	return status;
}

int png_encode(const char* path, uint32_t* pixels, uint32_t width, uint32_t height)
{
	if (!path || !pixels) {
		return PNG_ERROR_INVALID_ARGUMENTS;
	}

	if (width == 0 || height == 0) {
		return PNG_ERROR_ZERO_DIMENSIONS;
	}

	if (width > 0x7FFFFFFFu || height > 0x7FFFFFFFu) {
		return PNG_ERROR_TOO_LARGE;
	}

	uint64_t stride64 = (uint64_t)width * 4;
	uint64_t raw_size64 = (stride64 + 1) * height;
	if (raw_size64 > SIZE_MAX) {
		return PNG_ERROR_TOO_LARGE;
	}

	size_t stride = (size_t)stride64;
	size_t raw_size = (size_t)raw_size64;

	int status = PNG_SUCCESS;
	PNG_ByteBuffer zlib = { 0 };
	PNG_ByteBuffer file = { 0 };
	uint8_t* raw = (uint8_t*)malloc(raw_size);
	uint8_t* rows = (uint8_t*)malloc(stride * 2);
	size_t* hash_table = (size_t*)calloc(PNG_ENCODE_HASH_SIZE, sizeof(size_t));
	if (!raw || !rows || !hash_table) {
		status = PNG_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}

	uint8_t* line = rows;
	uint8_t* prior = NULL;
	for (uint32_t y = 0; y < height; y++) {
		const uint32_t* src = pixels + (size_t)y * width;
		for (uint32_t x = 0; x < width; x++) {
			uint32_t pixel = src[x];
			line[(size_t)x * 4 + 0] = (uint8_t)(pixel >> 24);
			line[(size_t)x * 4 + 1] = (uint8_t)(pixel >> 16);
			line[(size_t)x * 4 + 2] = (uint8_t)(pixel >> 8);
			line[(size_t)x * 4 + 3] = (uint8_t)pixel;
		}

		png_encode_filter_row(raw + (size_t)y * (stride + 1), line, prior, stride);
		prior = line;
		line = (line == rows) ? rows + stride : rows;
	}

	PNG_BitWriter bw = { &zlib, 0, 0 };
	png_encode_push_byte(&zlib, 0x78);
	png_encode_push_byte(&zlib, 0x01);
	png_encode_deflate(&bw, raw, raw_size, hash_table);
	png_encode_flush_bits(&bw);
	png_encode_push_be32(&zlib, png_decode_adler32(raw, raw_size));
	if (zlib.failed) {
		status = PNG_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}

	uint32_t crc_table[256];
	png_decode_crc_table_init(crc_table);

	uint8_t ihdr[13] = {
		(uint8_t)(width >> 24), (uint8_t)(width >> 16), (uint8_t)(width >> 8), (uint8_t)width,
		(uint8_t)(height >> 24), (uint8_t)(height >> 16), (uint8_t)(height >> 8), (uint8_t)height,
		8, 6, 0, 0, 0,
	};

	png_encode_push(&file, png_decode_signature, 8);
	png_encode_chunk(&file, crc_table, "IHDR", ihdr, 13);
	for (size_t offset = 0; offset < zlib.size; offset += PNG_ENCODE_IDAT_SIZE) {
		size_t len = zlib.size - offset;
		if (len > PNG_ENCODE_IDAT_SIZE) {
			len = PNG_ENCODE_IDAT_SIZE;
		}

		png_encode_chunk(&file, crc_table, "IDAT", zlib.data + offset, (uint32_t)len);
	}

	png_encode_chunk(&file, crc_table, "IEND", NULL, 0);
	if (file.failed) {
		status = PNG_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}

	FILE* f = fopen(path, "wb");
	if (!f) {
		status = PNG_ERROR_FILE_CANT_OPEN;
		goto cleanup;
	}

	size_t written = fwrite(file.data, 1, file.size, f);
	int close_result = fclose(f);
	if (written != file.size || close_result != 0) {
		remove(path);
		status = PNG_ERROR_FILE_CANT_WRITE;
	}

cleanup:
	free(raw);
	free(rows);
	free(hash_table);
	free(zlib.data);
	free(file.data);
	return status;
}

const char* png_error(int status)
{
	switch (status) {
	case (PNG_SUCCESS):                 return "success";
	case (PNG_ERROR_OUT_OF_MEMORY):     return "out of memory";
	case (PNG_ERROR_INVALID_ARGUMENTS): return "invalid arguments (NULL pointer)";
	case (PNG_ERROR_FILE_CANT_OPEN):    return "could not open file";
	case (PNG_ERROR_FILE_CANT_READ):    return "could not read file";
	case (PNG_ERROR_BAD_SIGNATURE):     return "not a PNG file (bad signature)";
	case (PNG_ERROR_UNSUPPORTED):       return "unsupported PNG format (only 8-bit, non-interlaced RGB or RGBA is supported)";
	case (PNG_ERROR_TOO_LARGE):         return "image dimensions too large";
	case (PNG_ERROR_CORRUPT):           return "corrupt or malformed PNG data";
	case (PNG_ERROR_BAD_CRC):           return "chunk CRC mismatch (corrupt file)";
	case (PNG_ERROR_BAD_ADLER):         return "zlib Adler-32 mismatch (corrupt image data)";
	case (PNG_ERROR_FILE_CANT_WRITE):   return "could not write file";
	case (PNG_ERROR_ZERO_DIMENSIONS):   return "image width and height must be nonzero";
	default: return "unknown";
	}
}

/*
 *   Decode Internal
 */

static uint32_t png_decode_read_be32(const uint8_t* p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void png_decode_crc_table_init(uint32_t table[256])
{
	for (uint32_t n = 0; n < 256; n++) {
		uint32_t c = n;
		for (int k = 0; k < 8; k++) {
			c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
		}

		table[n] = c;
	}
}

static uint32_t png_decode_crc32(const uint32_t table[256], const uint8_t* data, size_t len)
{
	uint32_t c = 0xFFFFFFFFu;
	for (size_t i = 0; i < len; i++) {
		c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
	}

	return c ^ 0xFFFFFFFFu;
}

static uint32_t png_decode_adler32(const uint8_t* data, size_t len)
{
	uint32_t a = 1, b = 0;
	while (len > 0) {
		size_t block = len < 5552 ? len : 5552;
		len -= block;
		while (block--) {
			a += *data++;
			b += a;
		}

		a %= 65521;
		b %= 65521;
	}

	return (b << 16) | a;
}

static void png_decode_br_refill(PNG_BitReader* br)
{
	while (br->bitcount <= 24) {
		uint32_t byte = (br->pos < br->size) ? br->data[br->pos] : 0;
		br->pos++;
		br->bitbuf |= byte << br->bitcount;
		br->bitcount += 8;
	}
}

static uint32_t png_decode_br_bits(PNG_BitReader* br, int n)
{
	if (n == 0) {
		return 0;
	}

	png_decode_br_refill(br);
	uint32_t v = br->bitbuf & ((1u << n) - 1);
	br->bitbuf >>= n;
	br->bitcount -= n;
	return v;
}

static bool png_decode_br_overrun(const PNG_BitReader* br)
{
	return (uint64_t)br->pos * 8 - (uint64_t)br->bitcount > (uint64_t)br->size * 8;
}

static bool png_decode_huff_build(PNG_Huffman* h, const uint8_t* lengths, int n)
{
	uint16_t offs[PNG_HUFFMAN_MAX_BITS + 2] = { 0 };
	uint32_t next_code[PNG_HUFFMAN_MAX_BITS + 1] = { 0 };

	memset(h, 0, sizeof(*h));
	for (int i = 0; i < n; i++) {
		h->count[lengths[i]]++;
	} h->count[0] = 0;

	int left = 1;
	for (int len = 1; len <= PNG_HUFFMAN_MAX_BITS; len++) {
		left <<= 1;
		left -= h->count[len];
		if (left < 0) {
			return false;
		}
	}

	offs[1] = 0;
	for (int len = 1; len <= PNG_HUFFMAN_MAX_BITS; len++) {
		offs[len + 1] = offs[len] + h->count[len];
	}

	for (int i = 0; i < n; i++) {
		if (lengths[i]) {
			h->symbol[offs[lengths[i]]++] = (uint16_t)i;
		}
	}

	uint32_t code = 0;
	next_code[0] = 0;
	for (int len = 1; len <= PNG_HUFFMAN_MAX_BITS; len++) {
		code = (code + h->count[len - 1]) << 1;
		next_code[len] = code;
	}

	for (int i = 0; i < n; i++) {
		int len = lengths[i];
		if (len == 0 || len > PNG_HUFFMAN_FAST_BITS) {
			if (len) {
				next_code[len]++;
			}

			continue;
		}

		uint32_t c = next_code[len]++;
		uint32_t rev = 0;

		for (int b = 0; b < len; b++) {
			rev |= ((c >> b) & 1) << (len - 1 - b);
		}

		for (uint32_t fill = rev; fill < (1u << PNG_HUFFMAN_FAST_BITS); fill += (1u << len)) {
			h->fast[fill] = (uint16_t)((len << PNG_HUFFMAN_FAST_BITS) | i);
		}
	}

	return true;
}

static int png_decode_huff_decode(PNG_BitReader* br, const PNG_Huffman* h)
{
	png_decode_br_refill(br);

	uint16_t e = h->fast[br->bitbuf & ((1u << PNG_HUFFMAN_FAST_BITS) - 1)];
	if (e) {
		int len = e >> PNG_HUFFMAN_FAST_BITS;
		br->bitbuf >>= len;
		br->bitcount -= len;
		return e & ((1u << PNG_HUFFMAN_FAST_BITS) - 1);
	}

	int code = 0, first = 0, index = 0;
	for (int len = 1; len <= PNG_HUFFMAN_MAX_BITS; len++) {
		code |= (br->bitbuf >> (len - 1)) & 1;
		int count = h->count[len];

		if (code - count < first) {
			br->bitbuf >>= len;
			br->bitcount -= len;
			return h->symbol[index + (code - first)];
		}

		index += count;
		first += count;
		first <<= 1;
		code <<= 1;
	}

	return -1;
}

static int png_decode_inflate_codes(PNG_BitReader* br, const PNG_Huffman* lit, const PNG_Huffman* dist, uint8_t* out, size_t out_size, size_t* out_pos)
{
	size_t o = *out_pos;
	for (;;) {
		int sym = png_decode_huff_decode(br, lit);
		if (sym < 0 || png_decode_br_overrun(br)) {
			return PNG_ERROR_CORRUPT;
		}

		if (sym < 256) {
			if (o >= out_size) {
				return PNG_ERROR_CORRUPT;
			} out[o++] = (uint8_t)sym;
		}
		else if (sym == 256) {
			break;
		}
		else {
			sym -= 257;
			if (sym >= 29) {
				return PNG_ERROR_CORRUPT;
			}

			size_t len = png_decode_len_base[sym] + png_decode_br_bits(br, png_decode_len_extra[sym]);
			int dsym = png_decode_huff_decode(br, dist);

			if (dsym < 0 || dsym >= 30) {
				return PNG_ERROR_CORRUPT;
			}

			size_t d = png_decode_dist_base[dsym] + png_decode_br_bits(br, png_decode_dist_extra[dsym]);

			if (png_decode_br_overrun(br) || d > o || len > out_size - o) {
				return PNG_ERROR_CORRUPT;
			}

			const uint8_t* src = out + o - d;
			uint8_t* dst = out + o;
			for (size_t i = 0; i < len; i++) {
				dst[i] = src[i];
			} o += len;
		}
	}

	*out_pos = o;
	return PNG_SUCCESS;
}

static int png_decode_inflate_dynamic_tables(PNG_BitReader* br, PNG_Huffman* lit, PNG_Huffman* dist)
{
	static const uint8_t order[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
	uint8_t lengths[286 + 30] = { 0 };
	uint8_t cl_lengths[19] = { 0 };
	PNG_Huffman cl;

	int hlit = (int)png_decode_br_bits(br, 5) + 257;
	int hdist = (int)png_decode_br_bits(br, 5) + 1;
	int hclen = (int)png_decode_br_bits(br, 4) + 4;
	if (hlit > 286 || hdist > 30) {
		return PNG_ERROR_CORRUPT;
	}

	for (int i = 0; i < hclen; i++) {
		cl_lengths[order[i]] = (uint8_t)png_decode_br_bits(br, 3);
	}

	if (!png_decode_huff_build(&cl, cl_lengths, 19)) {
		return PNG_ERROR_CORRUPT;
	}

	int total = hlit + hdist;
	int i = 0;
	while (i < total) {
		int sym = png_decode_huff_decode(br, &cl);
		if (sym < 0 || png_decode_br_overrun(br)) {
			return PNG_ERROR_CORRUPT;
		}

		if (sym < 16) {
			lengths[i++] = (uint8_t)sym;
			continue;
		}

		uint8_t value = 0;
		int repeat;
		if (sym == 16) {
			if (i == 0) {
				return PNG_ERROR_CORRUPT;
			}

			value = lengths[i - 1];
			repeat = 3 + (int)png_decode_br_bits(br, 2);
		}
		else if (sym == 17) {
			repeat = 3 + (int)png_decode_br_bits(br, 3);
		}
		else {
			repeat = 11 + (int)png_decode_br_bits(br, 7);
		}

		if (repeat > total - i) {
			return PNG_ERROR_CORRUPT;
		}

		memset(lengths + i, value, (size_t)repeat);
		i += repeat;
	}

	if (png_decode_br_overrun(br)) {
		return PNG_ERROR_CORRUPT;
	}

	if (lengths[256] == 0) {
		return PNG_ERROR_CORRUPT;
	}

	if (!png_decode_huff_build(lit, lengths, hlit)) {
		return PNG_ERROR_CORRUPT;
	}

	if (!png_decode_huff_build(dist, lengths + hlit, hdist)) {
		return PNG_ERROR_CORRUPT;
	}

	return PNG_SUCCESS;
}

static int png_decode_zlib_decompress(const uint8_t* in, size_t in_size, uint8_t* out, size_t out_size)
{
	if (in_size < 2 + 4) {
		return PNG_ERROR_CORRUPT;
	}

	uint8_t cmf = in[0], flg = in[1];
	if ((cmf & 0x0F) != 8 || (cmf >> 4) > 7) {
		return PNG_ERROR_CORRUPT;
	}

	if (((uint32_t)cmf * 256 + flg) % 31 != 0) {
		return PNG_ERROR_CORRUPT;
	}

	if (flg & 0x20) {
		return PNG_ERROR_CORRUPT;
	}

	PNG_BitReader br = { in, in_size, 2, 0, 0 };
	PNG_Huffman* lit = (PNG_Huffman*)malloc(sizeof(PNG_Huffman));
	PNG_Huffman* dist = (PNG_Huffman*)malloc(sizeof(PNG_Huffman));
	if (!lit || !dist) {
		free(lit);
		free(dist);
		return PNG_ERROR_OUT_OF_MEMORY;
	}

	int status = PNG_SUCCESS;
	size_t o = 0;
	bool final_block = false;

	while (!final_block) {
		final_block = png_decode_br_bits(&br, 1) != 0;
		uint32_t type = png_decode_br_bits(&br, 2);
		if (png_decode_br_overrun(&br)) {
			status = PNG_ERROR_CORRUPT;
			break;
		}

		if (type == 0) {
			png_decode_br_bits(&br, br.bitcount & 7);
			size_t p = br.pos - (size_t)(br.bitcount / 8);
			br.bitbuf = 0;
			br.bitcount = 0;
			if (p > in_size || in_size - p < 4) {
				status = PNG_ERROR_CORRUPT;
				break;
			}

			uint32_t len = (uint32_t)in[p] | ((uint32_t)in[p + 1] << 8);
			uint32_t nlen = (uint32_t)in[p + 2] | ((uint32_t)in[p + 3] << 8);
			p += 4;
			if (len != (~nlen & 0xFFFF) || len > in_size - p || len > out_size - o) {
				status = PNG_ERROR_CORRUPT;
				break;
			}

			memcpy(out + o, in + p, len);
			o += len;
			br.pos = p + len;
		}
		else if (type == 1) {
			uint8_t lengths[288];
			memset(lengths, 8, 144);
			memset(lengths + 144, 9, 112);
			memset(lengths + 256, 7, 24);
			memset(lengths + 280, 8, 8);
			png_decode_huff_build(lit, lengths, 288);
			memset(lengths, 5, 30);
			png_decode_huff_build(dist, lengths, 30);
			status = png_decode_inflate_codes(&br, lit, dist, out, out_size, &o);
		}
		else if (type == 2) {
			status = png_decode_inflate_dynamic_tables(&br, lit, dist);
			if (status == PNG_SUCCESS) {
				status = png_decode_inflate_codes(&br, lit, dist, out, out_size, &o);
			}
		}
		else {
			status = PNG_ERROR_CORRUPT;
		}

		if (status != PNG_SUCCESS) {
			break;
		}
	}

	free(lit);
	free(dist);
	if (status != PNG_SUCCESS) {
		return status;
	}

	if (o != out_size) {
		return PNG_ERROR_CORRUPT;
	}

	png_decode_br_bits(&br, br.bitcount & 7);
	size_t p = br.pos - (size_t)(br.bitcount / 8);
	if (p > in_size || in_size - p < 4) {
		return PNG_ERROR_CORRUPT;
	}

	if (png_decode_read_be32(in + p) != png_decode_adler32(out, out_size)) {
		return PNG_ERROR_BAD_ADLER;
	}

	return PNG_SUCCESS;
}

static uint8_t png_decode_paeth(int a, int b, int c)
{
	int p = a + b - c;
	int pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
	if (pa <= pb && pa <= pc) {
		return (uint8_t)a;
	}

	if (pb <= pc) {
		return (uint8_t)b;
	}

	return (uint8_t)c;
}

static int png_decode_unfilter(uint8_t* raw, uint32_t height, size_t stride, size_t bpp)
{
	const uint8_t* prior = NULL;
	for (uint32_t y = 0; y < height; y++) {
		uint8_t* row = raw + (size_t)y * (stride + 1);
		uint8_t filter = row[0];
		uint8_t* line = row + 1;

		switch (filter) {
		case 0: break;
		case 1:
			for (size_t i = bpp; i < stride; i++) {
				line[i] = (uint8_t)(line[i] + line[i - bpp]);
			}
			break;
		case 2:
			if (prior) {
				for (size_t i = 0; i < stride; i++) {
					line[i] = (uint8_t)(line[i] + prior[i]);
				}
			}
			break;
		case 3:
			for (size_t i = 0; i < stride; i++) {
				int a = (i >= bpp) ? line[i - bpp] : 0;
				int b = prior ? prior[i] : 0;
				line[i] = (uint8_t)(line[i] + ((a + b) >> 1));
			}
			break;
		case 4:
			for (size_t i = 0; i < stride; i++) {
				int a = (i >= bpp) ? line[i - bpp] : 0;
				int b = prior ? prior[i] : 0;
				int c = (prior && i >= bpp) ? prior[i - bpp] : 0;
				line[i] = (uint8_t)(line[i] + png_decode_paeth(a, b, c));
			}
			break;
		default: return PNG_ERROR_CORRUPT;
		}

		prior = line;
	}

	return PNG_SUCCESS;
}

static int png_decode_read_file(const char* path, uint8_t** data_out, size_t* size_out)
{
	FILE* f = fopen(path, "rb");
	if (!f) {
		return PNG_ERROR_FILE_CANT_OPEN;
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return PNG_ERROR_FILE_CANT_READ;
	}

	long size = ftell(f);
	if (size < 0 || fseek(f, 0, SEEK_SET) != 0) {
		fclose(f);
		return PNG_ERROR_FILE_CANT_READ;
	}

	uint8_t* data = (uint8_t*)malloc(size > 0 ? (size_t)size : 1);
	if (!data) {
		fclose(f);
		return PNG_ERROR_OUT_OF_MEMORY;
	}

	if (fread(data, 1, (size_t)size, f) != (size_t)size) {
		free(data);
		fclose(f);
		return PNG_ERROR_FILE_CANT_READ;
	}

	fclose(f);
	*data_out = data;
	*size_out = (size_t)size;
	return PNG_SUCCESS;
}

/*
 *   Encode Internal
 */

static void png_encode_push(PNG_ByteBuffer* buffer, const uint8_t* bytes, size_t n)
{
	if (buffer->failed || n == 0) {
		return;
	}

	if (n > buffer->capacity - buffer->size) {
		size_t capacity = buffer->capacity ? buffer->capacity : 4096;
		while (capacity - buffer->size < n) {
			if (capacity > SIZE_MAX / 2) {
				buffer->failed = true;
				return;
			}

			capacity *= 2;
		}

		uint8_t* data = (uint8_t*)realloc(buffer->data, capacity);
		if (!data) {
			buffer->failed = true;
			return;
		}

		buffer->data = data;
		buffer->capacity = capacity;
	}

	memcpy(buffer->data + buffer->size, bytes, n);
	buffer->size += n;
}

static void png_encode_push_byte(PNG_ByteBuffer* buffer, uint8_t byte)
{
	png_encode_push(buffer, &byte, 1);
}

static void png_encode_push_be32(PNG_ByteBuffer* buffer, uint32_t value)
{
	uint8_t bytes[4] = { (uint8_t)(value >> 24), (uint8_t)(value >> 16), (uint8_t)(value >> 8), (uint8_t)value };
	png_encode_push(buffer, bytes, 4);
}

static void png_encode_chunk(PNG_ByteBuffer* buffer, const uint32_t crc_table[256], const char* type, const uint8_t* data, uint32_t len)
{
	png_encode_push_be32(buffer, len);
	size_t start = buffer->size;
	png_encode_push(buffer, (const uint8_t*)type, 4);
	png_encode_push(buffer, data, len);
	if (buffer->failed) {
		return;
	}

	png_encode_push_be32(buffer, png_decode_crc32(crc_table, buffer->data + start, (size_t)len + 4));
}

static void png_encode_put_bits(PNG_BitWriter* bw, uint32_t bits, int n)
{
	bw->bitbuf |= bits << bw->bitcount;
	bw->bitcount += n;
	while (bw->bitcount >= 8) {
		png_encode_push_byte(bw->buffer, (uint8_t)bw->bitbuf);
		bw->bitbuf >>= 8;
		bw->bitcount -= 8;
	}
}

static void png_encode_flush_bits(PNG_BitWriter* bw)
{
	if (bw->bitcount > 0) {
		png_encode_push_byte(bw->buffer, (uint8_t)bw->bitbuf);
	}

	bw->bitbuf = 0;
	bw->bitcount = 0;
}

static uint32_t png_encode_reverse(uint32_t code, int len)
{
	uint32_t reversed = 0;
	for (int b = 0; b < len; b++) {
		reversed |= ((code >> b) & 1) << (len - 1 - b);
	}

	return reversed;
}

static void png_encode_put_symbol(PNG_BitWriter* bw, int symbol)
{
	uint32_t code;
	int len;
	if (symbol < 144) {
		code = 0x30u + (uint32_t)symbol;
		len = 8;
	}
	else if (symbol < 256) {
		code = 0x190u + (uint32_t)(symbol - 144);
		len = 9;
	}
	else if (symbol < 280) {
		code = (uint32_t)(symbol - 256);
		len = 7;
	}
	else {
		code = 0xC0u + (uint32_t)(symbol - 280);
		len = 8;
	}

	png_encode_put_bits(bw, png_encode_reverse(code, len), len);
}

static void png_encode_put_match(PNG_BitWriter* bw, size_t length, size_t distance)
{
	int li = 28;
	while (png_decode_len_base[li] > length) {
		li--;
	}

	png_encode_put_symbol(bw, 257 + li);
	png_encode_put_bits(bw, (uint32_t)(length - png_decode_len_base[li]), png_decode_len_extra[li]);

	int di = 29;
	while (png_decode_dist_base[di] > distance) {
		di--;
	}

	png_encode_put_bits(bw, png_encode_reverse((uint32_t)di, 5), 5);
	png_encode_put_bits(bw, (uint32_t)(distance - png_decode_dist_base[di]), png_decode_dist_extra[di]);
}

static uint32_t png_encode_hash(const uint8_t* p)
{
	uint32_t v = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[2];
	return (v * 2654435761u) >> (32 - PNG_ENCODE_HASH_BITS);
}

static void png_encode_deflate(PNG_BitWriter* bw, const uint8_t* data, size_t size, size_t* hash_table)
{
	png_encode_put_bits(bw, 1, 1);
	png_encode_put_bits(bw, 1, 2);

	size_t i = 0;
	while (i < size) {
		size_t best_len = 0;
		size_t best_dist = 0;
		if (size - i >= PNG_ENCODE_MIN_MATCH) {
			uint32_t h = png_encode_hash(data + i);
			size_t candidate = hash_table[h];
			hash_table[h] = i + 1;

			if (candidate != 0 && i - (candidate - 1) <= PNG_ENCODE_WINDOW_SIZE) {
				candidate--;
				size_t max_len = size - i;
				if (max_len > PNG_ENCODE_MAX_MATCH) {
					max_len = PNG_ENCODE_MAX_MATCH;
				}

				size_t len = 0;
				while (len < max_len && data[candidate + len] == data[i + len]) {
					len++;
				}

				if (len >= PNG_ENCODE_MIN_MATCH) {
					best_len = len;
					best_dist = i - candidate;
				}
			}
		}

		if (best_len) {
			png_encode_put_match(bw, best_len, best_dist);
			for (size_t k = 1; k < best_len; k++) {
				if (size - (i + k) >= PNG_ENCODE_MIN_MATCH) {
					hash_table[png_encode_hash(data + i + k)] = i + k + 1;
				}
			}

			i += best_len;
		}
		else {
			png_encode_put_symbol(bw, data[i]);
			i++;
		}
	}

	png_encode_put_symbol(bw, 256);
}

static uint8_t png_encode_filter_byte(int filter, int x, int a, int b, int c)
{
	switch (filter) {
	case 1:  return (uint8_t)(x - a);
	case 2:  return (uint8_t)(x - b);
	case 3:  return (uint8_t)(x - ((a + b) >> 1));
	case 4:  return (uint8_t)(x - png_decode_paeth(a, b, c));
	default: return (uint8_t)x;
	}
}

static void png_encode_filter_row(uint8_t* out, const uint8_t* line, const uint8_t* prior, size_t stride)
{
	int best_filter = 0;
	uint64_t best_score = UINT64_MAX;
	for (int filter = 0; filter < 5; filter++) {
		uint64_t score = 0;
		for (size_t i = 0; i < stride; i++) {
			int a = (i >= 4) ? line[i - 4] : 0;
			int b = prior ? prior[i] : 0;
			int c = (prior && i >= 4) ? prior[i - 4] : 0;
			uint8_t v = png_encode_filter_byte(filter, line[i], a, b, c);
			score += (v < 128) ? v : 256u - v;
		}

		if (score < best_score) {
			best_score = score;
			best_filter = filter;
		}
	}

	out[0] = (uint8_t)best_filter;
	for (size_t i = 0; i < stride; i++) {
		int a = (i >= 4) ? line[i - 4] : 0;
		int b = prior ? prior[i] : 0;
		int c = (prior && i >= 4) ? prior[i - 4] : 0;
		out[i + 1] = png_encode_filter_byte(best_filter, line[i], a, b, c);
	}
}

