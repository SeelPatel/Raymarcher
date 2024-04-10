#include <utils/err.h>

#include <iostream>
#include <algorithm>

void Err::print() const {
    std::cout << "Errors:" << std::endl;

    std::ranges::for_each(msg_stack, [](const std::string &msg) {
        std::cout << "---- " << msg << std::endl;
    });
}