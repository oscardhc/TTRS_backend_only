CXX = g++
CXXFLAGS = -Wall -std=c++11 -Ofast

code : fly/test.cpp
	$(CXX) -o code $^ $(CXXFLAGS)

clean:
	rm score -f
