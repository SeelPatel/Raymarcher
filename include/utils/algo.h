#ifndef RAYMARCHER_ALGO_H
#define RAYMARCHER_ALGO_H

#include <ranges>

template<typename T>
concept trivial_type = std::is_trivial<T>::value && !std::is_pointer<T>::value;

#endif //RAYMARCHER_ALGO_H
