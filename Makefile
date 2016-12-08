all:
	gcc image_viewer.c ppm/helpers.c ppm/header.c ppm/p6.c -lglfw3 -framework Cocoa -framework OpenGL -o bin/imageviewer -framework IOKit -framework CoreVideo
	./bin/imageviewer


clean:
	rm -rf bin/imageviewer *~