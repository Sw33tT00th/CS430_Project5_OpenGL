#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "helpers.h"
#include "header.h"
#include "p6.h"

/******************************************************************************
* READ P6
* Purpose: Reads the file as if it were a P6 formatted ppm file
* Return Value:
*				0 if the file was read completely without any errors
*				1 if the end of the file was found unexpectedly
*				2 if the value found is less than 0 or greater than max_val
******************************************************************************/
int read_p6(char *file_name, FILE* input_file, Header file_header, Pixel *body_content) {
	int size;
	int first_byte;
	int second_byte;

	// check how many bytes need to be read
	if (file_header.max_val < 256) {
		size = 1;
	}
	else if (file_header.max_val >= 256) {
		size = 2;
	}
	// close the file and reopen it as binary in the same place
	long int location = ftell(input_file);
	fclose(input_file);
	input_file = fopen(file_name, "rb");
	fseek(input_file, location, SEEK_SET);
	// main read loop
	int i;
	int j;
	for(i = 0; i < (file_header.width * file_header.height); i++) {
		for(j = 0; j < 3; j++) {
			// check if it's the end of the file
			if(peek_next_char(input_file) == EOF) {
				fprintf(stderr, "ERROR: File ended earlier than expected - Missing Data\n\nClosing Program\n");
				printf("%d\n", i);
				return 1;
			}
			first_byte = fgetc(input_file);
			if(size == 1) {
				if(first_byte < 0 || first_byte > file_header.max_val) {
					fprintf(stderr, "ERROR: Invalid data. Data value out of range\n\nClosing Program\n");
					return 2;
				}
			}
			if(size == 2) {
				first_byte = first_byte << 8; // left shift the first byte
				// check if it's the end of the file
				if(peek_next_char(input_file) == EOF) {
					fprintf(stderr, "ERROR: File ended earlier than expected - Missing Data\n\nClosing Program\n");
					return 1;
				}
				second_byte = fgetc(input_file);
				first_byte = first_byte + second_byte;
				if(first_byte < 0 || first_byte > file_header.max_val) {
					fprintf(stderr, "ERROR: Invalid data. Data value out of range\n\nClosing Program\n");
					return 2;
				}
			}
			if(j == 0) {
				body_content[i].r = (double)first_byte / file_header.max_val;
			}
			else if(j == 1) {
				body_content[i].g = (double)first_byte / file_header.max_val;
			}
			else {
				body_content[i].b = (double)first_byte / file_header.max_val;
			}
		}
	}
	fclose(input_file);
	input_file = fopen(file_name, "r");
	return 0;
}

/******************************************************************************
* WRITE P6
* Purpose: Writes the data passed in to a file in the P6 format
* Return Value:
*				0 if all went well and all data was accounted for.
******************************************************************************/
int write_p6(FILE* output_file, Header file_header, Pixel *body_content) {
	int i;
	int j;
	int val;
	char temp1;
	char temp2;
	
	for(i = 0; i < file_header.width * file_header.height; i++) {

		for(j = 0; j < 3; j++) {
			if(j == 0) {
				val = (int)(body_content[i].r * file_header.max_val);
			}
			else if (j == 1) {
				val = (int)(body_content[i].g * file_header.max_val);	
			}
			else {
				val = (int)(body_content[i].b * file_header.max_val);
			}

			if(file_header.max_val > 255) {
				temp1 = (char)(val >> 8);
				fwrite(&temp1, 1, sizeof(temp1), output_file);
				// clear bits to the left of the least significant byte
				val = val << 24;
				val = val >> 24;
			}
			temp2 = (char)val;
			fwrite(&temp2, 1, sizeof(temp2), output_file);	
		}
	}
	return 0;
}