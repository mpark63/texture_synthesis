# Define the compiler and flags
CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g

# Links together files needed to create executables
project: image.o ppm.o texture_synthesis.o project.o 
	$(CC) project.o image.o ppm.o texture_synthesis.o -o project -lm

test: image.o ppm.o test.o texture_synthesis.o
	$(CC) test.o image.o ppm.o texture_synthesis.o -o test -lm -g

# Compiles image.c to create image.o
# image.c depends on image.h
image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

# Compiles ppm.c to create ppm.o
# ppm.c depends on ppm.h
ppm.o: ppm.c ppm.h
	$(CC) $(CFLAGS) -c ppm.c

# Compiles texture_synthesis.c to create texture_synthesis.o
# texture_synthesis.c depends on texture_synthesis.h and image.h
texture_synthesis.o: texture_synthesis.c texture_synthesis.h image.h
	$(CC) $(CFLAGS) -c texture_synthesis.c

# Compiles project.c to create project.o
# project.c depends on image.h, ppm.h and texture_synthesis.h
project.o: project.c image.h ppm.h texture_synthesis.h
	$(CC) $(CFLAGS) -c project.c 

# Removes all object files and the executable named main,
clean:
	rm -f *.o project test
