#define BOOST_TEST_MODULE tinymp_test
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/timer/timer.hpp>

template<typename T>
inline T mygcd(T m, T n)
{
	if(m < n) std::swap(m, n);
	while(n != 0) {
		T r = m % n;
		m = std::move(n);
		n = std::move(r);
	}
	return std::move(m);
}

#define TINYMP_KARATSUBA
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#include "tinymp.all.cpp"
#pragma GCC diagnostic warning "-Wmisleading-indentation"

using std::to_string;
namespace bdata = boost::unit_test::data;

tinymp vals[] = { -10000000000000000000000_tmp, -1_tmp, 0, 1, 10000000000000000000000_tmp };

BOOST_AUTO_TEST_CASE( tinymp_arith )
{
	const std::size_t digits = 100; // arbitrary
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)) == "8589934590" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)+-tinymp(0xFFFFFFFFUL)) == "-8589934590" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)+tinymp(0)) == "8589934590" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0)) == "4294967295" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)) == "-4294967295" );
	BOOST_TEST( to_string(tinymp()+(-tinymp(0xFFFFFFFFUL))) == "-4294967295" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+(-tinymp(0xFFFFFFFFUL))) == "0" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)) == "0" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)-tinymp(0xFFFFFFFFUL)) == "0" );
	// 0xFFFFFFFFFFFFFFFF + 0x10000000000000001 == 0x0x20000000000000000
	BOOST_TEST( 18446744073709551615_tmp + 18446744073709551617_tmp == 36893488147419103232_tmp );
	// 0x10000000000000000 - 0xFFFFFFFFFFFFFFFF == 0x1
	BOOST_TEST( 18446744073709551616_tmp - 18446744073709551615_tmp == 1_tmp );

	BOOST_TEST( to_string(tinymp(1)*100000*100000*100000*100000) == "100000000000000000000" );
	BOOST_TEST( to_string(-12345678901234567890123467890_tmp) ==  "-12345678901234567890123467890" );
	BOOST_TEST( to_string(-1234567890_tmp + 12345678901234567890_tmp) == "12345678900000000000" );
	BOOST_TEST( to_string(10_tmp * 10_tmp) == "100" );
	
	BOOST_TEST( to_string(100000000000000000000_tmp  * 10000000000_tmp)  == "1000000000000000000000000000000" );
	BOOST_TEST( to_string(100000000000000000000_tmp  * -10000000000_tmp) == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * 10000000000_tmp)  == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * -10000000000_tmp) == "1000000000000000000000000000000" );
	tinymp t(100);
	unsigned int n = 10;
	for(std::size_t i = 2; i <= std::numeric_limits<unsigned int>::digits10; ++i) {
		BOOST_TEST_CONTEXT( "i = " << to_string(i) ) {
			BOOST_TEST( to_string( (t+1) /  n) == "10" );
			BOOST_TEST( to_string( (t+1) %  n) == "1" );
			BOOST_TEST( to_string(-(t+1) /  n) == "-10" );
			BOOST_TEST( to_string(-(t+1) %  n) == "-1" );
		}
		t *= 10;
		n *= 10;
	}
	t = 100;
	tinymp t2 = 10;
	for(std::size_t i = 2; i < digits; ++i) { 
		BOOST_TEST_CONTEXT( "i = " << to_string(i) ) {
			BOOST_TEST( to_string( (t+1) /  t2) == "10" );
			BOOST_TEST( to_string( (t+1) %  t2) == "1" );
			BOOST_TEST( to_string(-(t+1) /  t2) == "-10" );
			BOOST_TEST( to_string(-(t+1) %  t2) == "-1" );
			BOOST_TEST( to_string( (t+1) / -t2) == "-10" );
			BOOST_TEST( to_string( (t+1) % -t2) == "1" );
			BOOST_TEST( to_string(-(t+1) / -t2) == "10" );
			BOOST_TEST( to_string(-(t+1) % -t2) == "-1" );
		}
		t *= 10;
		t2 = t2 * 10;
	}
	t2 = tinymp(10);
	for(std::size_t i = 2; i < digits; ++i) {
		t = t2 * 10;
		std::string s("10");
		for(std::size_t j = i + 1; j < digits; ++j) {
			BOOST_TEST_CONTEXT( "i = " << to_string(i) << ", j = " << j) {
				std::string ms = std::string("-") + s;
				BOOST_TEST( to_string( (t+1) /  t2) == s );
				BOOST_TEST( to_string( (t+1) %  t2) == "1" );
				BOOST_TEST( to_string(-(t+1) /  t2) == ms );
				BOOST_TEST( to_string(-(t+1) %  t2) == "-1" );
				BOOST_TEST( to_string( (t+1) / -t2) == ms );
				BOOST_TEST( to_string( (t+1) % -t2) == "1" );
				BOOST_TEST( to_string(-(t+1) / -t2) == s );
				BOOST_TEST( to_string(-(t+1) % -t2) == "-1" );
			}
			t *= 10;
			s += "0";
		}
		t2 *= 10;
	}

#ifdef TINYMP_KARATSUBA
	BOOST_TEST( 100_tmp .mult(100_tmp) == 10000_tmp );
	BOOST_TEST( 1000000000000_tmp .mult(1000000000000_tmp) == 1000000000000000000000000_tmp );
	BOOST_TEST( 10000000000000000000000_tmp .mult(10000000000000000000000_tmp) == 100000000000000000000000000000000000000000000_tmp );
	tinymp t3 = 1;
	for(std::size_t i = 1; i < 100; ++i) {
		t3 *= 16;
		tinymp t4 = 1;
		for(std::size_t j = 1; j < 100; ++j) {
			t4 *= 16;
			tinymp t5 = t3 - 1, t6 = t4 - 1;
			BOOST_TEST_CONTEXT( "i = " << to_string(i) << " j = " << to_string(j) ) {
				BOOST_TEST( t5 * t6 == t5.mult(t6) );
			}
		}
	}
#endif

	BOOST_TEST( to_string(mygcd(10000000000000000000001_tmp, 1000000000000_tmp)) == "1" );
	BOOST_TEST( to_string(mygcd(11111111111111111111_tmp, 1111111111_tmp)) ==  "1111111111" );

	tinymp p1 = 359334085968622831041960188598043661065388726959079837_tmp;
	tinymp p2 = 265252859812191058636308479999999_tmp;
	tinymp p3 = 8683317618811886495518194401279999999_tmp;
	BOOST_TEST( to_string(-p1/p1) == "-1" );
	BOOST_TEST( to_string(p2/-p2) == "-1" );
	BOOST_TEST( to_string(p3/p3) == "1" );
	BOOST_TEST( to_string(p1*p2) == to_string(p2*p1));
	BOOST_TEST( to_string(p2*p3/p3) == to_string(p2) );
	BOOST_TEST( to_string(p3*p1/p1) == to_string(p3) );
	BOOST_TEST( to_string(mygcd(p1 * p2, p2 * p3)) == to_string(p2) );
	BOOST_TEST( to_string(mygcd(p2 * p1, p1 * p3)) == to_string(p1) );
	BOOST_TEST( to_string(mygcd(p1 * p3, p3 * p2)) == to_string(p3) );
}

BOOST_DATA_TEST_CASE( tinymp_comparison, bdata::make(vals), val0 )
{
	tinymp val1 = val0 + 1;
	BOOST_TEST(   val0 == val0 );
	BOOST_TEST( !(val0 == val1 ) );
	BOOST_TEST(   val0 != val1 );
	BOOST_TEST( !(val0 != val0 ) );
	BOOST_TEST(   val0 <  val1 );
	BOOST_TEST( !(val0 <  val0 ) );
	BOOST_TEST(   val0 <= val0 );
	BOOST_TEST( !(val1 <= val0 ) );
	BOOST_TEST(   val1 >  val0 );
	BOOST_TEST( !(val0 >  val0 ) );
	BOOST_TEST(   val1 >= val0 );
	BOOST_TEST( !(val0 >= val1 ) );
}

BOOST_AUTO_TEST_CASE( tinymp_io )
{
	tinymp t1, t2;
	std::istringstream iss("123456789012345678901234567890"); iss >> t1;
	BOOST_TEST( to_string(t1) == "123456789012345678901234567890" );
	iss.str("-123456789012345678901234567890"); iss.seekg(0); iss >> t2;
	BOOST_TEST( to_string(t2) == "-123456789012345678901234567890" );
	BOOST_TEST( to_string(t1+t2) == "0" );
	BOOST_TEST( to_string(t1-t2) == "246913578024691357802469135780" );
	std::ostringstream oss;
	oss << t1;
	BOOST_TEST( oss.str() == to_string(t1) );
	oss.str("");
	oss << t2;
	BOOST_TEST( oss.str() == to_string(t2) );
	BOOST_TEST( stotmp("123456789012345678901234567890") == 123456789012345678901234567890_tmp );
	BOOST_TEST( to_string(123456789012345678901234567890_tmp) == "123456789012345678901234567890" );
}

BOOST_AUTO_TEST_CASE( tinymp_time, *boost::unit_test::disabled() )
{
	BOOST_TEST_MESSAGE( "[multiplication for same size]" );
	for(std::size_t i = 400; i <= 10000; i+= 400) {
		std::ostringstream oss;
		tinymp t1(1);
		for(std::size_t j = 0; j < i; ++j) t1 *= 10;
		tinymp t2(t1);
		{
			boost::timer::auto_cpu_timer t(oss, "%w");
			for(int j = 0; j < 1000; ++j ) {
				BOOST_TEST( t1 * t2 > 0 );
			}
		}
		BOOST_TEST_MESSAGE( i << ":" << oss.str() << "s" );
	}
#ifdef TINYMP_KARATSUBA
	BOOST_TEST_MESSAGE( "[multiplication by Karatsuba for same size]" );
	for(std::size_t i = 400; i <= 10000; i+= 400) {
		std::ostringstream oss;
		tinymp t1(1);
		for(std::size_t j = 0; j < i; ++j) t1 *= 10;
		tinymp t2(t1);
		{
			boost::timer::auto_cpu_timer t(oss, "%w");
			for(int j = 0; j < 1000; ++j ) {
				BOOST_TEST( t1.mult(t2) > 0 );
			}
		}
		BOOST_TEST_MESSAGE( i << ":" << oss.str() << "s" );
	}
#endif
	BOOST_TEST_MESSAGE( "[division for changing width-difference and constant-width-divisor]" );
	std::string base;
	for(std::size_t i = 0; i < 600; ++i) { base += "123456789"; }
	for(std::size_t i = 200; i <= 4000; i+= 200) {
		std::ostringstream oss;
		tinymp t1 = stotmp(base.substr(i%9, 100+i+i%9));
		tinymp t2 = stotmp(base.substr(i%9, 100+i%9));
		{
			boost::timer::auto_cpu_timer t(oss, "%w");
			for(int j = 0; j < 1000; ++j ) {
				BOOST_TEST( t1 / t2 >= 0 );
			}
		}
		BOOST_TEST_MESSAGE( i << ":" << oss.str() << "s" );
	}
	BOOST_TEST_MESSAGE( "[division for 100-width-difference and changing divisor]" );
	for(std::size_t i = 200; i <= 4000; i+= 200) {
		std::ostringstream oss;
		tinymp t1 = stotmp(base.substr(i%9, 100+i+i%9));
		tinymp t2 = stotmp(base.substr(i%9, i+i%9));
		{
			boost::timer::auto_cpu_timer t(oss, "%w");
			for(int j = 0; j < 1000; ++j ) {
				BOOST_TEST( t1 / t2 >= 0 );
			}
		}
		BOOST_TEST_MESSAGE( i << ":" << oss.str() << "s" );
	}
	BOOST_TEST_MESSAGE( "[division for changing width-difference and divisor]" );
	for(std::size_t i = 200; i <= 2000; i+= 200) {
		std::ostringstream oss;
		tinymp t1 = stotmp(base.substr(i%9, i+i+i%9));
		tinymp t2 = stotmp(base.substr(i%9, i+i%9));
		{
			boost::timer::auto_cpu_timer t(oss, "%w");
			for(int j = 0; j < 1000; ++j ) {
				BOOST_TEST( t1 / t2 >= 0 );
			}
		}
		BOOST_TEST_MESSAGE( i << ":" << oss.str() << "s" );
	}
}
