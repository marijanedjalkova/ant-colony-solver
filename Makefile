
all: vecadd

vecadd: vecadd.c
	gcc -std=c99 vecadd.c -lOpenCL -lm -o vecadd

clean:
	rm -f vecadd *~