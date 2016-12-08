all:
	gcc image_viewer.c ppm/helpers.c ppm/header.c ppm/p6.c -lglfw3 -framework Cocoa -framework OpenGL -o bin/image_viewer -framework IOKit -framework CoreVideo


clean:
	rm -rf bin/image_viewer *~