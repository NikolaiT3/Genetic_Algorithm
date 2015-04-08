CC=g++-4.7
CFLAGS= $(fltk-config --use-images  --compile) -c -std=c++11

all: project3

project3: ga1_7.o
	$(CC) $(fltk-config --use-images  --compile) ga1_7conGraphics.o -o project3 -lfltk -lX11
ga1_7.o: *.cpp
	$(CC) $(CFLAGS) ga1_7conGraphics.cpp
clean:
	rm *.o project3