/*
 * Copyright (C) 2009 The Android Open Source Project
 * Copyright (C) 2011 Accenture Ltd
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

package com.accenture.aess.gles2demo;


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


class Gles2DemoView extends GLSurfaceView {
    private static String TAG = "Gles2DemoView";
    private static final boolean DEBUG = false;
    static float rotX, rotY, rotZ;

    public Gles2DemoView(Context context) {
        super(context);
        setEGLContextClientVersion(2);
        
        try {
        	loadShaders();
        	loadModels();
        } catch (IOException e) {
        	e.printStackTrace();
        }
        
        setRenderer(new Renderer());
        
        
    }

    public Gles2DemoView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        setEGLContextClientVersion(2);
        setRenderer(new Renderer());
    }

  
  
    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
        	Gles2DemoLib.step(rotX, rotY, rotZ);
        	Log.v(TAG, "render frame");
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Gles2DemoLib.init(width, height);
            Log.v(TAG, "init");
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.
        }
    }
    
    private byte[] loadBinaryFile(String fileName) throws IOException {
    	AssetManager m = getContext().getAssets();
        
    	InputStream input = m.open(fileName);
    	byte[] bytes = new byte[input.available()];
    	input.read(bytes);
    	return bytes;
    }
    
    private String loadFile(String fileName) throws IOException {
    	AssetManager m = getContext().getAssets();
        
    	InputStream input = m.open(fileName);
    	BufferedReader reader = new BufferedReader(new InputStreamReader(input));
    	String output = "";
    	String line = reader.readLine();
    	while (line != null) {
    		output += line  + "\n";
    		line = reader.readLine();
    	}
    	return output;
    }
  
    
    private void loadShaders() throws IOException {

    	String vertexShader = loadFile("shader.vsh");
    	Log.v(TAG, vertexShader);
    	String fragmentShader = loadFile("shader.fsh");
    	Log.v(TAG, fragmentShader);

    	Gles2DemoLib.setShaders(vertexShader, fragmentShader);	
    }
    private void loadModels()  throws IOException {
      	byte[] model = loadBinaryFile("object.obj");
    	Gles2DemoLib.loadModel(model);	
    	
    }
    
        @Override
        public boolean onTouchEvent(MotionEvent event) {

        rotY = (int) event.getX() * 2;
        rotX = (int) event.getY() * 2;
/*
        switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                break;

                default:
                return super.onTouchEvent(event);

	   }
*/
	   return true;
   	}
   
}
   
