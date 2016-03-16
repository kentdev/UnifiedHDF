CPP := g++


TARGET := UnifiedHDFTest.exe
OBJECTS := test.cpp

FLAGS := -std=c++11
LIBRARIES := -lmfhdf -lhdf5

%.o: %.cpp
	$(CPP) $(FLAGS) -o $@ $<

default: $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS) $(FLAGS) $(LIBRARIES)

all: default

