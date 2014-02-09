PRODUCT = shalopie
SOURCES += \
	sharopie/App.cpp \
	sharopie/Source.cpp \
	sharopie/FileMonitor.cpp \
	sharopie/Viewport.cpp

all: $(PRODUCT)
include 3p/kapusha/kapusha.mk
