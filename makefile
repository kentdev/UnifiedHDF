CPP := g++
RM := rm -f

ifneq ($(strip $(UHDF_DEBUG)),)
DEBUG := -g3
endif

TARGET := UnifiedHDFTest.exe
OBJECTS := test.o

FLAGS := -std=c++11 $(DEBUG)
LIBRARIES := -ldf -lmfhdf -lhdf5

%.o: %.cpp
	$(CPP) $(FLAGS) -c $<

default: $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS) $(FLAGS) $(LIBRARIES)

all: default

clean:
	$(RM) $(OBJECTS) $(TARGET)
