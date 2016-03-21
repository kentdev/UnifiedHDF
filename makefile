CPP := g++

ifneq ($(strip $(UHDF_DEBUG)),)
DEBUG := -g
endif

TARGET := UnifiedHDFTest.exe
OBJECTS := test.cpp

FLAGS := -std=c++11 $(DEBUG)
LIBRARIES := -ldf -lmfhdf -lhdf5

%.o: %.cpp
	$(CPP) $(FLAGS) -o $@ $<

default: $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS) $(FLAGS) $(LIBRARIES)

all: default

