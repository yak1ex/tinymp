# Some GCC versions can not suppress note for misleading-indentation by pragma
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81334
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84889
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89549
CXXFLAGS=-O3 -Wall -Wextra -std=c++11 -Wno-misleading-indentation
GCJEX=gcj2019qr_c.cpp

run: all
	./ut.exe --color_output=yes --log_level=message

runall: all
	./ut.exe --color_output=yes --log_level=message --run_test=\*

log: all
	./ut.exe --color_output=no --log_level=message --run_test=\* > `date +%Y%m%d%H%M%S`.txt 2>&1

all: ut.exe tinymp.cpp $(GCJEX)

clean:
	-rm *.o *.exe

ut.exe: tinymp_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_test_exec_monitor -lboost_timer

tinymp_test.o: tinymp.all.cpp

tinymp.cpp: tinymp.all.cpp
	unifdef -x 2 -UTINYMP_DEBUG -UTINYMP_KARATSUBA tinymp.all.cpp > tinymp.cpp

$(GCJEX): $(GCJEX).in tinymp.cpp
	sed -e '/\/\/ INCLUDE_TINYMP/,$$d' $(GCJEX).in > $@
	grep -v '#include' tinymp.cpp >> $@
	sed -e '0,/\/\/ INCLUDE_TINYMP/d' $(GCJEX).in >> $@
