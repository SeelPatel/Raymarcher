#ifndef RAYMARCHER_ERR_H
#define RAYMARCHER_ERR_H

#include <string>
#include <format>
#include <vector>

struct Err {
    std::vector<std::string> msg_stack;

    Err() = default;

    template<typename ...Args>
    explicit Err(const std::format_string<Args...> &fmt, Args &&... args) {
        msg_stack.push_back(std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename ...Args>
    Err &add(const std::format_string<Args...> &fmt, Args &&... args) {
        msg_stack.push_back(std::format(fmt, std::forward<Args>(args)...));
        return *this;
    }

    void print() const;

    [[nodiscard]] constexpr bool failure() const {
        return !msg_stack.empty();
    }

    [[nodiscard]] constexpr bool success() const {
        return msg_stack.empty();
    }

    operator bool() const { return !msg_stack.empty(); }
};


#endif //RAYMARCHER_ERR_H
