TEMPLATE = lib
TARGET = LedConf

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

HEADERS += ledconfplugin.h
SOURCES += ledconfplugin.cpp

OTHER_FILES += LedConf.pluginspec
