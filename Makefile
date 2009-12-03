# DEBUG  := -ggdb
CFLAGS := $(shell gdal-config --cflags) $(DEBUG) -O3 -funroll-loops

# if running GCC 4, need an additional flag
ifneq ($(shell g++ --version | head -1 | grep ' 4\.'), '')
CFLAGS := $(CFLAGS) -frounding-math
endif

LIBS   := $(shell gdal-config --libs) -lCGAL
objects = clustr.o component.o shapefile.o polygon.o

clustr: $(objects)
	g++ $(CFLAGS) -o $@ $+ $(LIBS)

$(objects): %.o: %.cpp
	g++ $(CFLAGS) -c $+

shape_test: stuff/ogr_test.cpp shapefile.cpp
	g++ -I. $(CFLAGS) -o $@ $+ $(LIBS)

all: clustr

clean:
	-rm -f clustr
	-rm -f *.o *.shp *.shx *.dbf *.prj *.gch *.rpo
