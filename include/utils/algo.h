#ifndef RAYMARCHER_ALGO_H
#define RAYMARCHER_ALGO_H

#include <ranges>

template<typename T>
concept trivial_type = std::is_trivial<T>::value && !std::is_pointer<T>::value;

template<std::unsigned_integral T>
constexpr T ceil_divide(const T x, const T y) { return x / y + (x % y != 0); }

#endif //RAYMARCHER_ALGO_H
