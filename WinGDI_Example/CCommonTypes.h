
#ifndef _CCOMMONTYPES_H_
#define _CCOMMONTYPES_H_

#include <stdint.h>

namespace Types
{
	typedef struct {
		uint8_t magic[2];
	} bmpfile_magic_t;

	typedef struct {
		uint32_t filesz;
		uint16_t creator1;
		uint16_t creator2;
		uint32_t bmp_offset;
	} bmpfile_header_t;

	typedef struct {
		uint32_t header_sz;
		int32_t  width;
		int32_t  height;
		uint16_t nplanes;
		uint16_t bitspp;
		uint32_t compress_type;
		uint32_t bmp_bytesz;
		int32_t  hres;
		int32_t  vres;
		uint32_t ncolors;
		uint32_t nimpcolors;
	} bitmap_info_header_t;

	typedef struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t nothing;
	} rgb_t;

	// Use short int instead `unsigned char' so that we can
	// store negative values.
	typedef short int pixel_t;

	// current image processing state
	enum EImageState
	{
		EIMAGESTATE_FIRST = 0,
		EIMAGESTATE_CLEAN = EIMAGESTATE_FIRST,
		EIMAGESTATE_GAUSSIAN = 1,
		EIMAGESTATE_DISTANCE = 2,
		EIMAGESTATE_CANNY = 3,
		EIMAGESTATE_FINAL = 4,
	};


}
#endif //_CCOMMONTYPES_H_
