#include <boost/test/unit_test.hpp>

#include "tinymp.cpp"
#include "rational.cpp"

BOOST_AUTO_TEST_CASE( rational_construct )
{
	rational<unsigned short> rs1, rs2(1), rs3(0, 2);
	rational<unsigned int> ri1, ri2(1), ri3(0, 2), ri4(rs3);
	rational<tinymp> rt1, rt2(5), rt3(1, 5), rt4(-rt3), rt5(rs3), rt6(ri3);
	BOOST_TEST( rs3 == ri4 );
	BOOST_TEST( ri4 == rs3 );
	BOOST_TEST( ri3 == rt6 );
	BOOST_TEST( rt6 == ri3 );
	BOOST_TEST( rs3 == rt5 );
	BOOST_TEST( rt5 == rs3 );
}

BOOST_AUTO_TEST_CASE( rational_comparison )
{
	rational<unsigned short> rs1, rs2(1), rs3(0, 2), rs4(2, 2), rs5(1, 0), rs6(2, 0);
	rational<unsigned int> ri1, ri2(1), ri3(0, 2), ri4(2, 2);
	BOOST_TEST(    rs1 == rs1   );
	BOOST_TEST( !( rs1 == rs2 ) );
	BOOST_TEST( !( rs2 == rs1 ) );
	BOOST_TEST(    rs1 == rs3   );
	BOOST_TEST(    rs3 == rs1   );
	BOOST_TEST(    rs2 == rs4   );
	BOOST_TEST(    rs4 == rs2   );
	BOOST_TEST( !( rs2 == rs3 ) );
	BOOST_TEST( !( rs3 == rs2 ) );
	BOOST_TEST(    rs5 == rs6   );
	BOOST_TEST(    rs6 == rs5   );
}
