OBJS=main.o GLRenderer.o PosixSharedMemory.o PosixSemaphore.o OpenGLES2Parser.o OpenGLES1_1Parser.o ParserCommon.o GLES2.o
CC=g++
CFLAGS=-g -fpic -m32
LD=g++ -shared -Wl,-soname,libglproxy.so -m32
INCLUDES=-I ../include

all: lib standalone

lib: $(OBJS)
	$(LD) $(OBJS) -o libglproxy.so.1.0.1 -lGL -lGLU -lglut

standalone: $(OBJS) proxystartup.o
	g++ -g -m32 -o glproxy proxystartup.cpp $(OBJS) -lGL -lGLU -lglut
.c.o:
	gcc $(CFLAGS) $(INCLUDES) -c $<  -o $@
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@
clean:
	@echo Cleaning up...
	@rm -f $(OBJS) libglproxy.so.1.0.1 
	@echo Done.
install:
	cp libglproxy.so.1.0.1 /usr/local/lib
	ldconfig
