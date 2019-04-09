CXXFLAGS=-O3 -Wall -Wextra -std=c++14

run: all
	./ut.exe --color_output=yes

all: ut.exe

clean:
	rm *.o *.exe

ut.exe: tinymp_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_test_exec_monitor 

tinymp_test.o: tinymp.cpp
