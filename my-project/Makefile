CFLAGS = -Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS = $(CFLAGS)

SOURCES = weather-station.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = weather-station

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION = ..
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME = rgbmatrix
RGB_LIBRARY = $(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS += -L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

all : $(TARGET)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

$(TARGET) : $(OBJECTS) $(RGB_LIBRARY)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<


clean:
	rm -f $(OBJECTS) $(TARGET)

FORCE:
.PHONY: FORCE
