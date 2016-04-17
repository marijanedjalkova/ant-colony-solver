
all: vecadd

vecadd: vecadd.c
	gcc -std=c99 vecadd.c -lOpenCL -o vecadd

clean:
	rm -f vecadd *~