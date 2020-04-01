#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"

// Create a helper function that allocates space for the pixel
struct Image *allocate_pixel(struct Pixel *passing_pixel) {
	struct Image *current_pixel = malloc(sizeof(struct Pixel));
	current_pixel->passing_pixel = malloc(sizeof(int) * 3);
	return current_pixel;
}


// Create the pixel for the image
struct Image create_image(struct Pixel *pixel_, int width , int height) {
	struct Pixel *new_pixel;
	total_dim = (height * (width * 3));
	for (int i_width = 1; i_width <= total_dim; i_width) {
		new_pixel = allocate_pixel(*pixel_);
		new_pixel.red = pixel_.red;
		new_pixel.green = pixel.green;
		new_pixel.blue = pixel.blue;
	}
	return new_pixel;
}

/*
 * Read an image from a file and create a corresponding
 * image struct
 */
Image* read_image(char *filename) {
	// Create a file pointer to read the file
	FILE *f_ptr;
	// Define a set of variables for the header information
	char format_buf[256];
	int width, height, max_value;

	int total_dim;

	// Open the file and set it to the pointer
	f_ptr = fopen(*filename, "r");
	if ( f_ptr == NULL ) {
		perror("Error: ");
		return -1;
	}

	// Scanf for the header information of the file
	scanf("%s %d %d %d", &format_buf, &width, &height, &max_value);
	if ( format_buf != "P3" ) {
		return NULL;
	}


	// Allocate space for the array of pixel.

	// Allocate the space for the info of each pixel




	Image *img;
	return img;
	}

/*
 * Print an image based on the provided Image struct
 */

void print_image(Image *img){
		printf("P3\n");
		printf("%d %d\n", img->width, img->height);
		printf("%d\n", img->max_value);

		for(int i=0; i<img->width*img->height; i++)
		   printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
		printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels
 */
float eucl_distance (Pixel p1, Pixel p2) {
		return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {
	// Create an Image struct of the filename image.
	struct Image *filename_image = read_image(filename);

	// Create a total_dim variable
	int total_dim = (img1.height * (img1.width *3));
	// Create a counter to average the pixel count
	int eucl_sum = 0;
	int eucl_count = 0;
	int eucl_return = 0;

	if ((img1.width != filename_image.width) || (img1.height != filename_image.height)) {
		return FLT_MAX;
	}
	for (int i_width = 1; i_width < total_dim; i_width += 3){
		eucl_sum += eucl_distance(img1->p[i_width], filename_image->p[i_width]);
		eucl_count++;
	}

	eucl_return = eucl_sum/eucl_count;

	return eucl_return;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it
* - for each file read the image in it
* - compare the image read to the image passed as parameter
* - keep track of the image that is most similar
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img, int out_fd){

		CompRecord CRec;

		return CRec;
}
