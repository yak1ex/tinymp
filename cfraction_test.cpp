#include <boost/test/unit_test.hpp>
#include "cfraction.cpp"

BOOST_AUTO_TEST_CASE( cfraction_test )
{
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)) == cfraction<int>({2, 1, 1, 2}) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)).alt() == cfraction<int>({2, 1, 1, 1, 1}) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 5)).alt().alt() == cfraction<int>({2, 1, 1, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(13, 5))) == rational<int>(13, 5) );
	BOOST_TEST( cfraction<int>(rational<int>(13, 100)).fit(rational<int>(14, 100)) == cfraction<int>({0, 7, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(13, 100)).fit(rational<int>(14, 100))) == rational<int>(2, 15) );
	BOOST_TEST( cfraction<int>(rational<int>(1, 2)).fit(rational<int>(1, 3)) == cfraction<int>({0, 2, 2}) );
	BOOST_TEST( rational<int>(cfraction<int>(rational<int>(1, 2)).fit(rational<int>(1, 3))) == rational<int>(2, 5) );
}
