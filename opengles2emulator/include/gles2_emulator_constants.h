/*
**
** Copyright 2011, Accenture Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef GLES2_EMULATOR_CONSTANTS_H_
#define GLES2_EMULATOR_CONSTANTS_H_


//Unique identifier used for each buffer sent to/from host
#define GLES2_DEVICE_HEADER 0x4703F322

// Virtual device ID
#define theVirtualDeviceFilename "/dev/virtual_device"
#define theVirtualDeviceIOCTLFilename "/dev/virtual_device_control"

// Host Buffer Size - This is the buffer of commands which are shared with the host
#define HostBufferSize 1048576 //1MB

// Command Buffer - These buffers are used to pass command data to/from OpenGLES2 Android library
#define CommandBufferSize 4096
#define NumberCommandBuffers 2

/* IRQ function command list. Must match between Android and QEMU */
enum sharedBuffer_commands {
	VIRTUALDEVICE_INT_STATUS	= 0, 
	VIRTUALDEVICE_INITIALISE = 8, 
	SET_INPUT_BUFFER_1_ADDRESS = 12,
	SET_INPUT_BUFFER_2_ADDRESS = 16,
	SET_OUTPUT_BUFFER_1_ADDRESS = 20,
	SET_OUTPUT_BUFFER_2_ADDRESS = 24,
	VIRTUALDEVICE_INPUT_BUFFER_1_AVAILABLE = 28,
	VIRTUALDEVICE_INPUT_BUFFER_2_AVAILABLE = 32,
	VIRTUALDEVICE_OUTPUT_BUFFER_1_AVAILABLE = 36,
	VIRTUALDEVICE_OUTPUT_BUFFER_2_AVAILABLE = 40,
	VIRTUALDEVICE_START_INPUT = 44,
	VIRTUALDEVICE_IOCTL_ALLOCATE_SHAREDMEM = 48,
	VIRTUALDEVICE_IOCTL_SYSTEM_RESET = 52,
	VIRTUALDEVICE_IOCTL_GRALLOC_ALLOCATED_REGION_INFO = 56,
	VIRTUALDEVICE_IOCTL_SIGNAL_BUFFER_SYNC = 60,
	VIRTUALDEVICE_IOCTL_REGION_PHYSICAL_ADDR_START = 64,
};

/* IRQ signal flags.  Must match between Android and QEMU */
enum sharedBuffer_interrupt_signals {
	VIRTUALDEVICE_INT_INPUT_BUFFER_1_FULL = 1U << 0, 
	VIRTUALDEVICE_INT_INPUT_BUFFER_2_FULL = 1U << 1, 
	VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY = 1U << 2, 
	VIRTUALDEVICE_INT_OUTPUT_BUFFER_2_EMPTY = 1U << 3, 
};
	
enum sharedBuffer_interrupt_mask {
	VIRTUALDEVICE_INT_MASK = VIRTUALDEVICE_INT_INPUT_BUFFER_1_FULL |  VIRTUALDEVICE_INT_INPUT_BUFFER_2_FULL |
	VIRTUALDEVICE_INT_OUTPUT_BUFFER_1_EMPTY |  VIRTUALDEVICE_INT_OUTPUT_BUFFER_2_EMPTY,
}; 


typedef enum _pixel_format {
	EGL_COLOR_INDEX=0x00000001,
	EGL_STENCIL_INDEX,
	EGL_DEPTH_COMPONENT, 
	EGL_RED,
        EGL_GREEN,
	EGL_BLUE,
	EGL_ALPHA,
	EGL_RGB,
	EGL_RGBA,
	EGL_LUMINANCE,
	EGL_LUMINANCE_ALPHA
} pixel_format;

typedef enum _pixel_type {
	    EGL_UNSIGNED_BYTE = 0x00000001,
	    EGL_BYTE,
	    EGL_BITMAP,
	    EGL_UNSIGNED_SHORT,
	    EGL_SHORT,
	    EGL_UNSIGNED_INT,
	    EGL_INT,
	    EGL_FLOAT,
	    EGL_UNSIGNED_BYTE_3_3_2,
	    EGL_UNSIGNED_BYTE_2_3_3_REV,
	    EGL_UNSIGNED_SHORT_5_6_5,
	    EGL_UNSIGNED_SHORT_5_6_5_REV,
	    EGL_UNSIGNED_SHORT_4_4_4_4,
	    EGL_UNSIGNED_SHORT_4_4_4_4_REV,
	    EGL_UNSIGNED_SHORT_5_5_5_1,
	    EGL_UNSIGNED_SHORT_1_5_5_5_REV,
	    EGL_UNSIGNED_INT_8_8_8_8,
	    EGL_UNSIGNED_INT_8_8_8_8_REV,
	    EGL_UNSIGNED_INT_10_10_10_2,
	    EGL_UNSIGNED_INT_2_10_10_10_REV
} pixel_type; 

typedef struct _Eglsurface_desc
{
      int egl14command;
      int pid;
      int phyaddr;
      int virtaddr;
      int width;
      int height;
      pixel_format pixelformat;
      pixel_type pixeltype;
} Eglsurface_desc;

enum command_type{
GLES11 = 0x00000001,
GLES20,
EGL14
};

enum virtual_function{
GLCLEAR = 0x00000001,
GLCLEARCOLORX,
GLCLEARCOLORF,
GLCREATEPROGRAM,
GLCREATESHADER,
GLSHADERSOURCE,
GLUSEPROGRAM,
GLVERTEXATTRIBPOINTER,
GLENABLEVERTEXATTRIBARRAY,
GLATTACHSHADER,
GLLINKPROGRAM,
GLGETPROGRAMIV,
GLGETPROGRAMINFOLOG,
GLDELETEPROGRAM,
GLGETATTRIBLOCATION,
GLCOMPILESHADER,
GLGETSHADERIV,
GLGETSHADERINFOLOG,
GLDELETESHADER,
GLCLEARDEPTHF,
GLCLEARDSTENCIL,
GLDRAWARRAYS,
GLDRAWELEMENTS,
GLGETUNIFORMLOCATION,
GLUNIFORM3FV,
GLUNIFORMMATRIX4FV,
GLVIEWPORT,
GLENABLE,
GLDISABLE


};

struct command_control
{
	int virtualDeviceMagicNumber;
    int length;                        
	int command;
	int sequenceNum;
	int glFunction;
};


#endif 
