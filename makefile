CPP := g++


TARGET := UnifiedHDFTest.exe
OBJECTS := test.cpp

CPPFLAGS := ""

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

default: $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS)

all: default

