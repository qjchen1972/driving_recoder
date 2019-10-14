Some preparations
====

* Build Caffe2 for Android

   * Infer to https://caffe2.ai/docs/getting-started.html?platform=android&configuration=compile

* Install android-ndk

   * download android-ndk from http://developer.android.com/ndk/downloads/index.html
   * unzip android-ndk-r16b-linux-x86_64.zip
   * export ANDROID_NDK= your-ndk-root-path

*  Build Ncnn

   * Infer to https://github.com/Tencent/ncnn/wiki/how-to-build#build-for-android

Build cv_lib
====
  * download opencv for android https://sourceforge.net/projects/opencvlibrary/files/4.1.1/opencv-4.1.1-android-sdk.zip/download
  *  mkdir  build & cd build
  *  cmake ..
  *  make & make install

Build ffmpeg
====
  * down ffmpeg and X264 
  * modify path of the sysroot and cross-prefix with ffmpeg_scripts/x264_android.sh and build x264.lib
  * modify modify path of the sysroot and cross-prefix with ffmpeg_scripts/config.sh and build ffmpeg
  * merge one lib with ffmpeg_scripts/make.sh
