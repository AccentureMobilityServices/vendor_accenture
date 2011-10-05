/*
 * Copyright (C) 2009 The Android Open Source Project
 * Copyright (C) 2011 Accenture
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include "globject.h"

bool setupGraphics(int, int);
void renderFrame();
void shutdown();
void setShaders(const char* vertexShader, const char* fragmentShader);
void setModel(const char* objfile);


extern "C" {
    JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_setShaders(JNIEnv * env, jobject obj,  jstring vsh, jstring fsh);
    JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_loadModel(JNIEnv * env, jobject obj,  jbyteArray model);
    JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_shutdown(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}
JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_shutdown(JNIEnv * env, jobject obj)
{
	shutdown();
}

JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_setShaders(JNIEnv * env, jobject obj, jstring vsh, jstring fsh)
{
	const char *vertexShader = env->GetStringUTFChars(vsh, 0);
	const char *fragmentShader = env->GetStringUTFChars(fsh, 0);

	setShaders(vertexShader,fragmentShader);

    env->ReleaseStringUTFChars(vsh, vertexShader);
  	env->ReleaseStringUTFChars(fsh, fragmentShader);

}

JNIEXPORT void JNICALL Java_com_accenture_aess_gles2demo_Gles2DemoLib_loadModel(JNIEnv * env, jobject obj, jbyteArray model)
{
	jbyte *objfile= env->GetByteArrayElements(model, 0);
	setModel((char*)objfile);
  	env->ReleaseByteArrayElements(model, objfile, 0);
}

