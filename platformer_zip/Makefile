CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCDIR   = src
SRCS     = $(wildcard $(SRCDIR)/*.cpp)
TARGET   = platformer

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) scores.txt

.PHONY: all clean
