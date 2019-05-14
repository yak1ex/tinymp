tinymp: tiny multi-precision integer for C++11
==============================================

Motivation
----------

I love C++, so that I always use C++ for competitive programming, with only quite few exception. [GCJ2019 Qualification Round Problem C: Cryptopangrams](https://codingcompetitions.withgoogle.com/codejam/round/0000000000051705/000000000008830b) is such an exception because it requires multi-precision arithmetic. It triggers development of this class and it can solve the problem.

Usage
-----

Major interger operations are implemented so that you can basically use this class as drop-in-replacement of integer types, embedding content of `tinymp.cpp` into your souce. Please refer to `gcj2019qr_c.cpp` and `tinymp_test.cpp`. Implemented operations are as follows:

- Unary arithmetic operators(+, -)
- Additional destructive sign change(flip_)
- Binary arithmetic operators(+, - , *, /, %)
- Additional division operations for getting both of quotinent and remainder (div(nondestructive), div_(destructive))
- Shift operators(<<, >>)
- Corresponding compound assignment operators(=, +=, -=, *=, /=, %=, <<=, >>=)
- Increment/Decrement operators(++, --)
- Comparision operatos(<, >, <=, >=, ==, !=)
- Conversion from std::uint32_t *IMPLICIT CONVERSION PERMITTED*
- Conversion from/to string (stotmp, to_string) *SLIGHTLY DIFFERENT INTERFACES AND SEMANTICS FROM SIMILAR STANDARD FUNCTIONS*
- User-defined literal (_tmp) (binary, octal, decimal and hexadecimal accepted)
- iostream operators(<<, >>) *DOES NOT RESPECT FMTFLAGS except for std::ios_base::basefield and std::ios_base::uppercase*
- std::hash specialization
- (std::swap uses default implementation)

Not-yet implemented operations are as follows:

- Bit-wise opreators(~, &, |, ^)
- Corresponding compound assignment operators(&=, |=, ^=)
- Logical operators(!, &&, ||)
- std::numeric_limits specialization

Implicit conversion from unsigned int is permitted, so that some cases look strange.

```
BOOST_TEST( -10_tmp == -10 ); // FAILS because -10 converts to a positive value
BOOST_TEST( 4294967306_tmp == 4294967306 ); // FAILS because 4294967306 truncates to 10
```

Use user-defined literal unless the value is strictly in the range of std::uint32_t.

License
-------

Creative Commons Zero v1.0 Universal(CC0-1.0)

This means that you can modify and/or use this code as you wrote this code, including any result of the usage is also attributed not to the author but to you.

Notes
-----

- [Karatsuba algorithm](https://en.wikipedia.org/wiki/Karatsuba_algorithm) is experimentally implemented. Asymptotic behavior seems to follow theoretical formula but it can not outperform naive algorithm even for square of 10,000 digits (base-10).
- Division algorithm is not smart. It tries a guess some times for each digits (base-2^32).
- A solution for GCJ2019 QR Problem C using this class is provided as `gcj2019qr_c.cpp`.
- Exception from this class means a logic error INSIDE this class, which is different from usual convention.

Limitation
----------

As described above, primary motivation is for competitive programming, so that it is not appropriate for any production usage.

- Almost all C++ libraries should have their own namespace and declare almost everything in the namespace but this code does not follow the rule.
- There is no specific consideration for exception safety.
- It trusts a given input.
- Performance is not a primary interest.
