//#include "stdafx.h"
#include "BitmapHandler.h"

// C99 doesn't define M_PI (GNU-C99 does)
#define M_PI 3.14159265358979323846264338327
#define MAX_BRIGHTNESS 255

CBitmapHandler::CBitmapHandler() :
	currentState(Types::EIMAGESTATE_FIRST),
	minThreshold(45),
	maxThreshold(50),
	sigmaValue(1.0f)

{
	// empty implentation for now
}


CBitmapHandler::~CBitmapHandler()
{
	if (bitmapImage != NULL)
	{
		delete[] bitmapImage;
		bitmapImage = NULL;
	}
	if (outputImage != NULL)
	{
		delete[] outputImage;
		outputImage = NULL;
	}
	if (G != NULL)
	{
		delete[] G;
		G = NULL;
	}
	if (after_Gx != NULL)
	{
		delete[] after_Gx;
		after_Gx = NULL;
	}
	if (after_Gy != NULL)
	{
		delete[] after_Gy;
		after_Gy = NULL;
	}
	if (nms != NULL)
	{
		delete[] nms;
		nms = NULL;
	}
}

bool
CBitmapHandler::load(char* filePath)
{
	bool retVal = false;

	FILE *filePtr = fopen(filePath, "rb");
	if (filePtr == NULL) {
		perror("fopen()");
		return NULL;
	}

	bmpfile_magic_t mag;
	if (fread(&mag, sizeof(bmpfile_magic_t), 1, filePtr) != 1) {
		fclose(filePtr);
		return NULL;
	}

	// verify that this is a bmp file by check bitmap id
	// warning: dereferencing type-punned pointer will break
	// strict-aliasing rules [-Wstrict-aliasing]
	if (*((uint16_t*)mag.magic) != 0x4D42) {
		fprintf(stderr, "Not a BMP file: magic=%c%c\n",
			mag.magic[0], mag.magic[1]);
		fclose(filePtr);
		return NULL;
	}

	bmpfile_header_t bitmapFileHeader; // our bitmap file header
									   // read the bitmap file header
	if (fread(&bitmapFileHeader, sizeof(bmpfile_header_t),
		1, filePtr) != 1) {
		fclose(filePtr);
		return NULL;
	}

	// read the bitmap info header
	if (fread(&bitmapInfoHeader, sizeof(bitmap_info_header_t),
		1, filePtr) != 1) {
		fclose(filePtr);
		return NULL;
	}

	if (bitmapInfoHeader.compress_type != 0)
		fprintf(stderr, "Warning, compression is not supported.\n");

	// move file point to the beginning of bitmap data
	if (fseek(filePtr, bitmapFileHeader.bmp_offset, SEEK_SET)) {
		fclose(filePtr);
		return NULL;
	}

	// allocate enough memory for the bitmap image data
	bitmapImage = new pixel_t[bitmapInfoHeader.bmp_bytesz]; //malloc(bitmapInfoHeader.bmp_bytesz * sizeof(pixel_t));
	outputImage = new pixel_t[bitmapInfoHeader.bmp_bytesz]; //(pixel_t*)malloc(bitmapInfoHeader.bmp_bytesz * sizeof(pixel_t));
	const int nx = bitmapInfoHeader.width;
	const int ny = bitmapInfoHeader.height;
	G = new pixel_t[nx * ny];
	after_Gx = new pixel_t[nx * ny];
	after_Gy = new pixel_t[nx * ny];
	nms = new pixel_t[nx * ny];

	// verify memory allocation
	if (bitmapImage == NULL) {
		fclose(filePtr);
		return NULL;
	}

	// read in the bitmap image data
	size_t pad, count = 0;
	unsigned char c;
	pad = 4 * ceil(bitmapInfoHeader.bitspp*bitmapInfoHeader.width / 32.) - bitmapInfoHeader.width;
	for (size_t i = 0; i<bitmapInfoHeader.height; i++) {
		for (size_t j = 0; j<bitmapInfoHeader.width; j++) {
			if (fread(&c, sizeof(unsigned char), 1, filePtr) != 1) {
				fclose(filePtr);
				return NULL;
			}
			bitmapImage[count++] = (pixel_t)c;
		}
		fseek(filePtr, pad, SEEK_CUR);
	}

	// If we were using unsigned char as pixel_t, then:
	// fread(bitmapImage, 1, bitmapInfoHeader->bmp_bytesz, filePtr);

	// close file and return bitmap image data
	fclose(filePtr);
	return bitmapImage;
}

bool
CBitmapHandler::save(char* filePath)
{
	FILE* filePtr = fopen(filePath, "wb");
	if (filePtr == NULL)
		return true;

	bmpfile_magic_t mag = { { 0x42, 0x4d } };
	if (fwrite(&mag, sizeof(bmpfile_magic_t), 1, filePtr) != 1) 
	{
		fclose(filePtr);
		return true;
	}

	const uint32_t offset = sizeof(bmpfile_magic_t) +
		sizeof(bmpfile_header_t) +
		sizeof(bitmap_info_header_t) +
		((1U << bitmapInfoHeader.bitspp) * 4);
	bmpfile_header_t bmp_fh;
	bmp_fh.filesz = offset + bitmapInfoHeader.bmp_bytesz;
	bmp_fh.creator1 = 0;
	bmp_fh.creator2 = 0;
	bmp_fh.bmp_offset = offset;

	if (fwrite(&bmp_fh, sizeof(bmpfile_header_t), 1, filePtr) != 1) 
	{
		fclose(filePtr);
		return true;
	}
	if (fwrite(&bitmapInfoHeader, sizeof(bitmap_info_header_t), 1, filePtr) != 1) {
		fclose(filePtr);
		return true;
	}

	// Palette
	for (size_t i = 0; i < (1U << bitmapInfoHeader.bitspp); i++) {
		const rgb_t color = { (uint8_t)i, (uint8_t)i, (uint8_t)i };
		if (fwrite(&color, sizeof(rgb_t), 1, filePtr) != 1) {
			fclose(filePtr);
			return true;
		}
	}

	// We use int instead of uchar, so we can't write img
	// in 1 call any more.
	// fwrite(data, 1, bmp_ih->bmp_bytesz, filePtr);

	// Padding: http://en.wikipedia.org/wiki/BMP_file_format#Pixel_storage
	size_t pad = 4 * ceil(bitmapInfoHeader.bitspp*bitmapInfoHeader.width / 32.) - bitmapInfoHeader.width;
	unsigned char c;
	for (size_t i = 0; i < bitmapInfoHeader.height; i++) {
		for (size_t j = 0; j < bitmapInfoHeader.width; j++) {
			c = (unsigned char)outputImage[j + bitmapInfoHeader.width*i];
			if (fwrite(&c, sizeof(char), 1, filePtr) != 1) {
				fclose(filePtr);
				return true;
			}
		}
		c = 0;
		for (size_t j = 0; j<pad; j++)
			if (fwrite(&c, sizeof(char), 1, filePtr) != 1) {
				fclose(filePtr);
				return true;
			}
	}

	fclose(filePtr);
	return false;
}

bitmap_info_header_t 
CBitmapHandler::getBmpHeader()
{
	return  bitmapInfoHeader;
}

// if normalize is true, map pixels to range 0..MAX_BRIGHTNESS
void 
CBitmapHandler::convolution(const pixel_t *in, pixel_t *out, const float *kernel,
	const int nx, const int ny, const int kn,
	const bool normalize)
{
	assert(kn % 2 == 1);
	assert(nx > kn && ny > kn);
	const int khalf = kn / 2;
	float min = FLT_MAX, max = -FLT_MAX;

	if (normalize)
		for (int m = khalf; m < nx - khalf; m++)
			for (int n = khalf; n < ny - khalf; n++) {
				float pixel = 0.0;
				size_t c = 0;
				for (int j = -khalf; j <= khalf; j++)
					for (int i = -khalf; i <= khalf; i++) {
						pixel_t val = in[(n - j) * nx + m - i];
						float fVal = kernel[c];
						pixel += in[(n - j) * nx + m - i] * kernel[c];
						c++;
					}
				if (pixel < min)
					min = pixel;
				if (pixel > max)
					max = pixel;
			}

	for (int m = khalf; m < nx - khalf; m++)
		for (int n = khalf; n < ny - khalf; n++) {
			float pixel = 0.0;
			size_t c = 0;
			for (int j = -khalf; j <= khalf; j++)
				for (int i = -khalf; i <= khalf; i++) {
					pixel += in[(n - j) * nx + m - i] * kernel[c];
					c++;
				}

			if (normalize)
				pixel = MAX_BRIGHTNESS * (pixel - min) / (max - min);
			out[n * nx + m] = (pixel_t)pixel;
		}
}

/*
* gaussianFilter:
* http://www.songho.ca/dsp/cannyedge/cannyedge.html
* determine size of kernel (odd #)
* 0.0 <= sigma < 0.5 : 3
* 0.5 <= sigma < 1.0 : 5
* 1.0 <= sigma < 1.5 : 7
* 1.5 <= sigma < 2.0 : 9
* 2.0 <= sigma < 2.5 : 11
* 2.5 <= sigma < 3.0 : 13 ...
* kernelSize = 2 * int(2*sigma) + 3;
*/
void 
CBitmapHandler::GaussianFilter(const pixel_t *in, pixel_t *out,
	const int nx, const int ny, const float sigma)
{
	const int n = 2 * (int)(2 * sigma) + 3;
	const float mean = (float)floor(n / 2.0);
	// prepares a kernel 7x7 
	float* kernel = new float[n * n];// [n * n]; // variable length array

	fprintf(stderr, "gaussian_filter: kernel size %d, sigma=%g\n",
		n, sigma);
	size_t c = 0;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			kernel[c] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) +
				pow((j - mean) / sigma, 2.0)))
				/ (2 * M_PI * sigma * sigma);
			c++;
		}
	// Executes convolution on this kernel 7x7 
	// Convolution: iterates through all pixels and masks it with the Kernel
	convolution(in, out, kernel, nx, ny, n, true);

	delete[] kernel;
}
/*
* Links:
* http://en.wikipedia.org/wiki/Canny_edge_detector
* http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
* http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
* http://www.songho.ca/dsp/cannyedge/cannyedge.html
*
* Note: T1 and T2 are lower and upper thresholds.
*/
pixel_t*
CBitmapHandler::canny_edge_detection()
{
	const int nx = bitmapInfoHeader.width;
	const int ny = bitmapInfoHeader.height;

	if (G == NULL || after_Gx == NULL || after_Gy == NULL ||
		nms == NULL || outputImage == NULL) {
		fprintf(stderr, "canny_edge_detection:"
			" Failed memory allocation(s).\n");
		exit(1);
	}

	clock_t start = clock();
	GaussianFilter(bitmapImage, outputImage, nx, ny, sigmaValue);
	clock_t end = clock();
	printf("gaussian_filter() %.5f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);

	if (currentState == Types::EIMAGESTATE_GAUSSIAN) return outputImage;

	start = clock();
	//return outputImage;
	// Kernel 3x3 for convolution Gx
	// to calculate aproximations of derivatives
	const float Gx[] = { -1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 };
	// uses kernel Gy to calculate approximations of derivatives on horizontal changes
	convolution(outputImage, after_Gx, Gx, nx, ny, 3, false);
	// Kernel 3x3 for convolution Gy
	const float Gy[] = { 1, 2, 1,
		0, 0, 0,
		-1,-2,-1 };
	// uses kernel Gy to calculate approximations of derivatives on vertical changes
	convolution(outputImage, after_Gy, Gy, nx, ny, 3, false);
	end = clock();
	printf("Gx, Gy convolution() %.5f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);

	start = clock();
	for (int i = 1; i < nx - 1; i++)
		for (int j = 1; j < ny - 1; j++) {
			const int c = i + nx * j;
			// G[c] = abs(after_Gx[c]) + abs(after_Gy[c]);
			G[c] = (pixel_t)hypot(after_Gx[c], after_Gy[c]);
		}
	end = clock();
	printf("distance calculation - hypot %.5f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);
	if (currentState == Types::EIMAGESTATE_DISTANCE) return outputImage;

	start = clock();
	// Non-maximum suppression, straightforward implementation.
	for (int i = 1; i < nx - 1; i++)
		for (int j = 1; j < ny - 1; j++) {
			const int c = i + nx * j;
			const int nn = c - nx;
			const int ss = c + nx;
			const int ww = c + 1;
			const int ee = c - 1;
			const int nw = nn + 1;
			const int ne = nn - 1;
			const int sw = ss + 1;
			const int se = ss - 1;

			const float dir = (float)(fmod(atan2(after_Gy[c],
				after_Gx[c]) + M_PI,
				M_PI) / M_PI) * 8;

			if (((dir <= 1 || dir > 7) && G[c] > G[ee] &&
				G[c] > G[ww]) || // 0 deg
				((dir > 1 && dir <= 3) && G[c] > G[nw] &&
					G[c] > G[se]) || // 45 deg
					((dir > 3 && dir <= 5) && G[c] > G[nn] &&
						G[c] > G[ss]) || // 90 deg
						((dir > 5 && dir <= 7) && G[c] > G[ne] &&
							G[c] > G[sw]))   // 135 deg
				nms[c] = G[c];
			else
				nms[c] = 0;
		}
	end = clock();
	printf("non maximum supression %.5f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);
	// Reuse array
	// used as a stack. nx*ny/2 elements should be enough.
	int *edges = (int*)after_Gy;
	memset(outputImage, 0, sizeof(pixel_t) * nx * ny);
	memset(edges, 0, sizeof(pixel_t) * nx * ny);

	start = clock();
	// Tracing edges with hysteresis . Non-recursive implementation.
	size_t c = 1;
	for (int j = 1; j < ny - 1; j++)
		for (int i = 1; i < nx - 1; i++) {
			if (nms[c] >= maxThreshold && outputImage[c] == 0) { // trace edges
				outputImage[c] = MAX_BRIGHTNESS;
				int nedges = 1;
				edges[0] = c;

				do {
					nedges--;
					const int t = edges[nedges];

					int nbs[8]; // neighbours
					nbs[0] = t - nx;     // nn
					nbs[1] = t + nx;     // ss
					nbs[2] = t + 1;      // ww
					nbs[3] = t - 1;      // ee
					nbs[4] = nbs[0] + 1; // nw
					nbs[5] = nbs[0] - 1; // ne
					nbs[6] = nbs[1] + 1; // sw
					nbs[7] = nbs[1] - 1; // se

					for (int k = 0; k < 8; k++)
						if (nms[nbs[k]] >= minThreshold && outputImage[nbs[k]] == 0) {
							outputImage[nbs[k]] = MAX_BRIGHTNESS;
							edges[nedges] = nbs[k];
							nedges++;
						}
				} while (nedges > 0);
			}
			c++;
		}
	end = clock();
	printf("tracing %.5f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);

	return outputImage;
}
