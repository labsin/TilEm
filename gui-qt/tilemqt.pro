######################################################################
# Automatically generated by qmake (2.01a) Thu Nov 19 17:10:38 2009
######################################################################

TEMPLATE = app
TARGET = tilemqt
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += debug

enable_link {
	CONFIG += link_pkgconfig
	PKGCONFIG += ticalcs2
	DEFINES += _TILEM_QT_HAS_LINK_
}

# Input
HEADERS += calc.h calclink.h settings.h calcview.h
SOURCES += main.cpp calc.cpp calclink.cpp settings.cpp calcview.cpp

INCLUDEPATH += ../core
LIBS += -L../core -ltilemcore
