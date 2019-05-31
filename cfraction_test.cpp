#include <boost/test/unit_test.hpp>
#include "cfraction.cpp"

BOOST_AUTO_TEST_CASE( cfraction_test )
{
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)) == cfraction<int>({2, 1, 1, 2}) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)).alt() == cfraction<int>({2, 1, 1, 1, 1}) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)).alt().alt() == cfraction<int>({2, 1, 1, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(13, 5))) == rational<int>(13, 5) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 100)).best(rational<int>(14, 100)) == cfraction<int>({0, 7, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(13, 100)).best(rational<int>(14, 100))) == rational<int>(2, 15) );
	BOOST_TEST( cfraction<int>(rational<int>(1, 2)).best(rational<int>(1, 3)) == cfraction<int>({0, 2, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(1, 2)).best(rational<int>(1, 3))) == rational<int>(2, 5) );
	// example from https://en.wikipedia.org/wiki/Continued_fraction#Best_rational_approximations
	std::vector<cfraction<int>> v, vr = {
		{ 0, 1 },
		{ 0, 1, 3 },
		{ 0, 1, 4 },
		{ 0, 1, 5 },
		{ 0, 1, 5, 2 },
		{ 0, 1, 5, 2, 1 },
		{ 0, 1, 5, 2, 2 }
	};
	cfraction<int>(rational<int>(84375, 100000)).foreach_approx([&v](const cfraction<int> &p){
		v.push_back(p); return false;
	});
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(4) == rational<int>(3, 4) );
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(5) == rational<int>(4, 5) );
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(6) == rational<int>(5, 6) );
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(13) == rational<int>(11, 13) );
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(19) == rational<int>(16, 19) );
	BOOST_TEST( cfraction<int>(rational<int>(84375, 100000)).approx(32) == rational<int>(27, 32) );

	BOOST_TEST( cfraction<int>(rational<int>(314159, 100000)).approx(1000) == rational<int>(355, 113) );
}
