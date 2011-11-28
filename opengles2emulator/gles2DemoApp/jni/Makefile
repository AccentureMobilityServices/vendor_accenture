OBJS=main.o gl_code.o globject.o 
CC=g++
CFLAGS=-g
LD=g++ 
UNAME=$(shell uname)
ifeq ($(UNAME), Linux)
GLLIBS=-lGL -lGLU -lglut
endif

ifeq ($(UNAME), Darwin)
GLLIBS=-framework OpenGL -framework GLUT
endif


all: $(OBJS)
	$(LD) $(OBJS) -o gl2demo $(GLLIBS)
.c.o:
	gcc $(CFLAGS) $(INCLUDES) -c $<  -o $@
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@
clean:
	@echo Cleaning up...
	@rm -f $(OBJS) gl2demo 
	@echo Done.
