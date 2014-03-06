APPLICATION_NAME = kitchentimer

android {
CONFIG += qt release
QT += widgets multimedia svg opengl androidextras gui-private
} ios {
CONFIG += qt release
QT += widgets svg opengl
} !android:!ios {
CONFIG += qt release
QT += widgets multimedia svg opengl
}

android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
}
ios {
QTPLUGIN += qtaudio_coreaudio
}


# DEFINES += KITCHENTIMER_DEBUG_BUILD=1


QMAKE_CXXFLAGS += "-include configuration.h"


HEADERS += configuration.h \
           application.h \
           applicationmanager.h \
           resourcemanager.h \
           mainwindow.h \
           pageintro.h \
           pagetimers.h \
           pagesettings.h \
           timer.h \
           referenceitem.h \
           referencemodel.h \
           alarmsequencer.h \
           audiosequencer.h \
           vibrosequencer.h \
           animator.h \
           editmode.h \
           widgets/analogtimer.h \
           widgets/digitaltimer.h \
           widgets/background.h \
           widgets/introbackground.h \
           widgets/buttonstick.h

SOURCES += main.cpp \
           application.cpp \
           applicationmanager.cpp \
           resourcemanager.cpp \
           mainwindow.cpp \
           pageintro.cpp \
           pagetimers.cpp \
           pagesettings.cpp \
           timer.cpp \
           referenceitem.cpp \
           referencemodel.cpp \
           alarmsequencer.cpp \
           audiosequencer.cpp \
           vibrosequencer.cpp \
           animator.cpp \
           editmode.cpp \
           widgets/analogtimer.cpp \
           widgets/digitaltimer.cpp \
           widgets/background.cpp \
           widgets/introbackground.cpp \
           widgets/buttonstick.cpp

ios {
OBJECTIVE_SOURCES += ios_common.mm
HEADERS += ios_common.h
}

TRANSLATIONS = translations/ru.ts

lupdate.commands = $$[QT_INSTALL_BINS]/lupdate $${APPLICATION_NAME}.pro
lupdate.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS

lrelease.commands = $$[QT_INSTALL_BINS]/lrelease $${APPLICATION_NAME}.pro
lupdate.depends = $$TRANSLATIONS

QMAKE_EXTRA_TARGETS += lupdate lrelease

android {
android_build.commands = make && make install INSTALL_ROOT=./android-build/ && $$[QT_INSTALL_BINS]/androiddeployqt --output ./android-build/
android_build_release.commands = make && make install INSTALL_ROOT=./android-build/ && $$[QT_INSTALL_BINS]/androiddeployqt --release --output ./android-build/
android_install.commands = adb install ./android-build/bin/QtApp-debug.apk
android_uninstall.commands = adb uninstall com.brandymint.$${APPLICATION_NAME}
android_reinstall.commands = adb uninstall com.brandymint.$${APPLICATION_NAME}; adb install ./android-build/bin/QtApp-debug.apk
android_exec.commands = adb shell am start -n com.brandymint.$${APPLICATION_NAME}/.MainActivity
run.commands = make -j 4 android_build && make android_reinstall && make android_exec

QMAKE_EXTRA_TARGETS += android_build android_build_release android_install android_uninstall android_reinstall android_exec run
}

!android:!ios {
run.commands = make && ./$$TARGET
QMAKE_EXTRA_TARGETS += run
}

RESOURCES += resources/images.qrc resources/misc.qrc

!ios {
RESOURCES += resources/audio-mono/audio-mono.qrc
}

ios {
QMAKE_INFO_PLIST = "ios-sources/Qt_Info.plist"

fonts.files = "resources/fonts/Cartonsix NC.ttf"
fonts.path = fonts
QMAKE_BUNDLE_DATA += fonts

audio.files = \
"resources/audio-stereo/audio-enabled.wav" \
"resources/audio-stereo/alarm.wav" \
"resources/audio-stereo/alarm-manual.wav" \
"resources/audio-stereo/timer-start.wav" \
"resources/audio-stereo/analog-timer-press.wav" \
"resources/audio-stereo/analog-timer-release.wav" \
"resources/audio-stereo/analog-timer-slide-01.wav" \
"resources/audio-stereo/analog-timer-slide-02.wav" \
"resources/audio-stereo/analog-timer-slide-03.wav" \
"resources/audio-stereo/analog-timer-slide-04.wav" \
"resources/audio-stereo/analog-timer-slide-05.wav" \
"resources/audio-stereo/analog-timer-slide-06.wav" \
"resources/audio-stereo/analog-timer-slide-07.wav" \
"resources/audio-stereo/analog-timer-slide-08.wav" \
"resources/audio-stereo/analog-timer-slide-09.wav" \
"resources/audio-stereo/analog-timer-slide-10.wav" \
"resources/audio-stereo/analog-timer-slide-11.wav" \
"resources/audio-stereo/analog-timer-slide-12.wav" \
"resources/audio-stereo/analog-timer-slide-13.wav" \
"resources/audio-stereo/analog-timer-slide-14.wav"
audio.path = audio
QMAKE_BUNDLE_DATA += audio

icons.files = "ios-sources/icons/6.1/Icon.png" \
"ios-sources/icons/6.1/Icon@2x.png" \
"ios-sources/icons/6.1/Icon-72.png" \
"ios-sources/icons/6.1/Icon-72@2x.png" \
"ios-sources/icons/6.1/Icon-Small.png" \
"ios-sources/icons/6.1/Icon-Small@2x.png" \
"ios-sources/icons/6.1/Icon-Small-50.png" \
"ios-sources/icons/6.1/Icon-Small-50@2x.png"
QMAKE_BUNDLE_DATA += icons

launchimages.files = "ios-sources/launch-images/Default.png"
QMAKE_BUNDLE_DATA += launchimages
}


MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .qrc
