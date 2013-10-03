.PHONY: all clean
KAPUSHA_ROOT=3p/kapusha
include $(KAPUSHA_ROOT)/common.mk

SOURCES = \
	sharopie/Viewport.cpp \
	sharopie/App.cpp

MODULES=$(SOURCES:.cpp=.o)
DEPENDS=$(SOURCES:.cpp=.d)

shalopie: $(MODULES) $(KAPUSHA_ROOT)/libkapusha.a $(KAPUSHA_ROOT)/libkapusha_main.a
	$(LD) $(MODULES) -lkapusha_main -lkapusha $(LDFLAGS) -L$(KAPUSHA_ROOT) -o $@

-include $(DEPENDS)

$(KAPUSHA_ROOT)/libkapusha.a:
	make -C $(KAPUSHA_ROOT) libkapusha.a

$(KAPUSHA_ROOT)/libkapusha_main.a:
	make -C $(KAPUSHA_ROOT) libkapusha_main.a

clean:
	@rm -f $(MODULES) $(DEPENDS) sharopie
