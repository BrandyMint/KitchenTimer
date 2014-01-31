CONFIG += qt debug qt
QT += widgets multimedia

DEFINES *= KITCHENTIMER_INTRO_TIMEOUT_MS=1000
DEFINES *= "KITCHENTIMER_SETTINGS_COMPANY_NAME=\"\\\"Brandy Mint\\\"\""
DEFINES *= "KITCHENTIMER_SETTINGS_PRODUCT_NAME=\"\\\"Kitchen Timer\\\"\""

HEADERS += application.h \
           mainwindow.h \
           pageintro.h \
           pagetimers.h \
           pagetimeredit.h \
           pagedishselect.h \
           timer.h \
           referenceitem.h \
           referencemodel.h \
           alarmsequencer.h \
           widgets/customdial.h \
           widgets/analogtimer.h \
           widgets/clickablelabel.h \
           widgets/digitaltimer.h \
           widgets/background.h

SOURCES += main.cpp \
           application.cpp \
           mainwindow.cpp \
           pageintro.cpp \
           pagetimers.cpp \
           pagetimeredit.cpp \
           pagedishselect.cpp \
           timer.cpp \
           referenceitem.cpp \
           referencemodel.cpp \
           alarmsequencer.cpp \
           widgets/customdial.cpp \
           widgets/analogtimer.cpp \
           widgets/clickablelabel.cpp \
           widgets/digitaltimer.cpp \
           widgets/background.cpp

           
TRANSLATIONS = translations/ru.ts

lupdate.commands = $$[QT_INSTALL_BINS]/lupdate kitchentimer.pro
lupdate.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS

lrelease.commands = $$[QT_INSTALL_BINS]/lrelease kitchentimer.pro
lupdate.depends = $$TRANSLATIONS

android_build.commands = make && make install INSTALL_ROOT=./android_build/ && $$[QT_INSTALL_BINS]/androiddeployqt --output ./android_build/
android_install.commands = adb install android_build/bin/QtApp-debug.apk
android_uninstall.commands = adb uninstall org.qtproject.example.kitchentimer
android_reinstall.commands = adb install -r android_build/bin/QtApp-debug.apk

QMAKE_EXTRA_TARGETS += lupdate lrelease android_build android_install android_uninstall android_reinstall

RESOURCES += resources/kitchentimer.qrc

MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .qrc


target.path = /usr/bin
INSTALLS += target
