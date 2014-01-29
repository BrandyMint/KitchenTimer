KitchenTimer
============

Installation on Android
-----------------------

### Preparing environment

1. Install Android SDK and Android NDK
   (TODO: Fill components to install here)

2. Install JDK (Java Development Kit)

3. Setup environment variables:
   1) Setup ANDROID_HOME variable
   2) Setup JAVA_HOME variable
   3) Setup PATH variable as following:
      $ export $PATH=$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-toos:$JAVA_HOME/bin

4. Build Qt locally:

```
http://download.qt-project.org/official_releases/qt/5.2/5.2.0/single/qt-everywhere-opensource-src-5.2.0.tar.xz
tar xf qt-everywhere-opensource-src-5.2.0.tar.xz
./configure -no-pch -opensource -confirm-license \
-android-ndk /opt/android-ndk-r9c \
-android-sdk /opt/android-sdk-linux \
-xplatform android-g++ \
-no-warnings-are-errors \
-nomake tests -nomake examples \
-skip qtwebkit -skip qtserialport -skip qtwebkit-examples
```

5. Setup environment variables:
   MAke $USER_QT_BUILD_DIR variable point to user Qt build

### Building from git

Execute following commands:
$ git clone git@github.com:BrandyMint/KitchenTimer.git
$ cd ./KitchenTimer/kitchentimer
$ $USER_QT_BUILD_DIR/bin/qmake
$ make build_anroid

Target package is: android_build/bin/QtApp-debug.apk

To install it on Android device use:
$ adb install android_build/bin/QtApp-debug.apk

To uninstall existing package from Android device use:
$ adb uninstall org.qtproject.example.kitchentimer
