CXX = gcc
CXXFLAGS = -g -O2 -Wextra -Wall -Werror -Wno-sign-compare -Wno-unused-result 

all: static_main2 static_main1 solution

# Static main creating
static_main2: static_main realization2
	$(CXX) $(CXXFLAGS) static_main.o realization2.o -o static_main2
	
static_main1: static_main realization1
	$(CXX) $(CXXFLAGS) static_main.o realization1.o -o static_main1
	
static_main: static_main.c
	$(CXX) $(CXXFLAGS) -c static_main.c

realization2: realization2.c functions.h
	$(CXX) $(CXXFLAGS) realization2.c -c
	
realization1: realization1.c functions.h
	$(CXX) $(CXXFLAGS) realization1.c -c

# Dynamic main creating

solution: dynamic_main dyn_realization2 dyn_realization1
	$(CXX) $(CXXFLAGS) dynamic_main.o -o solution -ldl
	
dynamic_main: dynamic_main.c
	$(CXX) $(CXXFLAGS) -c dynamic_main.c -ldl

dyn_realization2: realization2.c functions.h
	$(CXX) $(CXXFLAGS) -fPIC -shared -o realization2.so realization2.c

dyn_realization1: realization1.c functions.h
	$(CXX) $(CXXFLAGS) -fPIC -shared -o realization1.so realization1.c 

clean:
	rm -rf *.o

