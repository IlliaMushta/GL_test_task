// This program has 3 mode: 	1)Edge detection 
//				2)Object detection (find image2 on image1 with precision from range[0 ..10], image2.size < image1.size)
//				3)Gauss low intensity filter
// To build:
// gcc -std=gnu18 -O2 -Wall -Wextra -Wpedantic Image.c Image_Processing.c -lm -o ImProc
//
// To run Edge detection :
// ./ImProc 1 image.jpg
//
// To run Object detection:
// ./ImProc 2 image1.jpg image2.jpg precision
//
// To run Gauss low intensity filter :
// ./ImProc 3 image.jpg

#include "Image.h"
#include "utils.h"
#include "math.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) 
{
	if(!strcmp(argv[1],"1")){
		Image Im1;
		Image_load(&Im1, argv[2]);
		ON_ERROR_EXIT(argc!=3, "There must be 2 arguments: program mode, image");
		ON_ERROR_EXIT(Im1.data == NULL, "Error in loading the image");
		Image Im1_edge;
		Image_edge_detection(&Im1, &Im1_edge);
		Image_save(&Im1_edge, "edge_detection.jpg");
		Image_free(&Im1);
	   	Image_free(&Im1_edge);

	}else if(!strcmp(argv[1],"2")){
		Image Im2;
		Image Im3;
		Image_load(&Im2, argv[2]);
		Image_load(&Im3, argv[3]);
		ON_ERROR_EXIT(argc!=5, "There must be 4 arguments: program mode, image1, image2, precision");
		ON_ERROR_EXIT(Im2.data == NULL, "Error in loading the image");
		ON_ERROR_EXIT(Im3.data == NULL, "Error in loading the image");
		ON_ERROR_EXIT(  ((atoi(argv[4]) > 10) || (atoi(argv[4]) < 0)), "Precision must be in range [0 ..10]");
		Image Im_correlation;
		Image_object_detection(&Im2,&Im3,&Im_correlation,atoi(argv[4]));
		Image_save(&Im_correlation, "Im_correlation.jpg");
		Image_free(&Im2);
		Image_free(&Im3);
	    	Image_free(&Im_correlation);

	}else if(!strcmp(argv[1],"3")){
		Image Im4;
		Image_load(&Im4, argv[2]);
		ON_ERROR_EXIT(argc!=3, "There must be 2 arguments: program mode, image");
		ON_ERROR_EXIT(Im4.data == NULL, "Error in loading the image");
		Image new_image;
		Gauss_low_frequency_filter(&Im4,&new_image);
		Image_save(&new_image, "Gauss_filter.jpg");
		Image_free(&Im4);
	    	Image_free(&new_image);
	}else{
		ON_ERROR_EXIT(  ((atoi(argv[1]) > 3) || (atoi(argv[1]) < 0)), "The first argument(program mode) must be in range [1 ..3]");
	}
	return 0;
}

