#ifndef GLHEADERS_H
#define GLHEADERS_H

#ifdef ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#define  LOG_TAG    "gles2Demo"
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif defined __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#define  LOG_TAG    "gl2Demo"
#define  LOGV(...)  printf(__VA_ARGS__);;printf("\n");
#define  LOGI(...)  printf(__VA_ARGS__);printf("\n");
#define  LOGE(...)  printf(__VA_ARGS__);printf("\n");
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#define  LOG_TAG    "gl2Demo"
#define  LOGV(...)  printf(__VA_ARGS__);;printf("\n");
#define  LOGI(...)  printf(__VA_ARGS__);printf("\n");
#define  LOGE(...)  printf(__VA_ARGS__);;printf("\n");
#endif


#endif
