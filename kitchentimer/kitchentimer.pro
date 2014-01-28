CONFIG += qt debug
QT += widgets

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
           referencemodel.h

SOURCES += main.cpp \
           application.cpp \
           mainwindow.cpp \
           pageintro.cpp \
           pagetimers.cpp \
           pagetimeredit.cpp \
           pagedishselect.cpp \
           timer.cpp \
           referenceitem.cpp \
           referencemodel.cpp

           
TRANSLATIONS = translations/ru.ts

lupdate.commands = $$[QT_INSTALL_BINS]/lupdate kitchentimer.pro
lupdate.depends = $$SOURCES $$HEADERS $$FORMS $$TRANSLATIONS

lrelease.commands = $$[QT_INSTALL_BINS]/lrelease kitchentimer.pro
lupdate.depends = $$TRANSLATIONS

QMAKE_EXTRA_TARGETS += lupdate lrelease

RESOURCES += kitchentimer.qrc

MOC_DIR = .moc
OBJECTS_DIR = .obj
RCC_DIR = .qrc


target.path = /usr/bin
INSTALLS += target
