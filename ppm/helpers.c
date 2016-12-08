#include <stdio.h>
#include <stdlib.h>


/******************************************************************************
* PEEK NEXT CHARACTER
* Purpose: Check what the next character in a stream is
* Return Value: Returns the next character in the input_file stream.
******************************************************************************/
int peek_next_char(FILE* input_file) {
	int c;
	c = fgetc(input_file);
	ungetc(c, input_file);
	return c;
}

/******************************************************************************
* HANDLE COMMENT
* Purpose: move the file pointer past the end of a comment
******************************************************************************/
void handle_comment(FILE* input_file) {
	while(peek_next_char(input_file) != '\n') {
		fgetc(input_file);
	}
}