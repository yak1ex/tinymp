#define BOOST_TEST_MODULE tinymp_test
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/timer/timer.hpp>
#include <unordered_map>

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

tinymp vals[] = { -10000000000000000000000_tmp, -4294967296_tmp, -1_tmp, 0, 1, 4294967295_tmp, 10000000000000000000000_tmp };

BOOST_AUTO_TEST_CASE( tinymp_construct )
{
	BOOST_TEST( tinymp() == 0 );
	BOOST_TEST( tinymp(10) == 10 );
	BOOST_TEST( tinymp(10, false) == -10_tmp );
	BOOST_TEST( tinymp("1234567890", 10) == 1234567890_tmp );
	std::vector<tinymp::value_type> v;
	BOOST_TEST( tinymp(v.begin(), v.end()) == 0 );
	v.push_back(10);
	BOOST_TEST( tinymp(v.begin(), v.end()) == 10 );
	v.push_back(1);
	BOOST_TEST( tinymp(v.begin(), v.end()) == 4294967306_tmp );

	BOOST_TEST( 0b100000000000000000000000000000000_tmp == 0x100000000_tmp );
	BOOST_TEST( 0B100000000000000000000000000000000_tmp == 0X100000000_tmp );
	BOOST_TEST( 0b100000000000000000000000000000000_tmp == 040000000000_tmp );
	BOOST_TEST( 0b100000000000000000000000000000000_tmp == 4294967296_tmp );

// NOTE: If you use C++14, you can use digit separator like 0x1'FFFFFFFF'FFFFFFFF'FFFFFFFF_tmp.
//        0x1'FFFFFFFF'FFFFFFFF'FFFFFFFF
	BOOST_TEST( 0x1FFFFFFFFFFFFFFFFFFFFFFFF_tmp == 158456325028528675187087900671_tmp );
//          0xFFFFFFFF'FFFFFFFF'00000000
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF00000000_tmp == 79228162514264337589248983040_tmp );
// 0xFFFFFFFF'FFFFFFFF'00000000'00000000
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp == 340282366920938463444927863358058659840_tmp );
//          0xFFFFFFFF'FFFFFFFF'FFFFFFFF
	BOOST_TEST( 0xFFFFFFFFFFFFFFFFFFFFFFFF_tmp == 79228162514264337593543950335_tmp );
// 0xFFFFFFFF'00000000'00000000'00000000
	BOOST_TEST( 0xFFFFFFFF000000000000000000000000_tmp == 340282366841710300949110269838224261120_tmp );
}

BOOST_AUTO_TEST_CASE( tinymp_pitfall )
{
	BOOST_TEST( -10_tmp != -10 ); // because -10 converts to a positive value
	BOOST_TEST( 4294967306_tmp != 4294967306 ); // because 4294967306 truncates to 10
}

BOOST_AUTO_TEST_CASE( tinymp_arith_shift )
{
	tinymp t1(1), t2(1), t3(1), t4(1);
	t3 <<= 100; t4 <<= 100;
	for(std::size_t i = 1; i < 100; ++i) {
		t1 *= 2;
		t2 <<= 1;
		t3 /= 2;
		t4 >>= 1;
		tinymp tt1(t1), tt2(t2), tt3(t3), tt4(t4);
		for(std::size_t j = 1; j < 100; ++j) {
			tt1 *= 2;
			tt2 <<= 1;
			tt3 /= 2;
			tt4 >>= 1;
			BOOST_TEST_CONTEXT( "i = " << i << " j = " << j ) {
				BOOST_TEST( tt1 == tt2 );
				BOOST_TEST( tt1 == t2 << j );
				BOOST_TEST( tt3 == tt4 );
				BOOST_TEST( tt3 == t4 >> j );
			}
		}
	}
}

BOOST_AUTO_TEST_CASE( tinymp_arith_additive )
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
	BOOST_TEST( to_string(tinymp(0xFFFFFFFFUL)-tinymp(0)) == "4294967295" );
	// 0xFFFFFFFFFFFFFFFF + 0x10000000000000001 == 0x0x20000000000000000
	BOOST_TEST( 18446744073709551615_tmp + 18446744073709551617_tmp == 36893488147419103232_tmp );
	// 0x10000000000000000 - 0xFFFFFFFFFFFFFFFF == 0x1
	BOOST_TEST( 18446744073709551616_tmp - 18446744073709551615_tmp == 1_tmp );

	tinymp unit = 4294967296_tmp, t = 1;
	for(std::size_t i = 1; i <= 10; ++i) {
		t *= unit;
		tinymp tt(t);
		tt -= 1;
		BOOST_TEST_CONTEXT( "i = " << i ) {
			BOOST_TEST( tt + 1 == t ); // carry
			BOOST_TEST( t - 1 == tt ); // borrow
			BOOST_TEST( ++tt == t ); // pre-increment
			BOOST_TEST( t - 1 == --tt ); // pre-decrement
			BOOST_TEST( tt++ == t - 1 ); // post-increment
			BOOST_TEST( tt == t );
			BOOST_TEST( t == tt-- ); // post-decrement
			BOOST_TEST( t - 1 == tt );
		}
	}
}

BOOST_AUTO_TEST_CASE( tinymp_arith_multiplicative )
{
	const std::size_t digits = 100; // arbitrary

	BOOST_TEST( to_string(tinymp(1)*100000*100000*100000*100000) == "100000000000000000000" );
	BOOST_TEST( to_string(-12345678901234567890123467890_tmp) ==  "-12345678901234567890123467890" );
	BOOST_TEST( to_string(-1234567890_tmp + 12345678901234567890_tmp) == "12345678900000000000" );
	BOOST_TEST( to_string(10_tmp * 10_tmp) == "100" );

	BOOST_TEST( to_string(100000000000000000000_tmp  * 10000000000_tmp)  == "1000000000000000000000000000000" );
	BOOST_TEST( to_string(100000000000000000000_tmp  * -10000000000_tmp) == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * 10000000000_tmp)  == "-1000000000000000000000000000000" );
	BOOST_TEST( to_string(-100000000000000000000_tmp * -10000000000_tmp) == "1000000000000000000000000000000" );
	BOOST_TEST( 100000000000_tmp / 1000000000000_tmp == 0 );
	BOOST_TEST( 100000000000_tmp % 1000000000000_tmp == 100000000000_tmp );

// 0x1'FFFFFFFF'FFFFFFFF'FFFFFFFF / 0xFFFFFFFF'FFFFFFFF'00000000
	BOOST_TEST( 0x1FFFFFFFFFFFFFFFFFFFFFFFF_tmp / 0xFFFFFFFFFFFFFFFF00000000_tmp == 2_tmp );
	BOOST_TEST( 0x1FFFFFFFFFFFFFFFFFFFFFFFF_tmp % 0xFFFFFFFFFFFFFFFF00000000_tmp == 0x1FFFFFFFF_tmp );

// 0xFFFFFFFF'FFFFFFFF'00000000'00000000 / 0xFFFFFFFF'FFFFFFFF'00000000
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp / 0xFFFFFFFFFFFFFFFF00000000_tmp == 0x100000000_tmp );
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp % 0xFFFFFFFFFFFFFFFF00000000_tmp == 0_tmp );

// 0xFFFFFFFF'FFFFFFFF'00000000'00000000 / 0xFFFFFFFF'FFFFFFFF'FFFFFFFF
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp / 0xFFFFFFFFFFFFFFFFFFFFFFFF_tmp == 0xFFFFFFFF_tmp );
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp % 0xFFFFFFFFFFFFFFFFFFFFFFFF_tmp == 0xFFFFFFFF00000000FFFFFFFF_tmp );

// 0xFFFFFFFF'00000000'00000000'00000000 / 0xFFFFFFFF'FFFFFFFF'FFFFFFFF
	BOOST_TEST( 0xFFFFFFFF000000000000000000000000_tmp / 0xFFFFFFFFFFFFFFFFFFFFFFFF_tmp == 0xFFFFFFFF_tmp );
	BOOST_TEST( 0xFFFFFFFF000000000000000000000000_tmp % 0xFFFFFFFFFFFFFFFFFFFFFFFF_tmp == 0xFFFFFFFF_tmp );

// 0xFFFFFFFF'FFFFFFFF'00000000'00000000 / 0x1'FFFFFFFF'FFFFFFFF'00000000
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp / 0x1FFFFFFFFFFFFFFFF00000000_tmp == 0x7FFFFFFF_tmp );
	BOOST_TEST( 0xFFFFFFFFFFFFFFFF0000000000000000_tmp % 0x1FFFFFFFFFFFFFFFF00000000_tmp == 0X1FFFFFFFF7FFFFFFF00000000_tmp );

	tinymp t = 100;
	unsigned int n = 10;
	for(std::size_t i = 2; i <= std::numeric_limits<unsigned int>::digits10; ++i) {
		BOOST_TEST_CONTEXT( "i = " << i ) {
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
		BOOST_TEST_CONTEXT( "i = " << i ) {
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
			BOOST_TEST_CONTEXT( "i = " << i << ", j = " << j) {
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
			BOOST_TEST_CONTEXT( "i = " << i << " j = " << j ) {
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
	BOOST_TEST( !(val1 == val0 ) );
	BOOST_TEST(   val0 != val1 );
	BOOST_TEST(   val1 != val0 );
	BOOST_TEST( !(val0 != val0 ) );
	BOOST_TEST(   val0 <  val1 );
	BOOST_TEST( !(val0 <  val0 ) );
	BOOST_TEST( !(val1 <  val0 ) );
	BOOST_TEST(   val0 <= val0 );
	BOOST_TEST(   val0 <= val1 );
	BOOST_TEST( !(val1 <= val0 ) );
	BOOST_TEST(   val1 >  val0 );
	BOOST_TEST( !(val0 >  val0 ) );
	BOOST_TEST( !(val0 >  val1 ) );
	BOOST_TEST(   val1 >= val0 );
	BOOST_TEST(   val0 >= val0 );
	BOOST_TEST( !(val0 >= val1 ) );
}

tinymp in(const std::string& s, std::ios_base::fmtflags base = std::ios_base::dec)
{
	tinymp t;
	std::istringstream iss(s);
	iss.setf(base, std::ios_base::basefield);
	iss >> t;
	return t;
}

std::string out(const tinymp &v, std::ios_base::fmtflags base = std::ios_base::dec, bool upper = false)
{
	std::ostringstream oss;
	oss.setf(base, std::ios_base::basefield);
	if(upper) oss << std::uppercase;
	oss << v;
	return oss.str();
}

BOOST_AUTO_TEST_CASE( tinymp_io )
{
	tinymp t1, t2;
	t1 = in("123456789012345678901234567890");
	BOOST_TEST( to_string(t1) == "123456789012345678901234567890" );
	t2 = in("-123456789012345678901234567890");
	BOOST_TEST( to_string(t2) == "-123456789012345678901234567890" );
	BOOST_TEST( to_string(t1+t2) == "0" );
	BOOST_TEST( to_string(t1-t2) == "246913578024691357802469135780" );
	BOOST_TEST( out(t1) == to_string(t1) );
	BOOST_TEST( out(t2) == to_string(t2) );
	BOOST_TEST( stotmp("123456789012345678901234567890") == 123456789012345678901234567890_tmp );
	BOOST_TEST( to_string(123456789012345678901234567890_tmp) == "123456789012345678901234567890" );

	BOOST_TEST( in("100000000", std::ios_base::hex) == in("040000000000", std::ios_base::oct) );
	BOOST_TEST( in("-100000000", std::ios_base::hex) == in("-040000000000", std::ios_base::oct) );
	BOOST_TEST( in("100000000", std::ios_base::hex) == in("4294967296") );
	BOOST_TEST( in("-100000000", std::ios_base::hex) == in("-4294967296") );
	BOOST_TEST( in("1FFFFFFFFFFFFFFFFFFFFFFFF", std::ios_base::hex) == 158456325028528675187087900671_tmp );
	BOOST_TEST( in("-1FFFFFFFFFFFFFFFFFFFFFFFF", std::ios_base::hex) == -158456325028528675187087900671_tmp );
	BOOST_TEST( in("FFFFFFFFFFFFFFFF00000000", std::ios_base::hex) == 79228162514264337589248983040_tmp );
	BOOST_TEST( in("FFFFFFFFFFFFFFFF0000000000000000", std::ios_base::hex) == 340282366920938463444927863358058659840_tmp );
	BOOST_TEST( in("FFFFFFFFFFFFFFFFFFFFFFFF", std::ios_base::hex) == 79228162514264337593543950335_tmp );
	BOOST_TEST( in("FFFFFFFF000000000000000000000000", std::ios_base::hex) == 340282366841710300949110269838224261120_tmp );
	BOOST_TEST( in("1ffffffffffffffffffffffff", std::ios_base::hex) == 158456325028528675187087900671_tmp );
	BOOST_TEST( in("-1ffffffffffffffffffffffff", std::ios_base::hex) == -158456325028528675187087900671_tmp );
	BOOST_TEST( in("ffffffffffffffff00000000", std::ios_base::hex) == 79228162514264337589248983040_tmp );
	BOOST_TEST( in("ffffffffffffffff0000000000000000", std::ios_base::hex) == 340282366920938463444927863358058659840_tmp );
	BOOST_TEST( in("ffffffffffffffffffffffff", std::ios_base::hex) == 79228162514264337593543950335_tmp );
	BOOST_TEST( in("ffffffff000000000000000000000000", std::ios_base::hex) == 340282366841710300949110269838224261120_tmp );

	BOOST_TEST( stotmp("100000000", 16) == stotmp("040000000000", 8) );
	BOOST_TEST( stotmp("-100000000", 16) == stotmp("-040000000000", 8) );
	BOOST_TEST( stotmp("100000000", 16) == stotmp("4294967296") );
	BOOST_TEST( stotmp("-100000000", 16) == stotmp("-4294967296") );
	BOOST_TEST( stotmp("1FFFFFFFFFFFFFFFFFFFFFFFF", 16) == 158456325028528675187087900671_tmp );
	BOOST_TEST( stotmp("-1FFFFFFFFFFFFFFFFFFFFFFFF", 16) == -158456325028528675187087900671_tmp );
	BOOST_TEST( stotmp("FFFFFFFFFFFFFFFF00000000", 16) == 79228162514264337589248983040_tmp );
	BOOST_TEST( stotmp("FFFFFFFFFFFFFFFF0000000000000000", 16) == 340282366920938463444927863358058659840_tmp );
	BOOST_TEST( stotmp("FFFFFFFFFFFFFFFFFFFFFFFF", 16) == 79228162514264337593543950335_tmp );
	BOOST_TEST( stotmp("FFFFFFFF000000000000000000000000", 16) == 340282366841710300949110269838224261120_tmp );
	BOOST_TEST( stotmp("1ffffffffffffffffffffffff", 16) == 158456325028528675187087900671_tmp );
	BOOST_TEST( stotmp("-1ffffffffffffffffffffffff", 16) == -158456325028528675187087900671_tmp );
	BOOST_TEST( stotmp("ffffffffffffffff00000000", 16) == 79228162514264337589248983040_tmp );
	BOOST_TEST( stotmp("ffffffffffffffff0000000000000000", 16) == 340282366920938463444927863358058659840_tmp );
	BOOST_TEST( stotmp("ffffffffffffffffffffffff", 16) == 79228162514264337593543950335_tmp );
	BOOST_TEST( stotmp("ffffffff000000000000000000000000", 16) == 340282366841710300949110269838224261120_tmp );

	BOOST_TEST( out(4294967296_tmp, std::ios_base::hex) == "100000000" );
	BOOST_TEST( out(-4294967296_tmp, std::ios_base::hex) == "-100000000" );
	BOOST_TEST( out(4294967296_tmp, std::ios_base::dec) == "4294967296" );
	BOOST_TEST( out(-4294967296_tmp, std::ios_base::dec) == "-4294967296" );
	BOOST_TEST( out(4294967296_tmp, std::ios_base::oct) == "40000000000" );
	BOOST_TEST( out(-4294967296_tmp, std::ios_base::oct) == "-40000000000" );
	BOOST_TEST( std::string("1FFFFFFFFFFFFFFFFFFFFFFFF") == out(158456325028528675187087900671_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("-1FFFFFFFFFFFFFFFFFFFFFFFF") == out(-158456325028528675187087900671_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("FFFFFFFFFFFFFFFF00000000") == out(79228162514264337589248983040_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("FFFFFFFFFFFFFFFF0000000000000000") == out(340282366920938463444927863358058659840_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("FFFFFFFFFFFFFFFFFFFFFFFF") == out(79228162514264337593543950335_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("FFFFFFFF000000000000000000000000") == out(340282366841710300949110269838224261120_tmp, std::ios_base::hex, true) );
	BOOST_TEST( std::string("1ffffffffffffffffffffffff") == out(158456325028528675187087900671_tmp, std::ios_base::hex) );
	BOOST_TEST( std::string("-1ffffffffffffffffffffffff") == out(-158456325028528675187087900671_tmp, std::ios_base::hex) );
	BOOST_TEST( std::string("ffffffffffffffff00000000") == out(79228162514264337589248983040_tmp, std::ios_base::hex) );
	BOOST_TEST( std::string("ffffffffffffffff0000000000000000") == out(340282366920938463444927863358058659840_tmp, std::ios_base::hex) );
	BOOST_TEST( std::string("ffffffffffffffffffffffff") == out(79228162514264337593543950335_tmp, std::ios_base::hex) );
	BOOST_TEST( std::string("ffffffff000000000000000000000000") == out(340282366841710300949110269838224261120_tmp, std::ios_base::hex) );

	BOOST_TEST( to_string(4294967296_tmp, 16) == "100000000" );
	BOOST_TEST( to_string(-4294967296_tmp, 16) == "-100000000" );
	BOOST_TEST( to_string(4294967296_tmp, 10) == "4294967296" );
	BOOST_TEST( to_string(-4294967296_tmp, 10) == "-4294967296" );
	BOOST_TEST( to_string(4294967296_tmp, 8) == "40000000000" );
	BOOST_TEST( to_string(-4294967296_tmp, 8) == "-40000000000" );
	BOOST_TEST( std::string("1ffffffffffffffffffffffff") == to_string(158456325028528675187087900671_tmp, 16) );
	BOOST_TEST( std::string("-1ffffffffffffffffffffffff") == to_string(-158456325028528675187087900671_tmp, 16) );
	BOOST_TEST( std::string("ffffffffffffffff00000000") == to_string(79228162514264337589248983040_tmp, 16) );
	BOOST_TEST( std::string("ffffffffffffffff0000000000000000") == to_string(340282366920938463444927863358058659840_tmp, 16) );
	BOOST_TEST( std::string("ffffffffffffffffffffffff") == to_string(79228162514264337593543950335_tmp, 16) );
	BOOST_TEST( std::string("ffffffff000000000000000000000000") == to_string(340282366841710300949110269838224261120_tmp, 16) );
}

// std::is_swappable and std::is_nothrow_swappable are available only from C++17
namespace {
    using std::swap;

    struct swappable
    {
        template<typename T, typename = decltype(swap(std::declval<T&>(), std::declval<T&>()))>
        static std::true_type test(int);

        template<typename>
        static std::false_type test(...);
    };
    template<typename T> struct is_swappable_impl { typedef decltype(swappable::test<T>(0)) type; };
    template<typename T> struct is_swappable : public is_swappable_impl<T>::type {};

    struct nothrow_swappable
    {
        template<typename T>
        static std::integral_constant<bool, noexcept(swap(std::declval<T&>(), std::declval<T&>()))> test(int);

        template<typename>
        static std::false_type test(...);
    };
    template<typename T> struct is_nothrow_swappable_impl { typedef decltype(nothrow_swappable::test<T>(0)) type; };
    template<typename T> struct is_nothrow_swappable : public is_nothrow_swappable_impl<T>::type {};
}

BOOST_AUTO_TEST_CASE( tinymp_stdif )
{
	BOOST_TEST( std::is_default_constructible<tinymp>::value == true );
	BOOST_TEST( std::is_trivially_default_constructible<tinymp>::value == false );
	BOOST_TEST( std::is_nothrow_default_constructible<tinymp>::value == false );
	BOOST_TEST( std::is_copy_constructible<tinymp>::value == true );
	BOOST_TEST( std::is_trivially_copy_constructible<tinymp>::value == false );
	BOOST_TEST( std::is_nothrow_copy_constructible<tinymp>::value == false );
	BOOST_TEST( std::is_move_constructible<tinymp>::value == true );
	BOOST_TEST( std::is_trivially_move_constructible<tinymp>::value == false );
	BOOST_TEST( std::is_nothrow_move_constructible<tinymp>::value == true );
	BOOST_TEST(      is_swappable<tinymp>::value == true );
	BOOST_TEST(      is_nothrow_swappable<tinymp>::value == true );

	auto hasher = std::hash<tinymp>{};
	BOOST_TEST( hasher(10000000000_tmp) == hasher(10000000000_tmp) );
	BOOST_TEST( hasher(tinymp()) == hasher(tinymp()) );
	BOOST_TEST( hasher(tinymp(10000000000_tmp)) != hasher(tinymp()) );

	std::unordered_map<tinymp, tinymp> hmap;
	hmap[100000_tmp] = 10000000000_tmp;
	BOOST_TEST( hmap.count(100000_tmp) == 1 );
	BOOST_TEST( hmap[100000_tmp] == 10000000000_tmp );
	BOOST_TEST( hmap.count(1000000_tmp) == 0 );
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
