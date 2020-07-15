#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <complex.h>

enum allocation_type {
    NO_ALLOCATION, SELF_ALLOCATED, STB_ALLOCATED
};

typedef struct {
    int width;
    int height;
    int channels;
    size_t size;
    uint8_t *data;
    enum allocation_type allocation_;
} Image;

typedef struct 
{
	int x0;
	int y0;
	double intensity;
}CenterValue;

typedef struct 
{
	int x0;
	int y0;
	double complex intensity;
}CenterValueComplex;

void Image_load(Image *img, const char *fname);
void Image_create(Image *img, int width, int height, int channels, bool zeroed);
void Image_save(const Image *img, const char *fname);
void Image_free(Image *img);

void Image_edge_detection(const Image *orig, Image *edge_detected);
int Get_Pixel_Intensity(const Image *orig, int x, int y);
double ValueOFImage(const int filter[3][3],const Image *orig , int biasHorizon, int biasVertical);
void Set_Pixel_Intensity(const Image *orig,int x, int y, double intensity);
int AverageValueOFImage(const Image *orig, const Image *im_template, int biasHorizon, int biasVertical);
void Image_object_detection(const Image *orig, const Image *im_template,Image *new_image,int precision);
void Rect(Image *new_image,const Image *im_template, CenterValue ** Center, int sizeX, int sizeY, int thickness, int precision);
void Image_copy(const Image *orig, Image *newIm);
void Set_Pixel_IntensityRGB(const Image *orig, int x, int y, double intensityR, double intensityG, double intensityB);
void Gauss_low_frequency_filter(const Image *orig, Image *new_image);

