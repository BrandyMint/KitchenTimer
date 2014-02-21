CONFIG += qt release
QT += widgets multimedia gui-private svg

android {
QT += androidextras opengl
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
}
ios {
QTPLUGIN += qtaudio_coreaudio
}


DEFINES += "KITCHENTIMER_SETTINGS_COMPANY_NAME=\"\\\"Brandy Mint\\\"\""
DEFINES += "KITCHENTIMER_SETTINGS_PRODUCT_NAME=\"\\\"Kitchen Timer\\\"\""

DEFINES += KITCHENTIMER_ANIMATION_REPAINT_TIMEOUT_MS=10
DEFINES += KITCHENTIMER_LONG_PRESS_TIMEOUT_MS=1000

DEFINES += KITCHENTIMER_INTRO_TIMEOUT_MS=1000

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

TRANSLATIONS = translations/ru.ts

lupdate.commands = $$[QT_INSTALL_BINS]/lupdate kitchentimer.pro
lupdate.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS

lrelease.commands = $$[QT_INSTALL_BINS]/lrelease kitchentimer.pro
lupdate.depends = $$TRANSLATIONS

QMAKE_EXTRA_TARGETS += lupdate lrelease

android {
android_build.commands = make && make install INSTALL_ROOT=./android-build/ && $$[QT_INSTALL_BINS]/androiddeployqt --output ./android-build/
android_build_release.commands = make && make install INSTALL_ROOT=./android-build/ && $$[QT_INSTALL_BINS]/androiddeployqt --release --output ./android-build/
android_install.commands = adb install ./android-build/bin/QtApp-debug.apk
android_uninstall.commands = adb uninstall com.brandymint.kitchentimer
android_reinstall.commands = adb uninstall com.brandymint.kitchentimer; adb install ./android-build/bin/QtApp-debug.apk
android_exec.commands = adb shell am start -n com.brandymint.kitchentimer/org.qtproject.qt5.android.bindings.QtActivity
run.commands = make -j 4 android_build && make android_reinstall && make android_exec

QMAKE_EXTRA_TARGETS += android_build android_build_release android_install android_uninstall android_reinstall android_exec run
}

unix:!android {
run.commands = make && ./kitchentimer
QMAKE_EXTRA_TARGETS += run
}

RESOURCES += resources/images.qrc resources/misc.qrc

!ios {
RESOURCES += resources/audio-mono/audio-mono.qrc
}
ios {
RESOURCES += resources/audio-stereo/audio-stereo.qrc
}


MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .qrc
