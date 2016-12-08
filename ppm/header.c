#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "helpers.h"
#include "header.h"

/******************************************************************************
* READ HEADER
* Purpose: Reads the header of the input file and store it in a global variable
* Return value:
*				0 if file read has a properly formatted .ppm header
*				1 if the file is not ppm format
*				2 if the file is neither P3 nor P6
*				3 if the file has a width less than 1px
*				4 if the file has a height less than 1px
*				5 if the maximum color value is outside of the range [1, 65535]
******************************************************************************/
int read_header(FILE* input_file, Header *file_header) {
	// read in the magic number
	file_header->magic_number[0] = fgetc(input_file);
	// check if first character is P
	if(file_header->magic_number[0] != 'P') {
		fprintf(stderr, "ERROR: Invalid File:\n\tFile is not in the PPM format\n\nClosing Program\n");
		return 1;
	}
	file_header->magic_number[1] = fgetc(input_file);
	// check if second character is 3 or 6
	if(file_header->magic_number[1] != '3' && file_header->magic_number[1] != '6') {
		fprintf(stderr, "ERROR: Invalid File:\n\tFile is neither P3 or P6\n\nClosing Program\n");
		return 2;
	}
	int i;
	for(i = 0; i < 3; i++) {
		// walk through whitespace and comments and store none of it
		while(isspace(peek_next_char(input_file))) {
			fgetc(input_file);
			if(peek_next_char(input_file) == '#') { handle_comment(input_file); }
		}
		// get the width data
		if(i == 0) {
			fscanf(input_file, "%d", &(file_header->width));
			// check if Width value is valid
			if(file_header->width < 1) {
				fprintf(stderr, "ERROR: Invalid image width, must be at least 1px wide\n\nClosing Program\n");
				return 3;
			}
		}
		// get the height data
		if(i == 1) {
			fscanf(input_file, "%d", &(file_header->height));
			// check if height value is valid
			if(file_header->height < 1) {
				fprintf(stderr, "ERROR: Invalid image height, must be at least 1px high\n\nClosing Program\n");
				return 4;
			}
		}
		// get the maximum value data
		if(i == 2) {
			fscanf(input_file, "%d", &(file_header->max_val));
			// check if the maximum color value is valid
			if(file_header->max_val < 1 || file_header->max_val > 65535) {
				fprintf(stderr, "ERROR: Invalid maximum color value, must be between 1 and 65535\n\nClosing Program\n");
				return 5;
			}
		}
		
	}
	// walk through whitespace and comments and store none of it
	while(isspace(peek_next_char(input_file))) {
		fgetc(input_file);
		if(peek_next_char(input_file) == '#') { handle_comment(input_file); }
	}
	//printf("Magic Number: %s\nWidth: %d\nHeight: %d\nMaximum Color Value: %d\n", file_header.magic_number, file_header.width, file_header.height, file_header.max_val);
	return 0;
}

/******************************************************************************
* WRITE HEADER
* Purpose: Writes the header information into the specified file
* Return Value:
*				0 if all went well and all data was accounted for
*				1 if the output file failed to open
******************************************************************************/
int write_header(FILE* output_file, char *format, Header file_header) {
	if (output_file == NULL) {
		fprintf(stderr, "ERROR: Output File failed to open\n\nClosing Program\n");
		return 1;
	}
	char temp[80];
	// write magic number
	sprintf(temp, "P%s\n", format);
	fputs(temp, output_file);
	// write dimensions
	sprintf(temp, "%d %d\n", file_header.width, file_header.height);
	fputs(temp, output_file);
	// write max color value
	sprintf(temp, "%d\n", file_header.max_val);
	fputs(temp, output_file);
	return 0;
}