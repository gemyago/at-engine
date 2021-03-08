#ifndef test_random_h
#define test_random_h

#include <random>

namespace test
{

template <typename T>
T randomNumber(T min, T max)
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<T> uni(min, max);
    return uni(rng);
}

} // namespace test

#endif