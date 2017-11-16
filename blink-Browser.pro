QT       += widgets core gui webenginewidgets printsupport

TARGET    = blink-Browser
TEMPLATE  = app
DEFINES  += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES  += browser.cpp global.cpp interfacehttp.cpp udp.cpp main.cpp
HEADERS  += browser.h   global.h   interfacehttp.h   udp.h
FORMS    += browser.ui

#Icons & co
macx {
    LIBS              += -framework Carbon
    ICON               = icon.icns
    QMAKE_INFO_PLIST   = Info.plist
    QMAKE_LFLAGS      += -F/Library/Frameworks
    QMAKE_CXXFLAGS    += -Wno-delete-non-virtual-dtor
}
win32:RC_FILE          = icon.rc
RESOURCES             += icono/Ressources.qrc
