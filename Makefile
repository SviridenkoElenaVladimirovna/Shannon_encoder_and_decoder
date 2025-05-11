PROJECT = shannon
LIBPROJECT = lib$(PROJECT).a

CXX = g++
AR = ar
ARFLAGS = rcs

CXXFLAGS = -I. -std=c++20 -Wall -Wextra -O3 -march=native
LDFLAGS = -L. -l$(PROJECT)

SRC = Encoder.cpp Decoder.cpp Dictionary.cpp
OBJ = $(SRC:.cpp=.o)

.PHONY: all clean cleanall

all: $(PROJECT)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBPROJECT): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(PROJECT): main.o $(LIBPROJECT)
	rm -f stats.log
	$(CXX) main.o -o $@ $(LDFLAGS)

clean:
	rm -f *.o

cleanall: clean
	rm -f $(PROJECT) $(LIBPROJECT) stats.log
