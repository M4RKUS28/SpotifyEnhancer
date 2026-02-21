QT       += core gui
#LIBS += -luser32 -lgdi32
LIBS += -lws2_32

win32:RC_ICONS += icons/spotify-32.ico


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/menue.cpp \
    src/spotifymanager.cpp

HEADERS += \
    src/mainwindow.h \
    src/menue.h \
    src/spotifymanager.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/MUpdaterLib/release/ -lMUpdater
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/MUpdaterLib/debug/ -lMUpdater

INCLUDEPATH += $$PWD/libs/MUpdaterLib
DEPENDPATH += $$PWD/libs/MUpdaterLib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/release/libMUpdater.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/debug/libMUpdater.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/release/MUpdater.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/MUpdaterLib/debug/MUpdater.lib
