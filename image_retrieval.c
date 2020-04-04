#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"

#define INT_MAX 1.84e19
#define BUFFER_SIZE 256



int dir_count(char *dir) {
    struct dirent *entry;

    int counter = 0;
    DIR *dir_ptr = opendir(dir);

    if (dir_ptr == NULL) {
        fprintf(stderr, "Opening directory failed: %s\n", dir);
        return -1;
    }

    if (dir_ptr) {
        while((entry = readdir(dir_ptr)) != NULL) {
            if(entry->d_type == 4){
                char *new_dir = NULL;
                strcpy(new_dir, dir);
                strcat(new_dir, "/");
                strcat(new_dir, entry->d_name);
                counter++;
                chdir(new_dir);
            }
        }
        close(dir_ptr->__dd_fd);
    }
    return counter;
}

CompRecord process_program(Image *cmp_image, char *dirname, int num_dirs) {
	//Set up the pipes to read and write
	int pipe_fd[num_dirs][2];
	CompRecord worker_results[num_dirs];
	// Fork one process per num_dirs
	for (int i = 0; i < num_dirs; i++) {
		if (pipe(pipe_fd[i]) == -1) {
			perror("Pipe");
			exit(EXIT_FAILURE);
		}
		pid_t p = fork();
		if (p < 0) {
			perror("Fork");
			exit(EXIT_FAILURE);
		} else if(p > 0) {
			// Parent process
			close(pipe_fd[i][1]);
			// Populate worker_results
			CompRecord readRecord;
			read(pipe_fd[i][0], &readRecord, sizeof(CompRecord));
			worker_results[i] = readRecord;
		} else {
			// Child process
			if (num_dirs == 0) {
				CompRecord result = process_dir(dirname, cmp_image, pipe_fd[i][0]);
				return result;
			}
			struct dirent *entry;

			DIR *dir_ptr = opendir(dirname);

			if (dir_ptr == NULL) {
				fprintf(stderr, "Opening directory failed: %s\n", dirname);
				exit(EXIT_FAILURE);
			}
			CompRecord child_results[num_dirs];
			int child_ix = 0;
			while((entry = readdir(dir_ptr)) != NULL) {
				if(entry->d_type == 4){
					char *new_dir = malloc(sizeof(char) * BUFFER_SIZE);
					strncpy(new_dir, dirname, strlen(dirname));
					strcat(new_dir, "/");
					strcat(new_dir, entry->d_name);
					CompRecord child_result  = process_program(cmp_image, new_dir, dir_count(new_dir));
					free(new_dir);
					child_results[child_ix] = child_result;
					child_ix++;
				}
			}
			close(dir_ptr->__dd_fd);

			// Loop on child_results and return the smallest one
			CompRecord CRec;
			//Set CRec to the biggest number and root path name;
			strcpy(CRec.filename, ".");
			CRec.distance = INT_MAX;
			while (child_ix != -1) {
				CompRecord result_comp = child_results[child_ix];
				if (CRec.distance > result_comp.distance) {
					CRec.distance = result_comp.distance;
					strncpy(CRec.filename, result_comp.filename, strlen(result_comp.filename));
				}
				child_ix--;
			}
			return CRec;
		}

	}
	// Parent aggregates results
	CompRecord CRec;
	//Set CRec to the biggest number and root path name;
	strcpy(CRec.filename, ".");
	CRec.distance = INT_MAX;
	int child_ix = 0;
	while (child_ix != -1) {
		CompRecord result_comp = worker_results[child_ix];
		if (CRec.distance > result_comp.distance) {
			CRec.distance = result_comp.distance;
			strncpy(CRec.filename, result_comp.filename, strlen(result_comp.filename));
		}
		child_ix--;
	}
	return CRec;
}

int main(int argc, char **argv) {

	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
    char *image_file = NULL;

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

	if (optind != argc-1) {
		fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
	} else {
		image_file = argv[optind];
	}
	// Open the directory provided by the user (or current working directory)

	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	}

	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/

	struct dirent *dp;
    CompRecord CRec;

	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 ||
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0){
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		}

		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
			// Create the image for from the image_file path
			Image *passing_image = read_image(image_file);
        	CRec = process_program(passing_image, path, dir_count(path));
		}
		else {
			Image *passing_image = read_image(image_file);
			CRec.distance = compare_images(passing_image, path);
			strcpy(CRec.filename, path);
		}

	}
	printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);
	return 0;
}

