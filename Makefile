PROJECT = shannon
LIBPROJECT = lib$(PROJECT).a
TESTPROJECT = test-$(PROJECT)

CXX = g++
AR = ar
ARFLAGS = rcs

CXXFLAGS = -I. -std=c++20 -Wall -Wextra -O3 -march=native
LDFLAGS = -L. -l:$(LIBPROJECT)
LDFLAGS_TEST = $(LDFLAGS) -lgtest -lgtest_main -lpthread

SRC = Encoder.cpp Decoder.cpp Dictionary.cpp main.cpp
OBJ = $(SRC:.cpp=.o)

TEST_SRC = tests.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)

DEPS = Encoder.h Decoder.h Dictionary.h

MAIN_SRC = main.cpp
MAIN_OBJ = main.o

.PHONY: default all clean cleanall test

default: all

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(LIBPROJECT): Encoder.o Decoder.o Dictionary.o
	$(AR) $(ARFLAGS) $@ $^

$(PROJECT): $(MAIN_OBJ) $(LIBPROJECT)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TESTPROJECT): $(TEST_OBJ) $(LIBPROJECT)
	$(CXX) -o $@ $^ $(LDFLAGS_TEST)

test: $(TESTPROJECT)
	./$(TESTPROJECT)

all: $(PROJECT)

clean:
	rm -f *.o

cleanall: clean
	rm -f $(PROJECT) $(LIBPROJECT) $(TESTPROJECT) stats.log
