
/*
 *   Header [png.h]
 */

#ifndef PNG_DECODER_ENCODER_HEADER_GUARD
#define PNG_DECODER_ENCODER_HEADER_GUARD

/***************** Light-Weight Zero-Dependency PNG Decoder & Encoder (C99) *****************
 * 
 * Both 'png_decode()' and 'png_encode()' return zero on success.
 * A nonzero return code may be passed to 'png_error()' to receive an error description.
 * On failure, outputs are left unmodified.
 * 
 * Decoder only supports 8-bit non-interlaced RGB or RGBA images.
 * Caller is responsible for freeing memory allocated in the returned image (using 'free()').
 * Encoder writes an image as 8-bit RGBA, overwriting any existing file at 'path'.
 * 
 * Position (0, 0) is the image's top-left corner.
 * Images are stored in memory such that indexing is done using '(y * width) + x'.
 * Each pixel is an unsigned 32 bit integer in the form '0xRRGGBBAA'.
 * 
 */

#include <stdint.h>

int png_decode(const char* path, uint32_t** pixels, uint32_t* width, uint32_t* height);
int png_encode(const char* path, uint32_t* pixels, uint32_t width, uint32_t height);
const char* png_error(int retcode);

#endif /* PNG_DECODER_ENCODER_HEADER_GUARD */

