// TYPE DEFINITIONS
typedef struct Header {
	char magic_number[2];
	int width;
	int height;
	int max_val;
} Header;

typedef struct Pixel {
	double r;
	double g;
	double b;
} Pixel;

// FUNCTION SIGNATURES
int read_header(FILE* input_file, Header *file_header);
int write_header(FILE* output_file, char *format, Header file_header);