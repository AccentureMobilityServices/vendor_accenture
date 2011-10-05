README

To build:

From eclipse:
Create New Android Project, Choose "From existing source" and select this directory.
Eclipse does not rebuild the jni library. To rebuild you should use "ndk-build" (From the android ndk)

From android build structure

. build/envsetup.sh
mmm packages/apps/gles2Demo/gles2Demo

(you may have to delete the gen directory if you've been building with eclipse previously)
This will install the apk into $ANDROID_PRODUCT_OUT/system/app
