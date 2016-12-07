all: texdemo.c
	gcc texdemo.c -lglfw3 -framework Cocoa -framework OpenGL -o texdemo -framework IOKit -framework CoreVideo
	./texdemo


clean:
	rm -rf texdemo *~