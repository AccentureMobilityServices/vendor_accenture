LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	GLES2.cpp \
	GLRenderer.cpp \
	OpenGLES2Parser.cpp \
	ParserCommon.cpp \
	main.cpp \
	glproxy_context.cpp \
	GLee.c \
	pp.cpp

ifeq ($(HOST_OS),windows)
	LOCAL_SRC_FILES+= Win32SharedMemory.cpp
else
	LOCAL_SRC_FILES+= PosixSharedMemory.cpp
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += vendor/accenture/opengles2emulator/include
LOCAL_CFLAGS := -fPIC

LOCAL_CFLAGS += -m32
LOCAL_LDFLAGS += -m32

ifeq ($(HOST_OS),darwin)
LOCAL_LDFLAGS += -framework OpenGL -framework GLUT -framework Carbon
endif

ifeq ($(HOST_OS),linux)
LOCAL_LDFLAGS += -lGL -lglut
endif

ifeq ($(HOST_OS),windows)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/win32/freeglut/include
LOCAL_LDLIBS += -lopengl32 -lws2_32 -lfreeglut -L$(LOCAL_PATH)/win32/freeglut/lib
endif
LOCAL_MODULE:= glproxy 
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)

