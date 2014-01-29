CONFIG += qt debug qt
QT += widgets multimedia

DEFINES *= KITCHENTIMER_INTRO_TIMEOUT_MS=1000
DEFINES *= "KITCHENTIMER_SETTINGS_COMPANY_NAME=\"\\\"Brandy Mint\\\"\""
DEFINES *= "KITCHENTIMER_SETTINGS_PRODUCT_NAME=\"\\\"Kitchen Timer\\\"\""


# DEFINES *= KITCHENTIMER_INTRO_TIMEOUT_MS=1000

HEADERS += application.h \
           mainwindow.h \
           pageintro.h \
           pagetimers.h \
           pagetimeredit.h \
           pagedishselect.h \
           timer.h \
           referenceitem.h \
           referencemodel.h \
           customdial.h \
           clickablelabel.h \
           background.h \
           alarmsequencer.h

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
           customdial.cpp \
           clickablelabel.cpp \
           background.cpp \
           alarmsequencer.cpp

           
TRANSLATIONS = translations/ru.ts

lupdate.commands = $$[QT_INSTALL_BINS]/lupdate kitchentimer.pro
lupdate.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS

lrelease.commands = $$[QT_INSTALL_BINS]/lrelease kitchentimer.pro
lupdate.depends = $$TRANSLATIONS

build_android.commands = make && make install INSTALL_ROOT=./android_build/ && $$[QT_INSTALL_BINS]/androiddeployqt --output ./android_build/

QMAKE_EXTRA_TARGETS += lupdate lrelease build_android

RESOURCES += kitchentimer.qrc

MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .qrc


target.path = /usr/bin
INSTALLS += target
