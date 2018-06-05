#include <aaciolib/real.hpp>
#include <aaciolib/aacio.hpp>

#include "test_api.hpp"

void test_real::create_instances() {
    aacio::real lhs1(5);
    aacio_assert(lhs1.value() == 5, "real instance value is wrong");
}

void test_real::test_division() {
    aacio::real lhs1(5);
    aacio::real rhs1(10);
    aacio::real result1 = lhs1 / rhs1;

    uint64_t a = double_div(i64_to_double(5), i64_to_double(10));
    aacio_assert(a == result1.value(), "real division result is wrong");
}

void test_real::test_division_by_0() {
    aacio::real lhs1(5);
    aacio::real rhs1(0);
    aacio::real result1 = lhs1 / rhs1;
    // in order to get rid of unused parameter warning
    result1 = 0;

    aacio_assert(false, "should've thrown an error");
}

void test_real::test_multiplication() {
    aacio::real lhs1(5);
    aacio::real rhs1(10);
    aacio::real result1 = lhs1 * rhs1;
    uint64_t res = double_mult( 5, 10 );
    aacio_assert(res == result1.value(), "real multiplication result is wrong");
}

void test_real::test_addition()
{
    aacio::real lhs1(5);
    aacio::real rhs1(10);
    aacio::real result1 = lhs1 / rhs1;
    uint64_t a = double_div(i64_to_double(5), i64_to_double(10));

    aacio::real lhs2(5);
    aacio::real rhs2(2);
    aacio::real result2 = lhs2 / rhs2;
    uint64_t b = double_div(i64_to_double(5), i64_to_double(2));


    aacio::real sum = result1+result2;
    uint64_t c = double_add( a, b );
    aacio_assert(sum.value() == c, "real addition operation result is wrong");
}


