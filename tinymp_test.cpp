#define BOOST_TEST_MODULE tinymp_test
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

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

#include "tinymp.cpp"

using std::to_string;
namespace bdata = boost::unit_test::data;

tinymp vals[] = { -10000000000000000000000_tmp, -1_tmp, 0, 1, 10000000000000000000000_tmp };

BOOST_AUTO_TEST_CASE( tinymp_arith )
{
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)) == "8589934590" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)+-tinymp(0xFFFFFFFFUL)) == "-8589934590" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)+tinymp(0)) == "8589934590" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+tinymp(0)) == "4294967295" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)) == "-4294967295" );
	BOOST_TEST( to_string(tinymp()+(-tinymp(0xFFFFFFFFUL))) == "-4294967295" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)+(-tinymp(0xFFFFFFFFUL))) == "0" );
	BOOST_TEST( to_string(-tinymp(0xFFFFFFFFUL)+tinymp(0xFFFFFFFFUL)) == "0" );
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)-tinymp(0xFFFFFFFFUL)) == "0" );
	BOOST_TEST( to_string(tinymp(1)*100000*100000*100000*100000) == "100000000000000000000" );
	BOOST_TEST( to_string(-12345678901234567890123467890_tmp) ==  "-12345678901234567890123467890" );
	BOOST_TEST( to_string(-1234567890_tmp + 12345678901234567890_tmp) == "12345678900000000000" );
	BOOST_TEST( to_string(10_tmp * 10_tmp) == "100" );
	BOOST_TEST( to_string(100000000000000000000_tmp  * 10000000000_tmp)  == "1000000000000000000000000000000" );
	BOOST_TEST( to_string(100000000000000000000_tmp  * -10000000000_tmp) == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * 10000000000_tmp)  == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * -10000000000_tmp) == "1000000000000000000000000000000" );
	BOOST_TEST( to_string(10000000000000000001_tmp    / 1000000000_tmp)    == "10000000000" );
	BOOST_TEST( to_string(10000000000000000001_tmp    % 1000000000_tmp)    == "1" );
	BOOST_TEST( to_string(10000000000000000001_tmp    / -1000000000_tmp)   == "-10000000000" );
	BOOST_TEST( to_string(10000000000000000001_tmp    % -1000000000_tmp)   == "1" );
	BOOST_TEST( to_string(-10000000000000000001_tmp   / 1000000000_tmp)    == "-10000000000" );
	BOOST_TEST( to_string(-10000000000000000001_tmp   % 1000000000_tmp)    == "1" );
	BOOST_TEST( to_string(-10000000000000000001_tmp   / -1000000000_tmp)   == "10000000000" );
	BOOST_TEST( to_string(-10000000000000000001_tmp   % -1000000000_tmp)   == "1" );
	BOOST_TEST( to_string(10000000000000000001_tmp    / 1000000000)        == "10000000000" );
	BOOST_TEST( to_string(10000000000000000001_tmp    % 1000000000)        == "1" );
	BOOST_TEST( to_string(100000000000000000001_tmp   / 10000000000_tmp)   == "10000000000" );
	BOOST_TEST( to_string(100000000000000000001_tmp   % 10000000000_tmp)   == "1" );
	BOOST_TEST( to_string(1000000000000000000001_tmp  / 100000000000_tmp)  == "10000000000" );
	BOOST_TEST( to_string(1000000000000000000001_tmp  % 100000000000_tmp)  == "1" );
	BOOST_TEST( to_string(10000000000000000000001_tmp / 1000000000000_tmp) == "10000000000" );
	BOOST_TEST( to_string(10000000000000000000001_tmp % 1000000000000_tmp) == "1" );
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

#if 0
// Core i7-3720QM 2.6GHz 1,000,000 times => 43s
BOOST_AUTO_TEST_CASE( tinymp_time )
{
	tinymp p1 = 359334085968622831041960188598043661065388726959079837_tmp;
	tinymp p2 = 265252859812191058636308479999999_tmp;
	tinymp p3 = 8683317618811886495518194401279999999_tmp;
	for(int i = 0; i < 1000000; ++i ) {
		BOOST_TEST( p1 * p2 * p3 / p1 / p2 / p3 == 1 );
	}
}
#endif
