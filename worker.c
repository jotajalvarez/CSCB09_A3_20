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

#define BUFFER_SIZE 256
#define INT_MAX 1.84e19


// Helper function to open directory and search for files

void open_dir(char *dir, char *result[BUFFER_SIZE]) {
	DIR *dir_ptr;
	struct dirent *entry;

	// int array_ind;
	int counter_ind = 0;
	dir_ptr = opendir(dir);

	if (dir_ptr == NULL){
		fprintf(stderr, "Directory cannot be opened: %s\n", dir);
		return;
	}

	if (dir_ptr) {
		while ((entry = readdir(dir_ptr)) != NULL){
			result[counter_ind] = malloc(strlen(entry->d_name)+1);
			strcpy(result[counter_ind], entry->d_name);
			printf("%s\n", result[counter_ind]);
			printf("%p\n", &result[counter_ind]);
			counter_ind++;
		}
		closedir(dir_ptr);
	}

}

/*
 * Read an image from a file and create a corresponding
 * image struct
 */
Image* read_image(char *filename) {
	// Create a file pointer to read the file
	FILE *f_ptr;
	// Define a set of variables for the header information
	char *format_buf = malloc(sizeof(char) * 256);
	int width, height, max_value;

	// int total_dim;

	// Open the file and set it to the pointer
	f_ptr = fopen(filename, "r");
	if ( f_ptr == NULL ) {
		perror("Error: ");
		return NULL;
	}

	// Scanf for the header information of the file
	int assigned_items = fscanf(f_ptr, "%s %d %d %d", format_buf, &width, &height, &max_value);
	if ( assigned_items <= 0) {
		return NULL;
	}

	// Get the info from the file

	int num_of_pixels = (width * height * 3);
	Image *img = (Image*)malloc(sizeof(Image));
	img->height = height;
	img->width = width;
	img->p = (Pixel*)malloc(sizeof(Pixel*) * (num_of_pixels));

	// loop through the pixels in the image
	int red, blue, green = 0;
	int pixel_index = 0;
	while(fscanf(f_ptr, "%d %d %d", &red, &green, &blue) != EOF) {
		Pixel p;
		p.blue = blue;
		p.green = green;
		p.red = red;
		img->p[pixel_index] = p;
		pixel_index++;
	}

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
		return sqrt( pow(p1.red - p2.red, 2) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {
	// Create an Image struct of the filename image.
	Image *cmp_image = read_image(filename);

	// Create a counter to average the pixel count
	float eucl_sum = 0;

	if ((img1->width != cmp_image->width) || (img1->height != cmp_image->height)) {
		return FLT_MAX;
	}
	int total_dim = (img1->height * (img1->width));
	for (int i_width = 0; i_width < total_dim; i_width += 1) {
		Pixel first = img1->p[i_width];
		Pixel second = cmp_image->p[i_width];
		eucl_sum += eucl_distance(first, second);
	}

	int eucl_return = (eucl_sum/total_dim);

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

	// Create the array for all the files
	char result[BUFFER_SIZE][BUFFER_SIZE];
	// Open the directory and search for all the files
	// open_dir(dirname, &result[BUFFER_SIZE]); // here is the problem

	DIR *dir_ptr = opendir(dirname);

	if (dir_ptr == NULL){
		perror("Unable to open file/directory");
		exit(1);
	}


	struct dirent *entry = readdir(dir_ptr);
	int counter_ind = 0;
	while ((entry = readdir(dir_ptr)) != NULL){
		if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) {
			continue;
		}
		strcpy(result[counter_ind], entry->d_name);
		counter_ind++;
	}
	closedir(dir_ptr);

	float result_comp = 0;
	CompRecord CRec;
	//Set CRec to the biggest number and root path name;
	strcpy(CRec.filename, ".");
	CRec.distance = INT_MAX;

	while (counter_ind != -1) {
		result_comp = compare_images(img, result[counter_ind]);
		if (CRec.distance > result_comp) {
			CRec.distance = result_comp;
			strcpy(CRec.filename, result[counter_ind]);
		}
		counter_ind--;
	}
	write(out_fd, &CRec, sizeof(CompRecord));
	return CRec;
}
