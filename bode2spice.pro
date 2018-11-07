QT += widgets printsupport

HEADERS       = mainwindow.h \
    qcustomplot.h
SOURCES       = main.cpp \
                mainwindow.cpp \
    qcustomplot.cpp

win32:RC_ICONS += images/bode2spice.ico

RESOURCES     = bode2spice.qrc

# install
target.path = bode2spice
INSTALLS += target

FORMS +=

QMAKE_CFLAGS_WARN_ON += -Wno-class-memaccess
QMAKE_CXXFLAGS_WARN_ON += -Wno-class-memaccess
QMAKE_CFLAGS += -Wno-class-memaccess
QMAKE_CXXFLAGS += -Wno-class-memaccess

DISTFILES += \
    images/bode2spice.ico

