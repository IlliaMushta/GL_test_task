#include "Image.h"
#include "utils.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define PI 3.141592653589793238462643383

void Image_load(Image *img, const char *fname) {
    if((img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0)) != NULL) {
        img->size = img->width * img->height * img->channels;
        img->allocation_ = STB_ALLOCATED;
    }
}

void Image_create(Image *img, int width, int height, int channels, bool zeroed) {
    size_t size = width * height * channels;
    if(zeroed) {
        img->data = calloc(size, 1);
    } else {
        img->data = malloc(size);
    }

    if(img->data != NULL) {
        img->width = width;
        img->height = height;
        img->size = size;
        img->channels = channels;
        img->allocation_ = SELF_ALLOCATED;
    }
}

void Image_save(const Image *img, const char *fname) {
    // Check if the file name ends in one of the .jpg/.JPG/.jpeg/.JPEG or .png/.PNG
    if(str_ends_in(fname, ".jpg") || str_ends_in(fname, ".JPG") || str_ends_in(fname, ".jpeg") || str_ends_in(fname, ".JPEG")) {
        stbi_write_jpg(fname, img->width, img->height, img->channels, img->data, 100);
    } else if(str_ends_in(fname, ".png") || str_ends_in(fname, ".PNG")) {
        stbi_write_png(fname, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else {
        ON_ERROR_EXIT(false, "");
    }
}

void Image_free(Image *img) {
    if(img->allocation_ != NO_ALLOCATION && img->data != NULL) {
        if(img->allocation_ == STB_ALLOCATED) {
            stbi_image_free(img->data);
        } else {
            free(img->data);
        }
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->size = 0;
        img->allocation_ = NO_ALLOCATION;
    }
}

int Get_Pixel_Intensity(const Image *orig, int x, int y)
{
	ON_ERROR_EXIT(((x > orig->width) || (y > orig->height)), "Error in access to the pixel");
	unsigned char *p = orig->data; 
	int change = x * 3 + 3 *y * (orig->width);
	return 0.299 * (*(p + change)) + 0.587 * (*(p + 1 + change)) + 0.144 * (*(p + 2 + change));
}

void Set_Pixel_Intensity(const Image *orig, int x, int y, double intensity)
{
	ON_ERROR_EXIT(((x > orig->width) || (y > orig->height)), "Error in access to the pixel");
	int change = x * 3 + 3 * y * (orig->width);
	unsigned char *p = orig->data + change; 
	*p = (int)intensity;
	*(p + 1) = (int)intensity;
	*(p + 2) = (int)intensity;
	
}

void Set_Pixel_IntensityRGB(const Image *orig, int x, int y, double intensityR, double intensityG, double intensityB)
{
	ON_ERROR_EXIT(((x > orig->width) || (y > orig->height)), "Error in access to the pixel");
	int change = x * 3 + 3 * y * (orig->width);
	unsigned char *p = orig->data + change; 
	*p = (int)intensityR;     //red
	*(p + 1) = (int)intensityG; //green
	*(p + 2) = (int)intensityB; //blue
	
}

double  ValueOFImage(const int filter[3][3], const Image *orig , int biasVertical, int biasHorizon)
{
	double value = 0;
	int intensity;
	for (int y = 0 + biasVertical; y < 3 + biasVertical; y++) {
		for (int x = 0 + biasHorizon; x < 3 + biasHorizon; x++) {
			if ((y < orig->height) && (x < orig->width)) {
				intensity = Get_Pixel_Intensity(orig,x,y);
			}else {
				intensity = 0;
			}
			value =value + intensity*filter[x - biasHorizon][y - biasVertical];
		}
	}
	
	return value;
}

void Image_edge_detection(const Image *orig, Image *edge_detected)
{
    	ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    	Image_create(edge_detected, orig->width, orig->height, orig->channels, false);
    	ON_ERROR_EXIT(edge_detected->data == NULL, "Error in creating the image");
	
	const int Gy[3][3]={ {-1,-2,-1},{0,0,0},{1,2,1} };
	const int Gx[3][3]={ {-1,0,1},{-2,0,2},{-1,0,1} };
	CenterValue** Center = (CenterValue**)malloc(orig->height*sizeof(CenterValue*));
	for (int count = 0; count < orig->height; count++){
		Center[count] = (CenterValue*)malloc(orig->width*sizeof(CenterValue));
	}

	int i = 0,j = 0;
	for (int biasHorizon = 0; biasHorizon < orig->width; biasHorizon++) {
		for (int biasVertical = 0; biasVertical < orig->height; biasVertical++) {
			Center[i][j].x0 = biasVertical;
			Center[i][j].y0 = biasHorizon;
			Center[i][j].intensity = sqrt(pow(ValueOFImage(Gx, orig, biasVertical, biasHorizon), 2)+pow(ValueOFImage(Gy, orig, biasVertical, biasHorizon), 2));
			i++;
		}
		i = 0;
		j++;
	}

	for (int y = 0; y < orig->width; y++){
		for (int x = 0; x < orig->height; x++){
			Set_Pixel_Intensity(edge_detected,y,x,Center[x][y].intensity);
		}
	}

	for (int count = 0; count < orig->height; count++){
		free(Center[count]);
	}
}

int  AverageValueOFImage(const Image *orig, const Image *im_template, int biasVertical, int biasHorizon)
{
	const int klPixIm = (im_template->height) * (im_template->width);
	double sumPixel = 0;
	double averageImage = 0;
	int intensity;
	for (int y = 0 + biasVertical; y < im_template->height + biasVertical; y++) {
		for (int x = 0 + biasHorizon; x < im_template->width + biasHorizon; x++) {
			if ((y < orig->height) && (x < orig->width)) {
				intensity = Get_Pixel_Intensity(orig,x,y);
			}else {
				intensity = 0;
			}			
			sumPixel = sumPixel + intensity;
		}
	}
	averageImage = sumPixel / klPixIm;

	return (int)averageImage;
}

void Image_copy(const Image *orig, Image *newIm) 
{
    
    for(unsigned char *p = orig->data, *pg = newIm->data; p != orig->data + orig->size; p += orig->channels, pg += newIm->channels) {
        *pg       = *p;         // red
        *(pg + 1) = *(p + 1);   // green
        *(pg + 2) = *(p + 2);   // blue        
    }
}


void Rect(Image *new_image,const Image *im_template, CenterValue ** Center, int sizeX, int sizeY, int thickness, int precision)
{
	for (int y = 0; y < sizeY; y++){
		for (int x = 0; x < sizeX; x++){
			if((Center[x][y].y0 == y)&&(Center[x][y].x0 == x)&&(Center[x][y].intensity >= precision)){

				for(int i = y;i < y + im_template->width;i++){
					for(int j = x;j < x + thickness;j++){
						if((i < new_image->width)&&(j < new_image->height))
							Set_Pixel_IntensityRGB(new_image,i,j,255,0,0);
					}
				}

				for(int i = y;i < y + im_template->width;i++){
					for(int j = x + im_template->height - thickness + 1;j < x + im_template->height + 1;j++){
						if((i < new_image->width)&&(j < new_image->height))
							Set_Pixel_IntensityRGB(new_image,i,j,255,0,0);
					}
				}

				for(int i = y;i < y + thickness;i++){
					for(int j = x;j < x + im_template->height;j++){
						if((i < new_image->width)&&(j < new_image->height))
							Set_Pixel_IntensityRGB(new_image,i,j,255,0,0);
					}
				}

				for(int i = y + im_template->width - thickness + 1;i < y + im_template->width  + 1;i++){
					for(int j = x;j < x + im_template->height;j++){
						if((i < new_image->width)&&(j < new_image->height))
							Set_Pixel_IntensityRGB(new_image,i,j,255,0,0);
					}
				}
			}

		}
	}
}





void Image_object_detection(const Image *orig, const Image *im_template, Image *new_image,int precision)
{
    	ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    	Image_create(new_image, orig->width, orig->height, orig->channels, false);
    	ON_ERROR_EXIT(new_image->data == NULL, "Error in creating the image");

	int averageEtalonImage = AverageValueOFImage(im_template, im_template, 0, 0);
	
	double sumDenominator1 = 0;
	double sumDenominator2 = 0;
	double numerator = 0;
	double coefCorrelation = 0;
	
	CenterValue** Center = (CenterValue**)malloc(orig->height*sizeof(CenterValue*));
	for (int count = 0; count < orig->height; count++){
		Center[count] = (CenterValue*)malloc(orig->width*sizeof(CenterValue));
	}
	
	int i = 0,j = 0;

	for (int y = 0; y < im_template->height; y++) {
		for (int x = 0; x < im_template->width; x++) {
			sumDenominator1 += pow((Get_Pixel_Intensity(im_template,x,y) - averageEtalonImage), 2);
		}
	}

	double part = 0;
	int intensity = 0;
	for (int biasHorizon = 0; biasHorizon < orig->width; biasHorizon++) {
		for (int biasVertical = 0; biasVertical < orig->height; biasVertical++) {
			int Average = AverageValueOFImage(orig, im_template, biasVertical, biasHorizon);///===
			
			for (int y = 0 + biasVertical; y < im_template->height + biasVertical; y++) {
				for (int x = 0 + biasHorizon; x < im_template->width + biasHorizon; x++) {
					if ((y < orig->height) && (x < orig->width)) {
						intensity = Get_Pixel_Intensity(orig,x,y);
					}else {
						intensity = 0;
					}		
					part = (intensity - Average);
					sumDenominator2 += pow(part, 2);
					numerator += (Get_Pixel_Intensity(im_template,x - biasHorizon,y - biasVertical) - averageEtalonImage)*part;
					part = 0;
				}
			}
			Average = 0;
			coefCorrelation = numerator / sqrt(sumDenominator1 * sumDenominator2);
			
			double intensity = fabs(coefCorrelation) * 255;
			
			Center[i][j].y0 = biasHorizon;
			Center[i][j].x0 = biasVertical;
			
			Center[i][j].intensity = intensity;
			
			intensity = 0;

			i++;
			sumDenominator2 = 0;
			numerator = 0;
			coefCorrelation = 0;
		}
		i = 0;
		j++;

	}
	
	int precision_ = (int)(15.5 * precision) + 100;
	Image_copy(orig,new_image);
	Rect(new_image,im_template,Center,orig->height, orig->width,3,precision_);
	
}


void Gauss_low_frequency_filter(const Image *orig, Image *new_image)
{
        ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    	Image_create(new_image, orig->width, orig->height, orig->channels, false);
    	ON_ERROR_EXIT(new_image->data == NULL, "Error in creating the image");
    	ON_ERROR_EXIT(orig->size > 20000, "Image too large,size > 20000 pixels");


	CenterValue** f = (CenterValue**)malloc(orig->height*sizeof(CenterValue*));
	for (int count = 0; count < orig->height; count++){
		f[count] = (CenterValue*)malloc(orig->width*sizeof(CenterValue));
	}

	CenterValueComplex** Fourier_transform = (CenterValueComplex**)malloc(orig->height * sizeof(CenterValueComplex*));
	for (int count = 0; count < orig->height; count++){
		Fourier_transform[count] = (CenterValueComplex*)malloc(orig->width * sizeof(CenterValueComplex));
	}

	for(int x = 0;x < orig->height; x++){
		for(int y = 0;y < orig->width; y++){
			f[x][y].x0 = x;
			f[x][y].y0 = y;
			f[x][y].intensity = pow(-1,x + y) * Get_Pixel_Intensity(orig,y,x);
		}
	}

	double complex direct_sum;
	
	for(int u = 0;u < orig->height; u++){
		for(int v = 0;v < orig->width; v++){
			for(int x = 0;x < orig->height; x++){
				for(int y = 0;y < orig->width; y++){
				 	direct_sum += (f[x][y].intensity * cos((-2) * PI * (u * (double)x/orig->height + v * (double)y/orig->width)) + I*(f[x][y].intensity * sin((-2) * PI * (u * (double)x/orig->height + v * (double)y/orig->width))));
				}
			}	
			Fourier_transform[u][v].x0 = u;
			Fourier_transform[u][v].y0 = v;	
			Fourier_transform[u][v].intensity = direct_sum/(orig->height * orig->width);;
			direct_sum = 0 ;	
		}
	}

	for(int u = 0;u < orig->height; u++){
		for(int v = 0;v < orig->width; v++){
				if((Fourier_transform[u][v].x0 == u)&&(Fourier_transform[u][v].y0 == v)){
				Fourier_transform[u][v].intensity = Fourier_transform[u][v].intensity * exp(  (-1)*(double)(  pow((u - (double)orig->height/2),2) + pow((v - (double)orig->width/2),2)  )/ (2 * 3)  );
			}		
		}
	}
	

	double complex reverse_sum;
	for(int x = 0;x < orig->height; x++){
		for(int y = 0;y < orig->width; y++){
			for(int u = 0;u < orig->height; u++){
				for(int v = 0;v < orig->width; v++){		
					reverse_sum += (Fourier_transform[u][v].intensity * cos((2) * PI * (u * (double)x/orig->height + v * (double)y/orig->width)) + I * (Fourier_transform[u][v].intensity * sin((2) * PI * (u * (double)x/orig->height + v * (double)y/orig->width))));
				}
			}
			f[x][y].intensity = creal(reverse_sum);
			f[x][y].intensity *= pow(-1,x + y);
			reverse_sum = 0;	
		}
	}


	unsigned char *p = orig->data;	
	double R,G,B;

	for (int y = 0; y < orig->width; y++){
		for (int x = 0; x < orig->height; x++){	
			if((f[x][y].x0 == x)&&(f[x][y].y0 == y)){ 
				R = (double) *(p + y * 3 + 3 * x * (orig->width))/255;
				G = (double) *(p + 1 + y * 3 + 3 * x * (orig->width))/255;
				B = (double) *(p + 2 + y * 3 + 3 * x * (orig->width))/255;
				Set_Pixel_IntensityRGB(new_image,y,x,R * f[x][y].intensity,G * f[x][y].intensity,B * f[x][y].intensity);
				R = G = B = 0;
			}		
		}
	}

	for (int count = 0; count < orig->height; count++){
		free(Fourier_transform[count]);
	}

	for (int count = 0; count < orig->height; count++){
		free(f[count]);
	}

}



