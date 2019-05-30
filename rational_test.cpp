#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include "tinymp.cpp"
#include "rational.cpp"

namespace utf = boost::unit_test;

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

const int LT = 0, EQ = 1, GT = 2;
struct comparison
{
	int ln, ld, rn, rd;
	int type;
};
std::ostream& operator<<(std::ostream &os, const comparison& v)
{
	return os << v.ln << '/' << v.ld << ',' << v.rn << '/' << v.rd << ':' << v.type;
}

comparison cases[] = {
	{ -1, 0,  2, 0, LT },
	{  2, 0, -1, 0, GT },
	{  1, 0,  2, 0, EQ },
	{  2, 0,  1, 0, EQ },
	{ -1, 0, -2, 0, EQ },
	{ -2, 0, -1, 0, EQ },

	{  1, 1,  2, 2, EQ },
	{  2, 2,  1, 1, EQ },
	{ -1, 1, -2, 2, EQ },
	{ -2, 2, -1, 1, EQ },
	{  0, 1,  0, 2, EQ },
	{  0, 2,  0, 1, EQ },

	{ -2, 1, -1, 1, LT },
	{ -1, 1,  2, 1, LT },
	{ -1, 1,  0, 1, LT },
	{  0, 1,  1, 1, LT },
	{  1, 1,  2, 1, LT },
	{ -4, 2, -1, 1, LT },
	{ -1, 1,  4, 2, LT },
	{ -2, 2,  0, 1, LT },
	{  0, 1,  2, 2, LT },
	{  2, 2,  2, 1, LT },

	{ -2, 1, -3, 1, GT },
	{  1, 1, -2, 1, GT },
	{  0, 1, -2, 1, GT },
	{  3, 1,  0, 1, GT },
	{  3, 1,  2, 1, GT },
	{ -4, 2, -3, 1, GT },
	{  1, 1, -4, 2, GT },
	{  0, 2, -4, 1, GT },
	{  4, 2,  0, 2, GT },
	{  6, 2,  2, 1, GT }
};

BOOST_DATA_TEST_CASE( rational_comparison, utf::data::make(cases), input )
{
	rational<unsigned short> rsl(input.ln, input.ld), rsr(input.rn, input.rd);
	rational<unsigned int> ril(input.ln, input.ld), rir(input.rn, input.rd);
	switch(input.type) {
	case LT:
	        BOOST_TEST( !( rsl == rsr ) );
	        BOOST_TEST(    rsl != rsr   );
	        BOOST_TEST(    rsl <  rsr   );
	        BOOST_TEST(    rsl <= rsr   );
	        BOOST_TEST( !( rsl >  rsr ) );
	        BOOST_TEST( !( rsl >= rsr ) );
	        BOOST_TEST( !( ril == rir ) );
	        BOOST_TEST(    ril != rir   );
	        BOOST_TEST(    ril <  rir   );
	        BOOST_TEST(    ril <= rir   );
	        BOOST_TEST( !( ril >  rir ) );
	        BOOST_TEST( !( ril >= rir ) );
	        BOOST_TEST( !( rsl == rir ) );
	        BOOST_TEST(    rsl != rir   );
	        BOOST_TEST(    rsl <  rir   );
	        BOOST_TEST(    rsl <= rir   );
	        BOOST_TEST( !( rsl >  rir ) );
	        BOOST_TEST( !( rsl >= rir ) );
	        BOOST_TEST( !( ril == rsr ) );
	        BOOST_TEST(    ril != rsr   );
	        BOOST_TEST(    ril <  rsr   );
	        BOOST_TEST(    ril <= rsr   );
	        BOOST_TEST( !( ril >  rsr ) );
	        BOOST_TEST( !( ril >= rsr ) );
		break;
	case EQ:
	        BOOST_TEST(    rsl == rsr   );
	        BOOST_TEST( !( rsl != rsr ) );
	        BOOST_TEST( !( rsl <  rsr ) );
	        BOOST_TEST(    rsl <= rsr   );
	        BOOST_TEST( !( rsl >  rsr ) );
	        BOOST_TEST(    rsl >= rsr   );
	        BOOST_TEST(    ril == rir   );
	        BOOST_TEST( !( ril != rir ) );
	        BOOST_TEST( !( ril <  rir ) );
	        BOOST_TEST(    ril <= rir   );
	        BOOST_TEST( !( ril >  rir ) );
	        BOOST_TEST(    ril >= rir   );
	        BOOST_TEST(    rsl == rir   );
	        BOOST_TEST( !( rsl != rir ) );
	        BOOST_TEST( !( rsl <  rir ) );
	        BOOST_TEST(    rsl <= rir   );
	        BOOST_TEST( !( rsl >  rir ) );
	        BOOST_TEST(    rsl >= rir   );
	        BOOST_TEST(    ril == rsr   );
	        BOOST_TEST( !( ril != rsr ) );
	        BOOST_TEST( !( ril <  rsr ) );
	        BOOST_TEST(    ril <= rsr   );
	        BOOST_TEST( !( ril >  rsr ) );
	        BOOST_TEST(    ril >= rsr   );
		break;
	case GT:
	        BOOST_TEST( !( rsl == rsr ) );
	        BOOST_TEST(    rsl != rsr   );
	        BOOST_TEST( !( rsl <  rsr ) );
	        BOOST_TEST( !( rsl <= rsr ) );
	        BOOST_TEST(    rsl >  rsr   );
	        BOOST_TEST(    rsl >= rsr   );
	        BOOST_TEST( !( ril == rir ) );
	        BOOST_TEST(    ril != rir   );
	        BOOST_TEST( !( ril <  rir ) );
	        BOOST_TEST( !( ril <= rir ) );
	        BOOST_TEST(    ril >  rir   );
	        BOOST_TEST(    ril >= rir   );
	        BOOST_TEST( !( rsl == rir ) );
	        BOOST_TEST(    rsl != rir   );
	        BOOST_TEST( !( rsl <  rir ) );
	        BOOST_TEST( !( rsl <= rir ) );
	        BOOST_TEST(    rsl >  rir   );
	        BOOST_TEST(    rsl >= rir   );
	        BOOST_TEST( !( ril == rsr ) );
	        BOOST_TEST(    ril != rsr   );
	        BOOST_TEST( !( ril <  rsr ) );
	        BOOST_TEST( !( ril <= rsr ) );
	        BOOST_TEST(    ril >  rsr   );
	        BOOST_TEST(    ril >= rsr   );
		break;
	}
}

BOOST_AUTO_TEST_CASE( rational_arithmetic_additive )
{
}

BOOST_AUTO_TEST_CASE( rational_arithmetic_multiplicative )
{
}
