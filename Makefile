# Some GCC versions can not suppress note for misleading-indentation by pragma
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81334
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84889
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89549
CXXFLAGS=-O3 -Wall -Wextra -std=c++11 -Wno-misleading-indentation

run: all
	./ut.exe --color_output=yes --log_level=message

runall: all
	./ut.exe --color_output=yes --log_level=message --run_test=\*

all: ut.exe tinymp.minify.cpp

clean:
	rm *.o *.exe tinymp.minify.cpp

ut.exe: tinymp_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_test_exec_monitor -lboost_timer

tinymp_test.o: tinymp.minify.cpp

tinymp.minify.cpp: tinymp.cpp
	python ./cminify/minifier.py tinymp.cpp | sed -e 's, : ,:,g;s, ? ,?,g;s, \.\.\. ,...,g' > $@