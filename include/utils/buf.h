#ifndef RAYMARCHER_BUF_H
#define RAYMARCHER_BUF_H

#include <utils/err.h>
#include <utils/algo.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>

class Buffer {
    uint8_t *data;
    size_t data_size;
    size_t length;
    size_t offset;

public:
    explicit Buffer(size_t size = 1024);

    ~Buffer();

    Buffer(Buffer &&) = delete;

    Buffer &operator=(Buffer &&) = delete;

    Buffer(const Buffer &) = delete;

    Buffer &operator=(const Buffer &) = delete;

    Err read_from_file(const std::filesystem::path &path);

    Err write_to_file(const std::filesystem::path &path);


    Err expand(size_t amt);

    void free_data();

    constexpr void rewind() { offset = 0; }

    constexpr void reset() {
        offset = 0;
        length = 0;
    }

    [[nodiscard]] constexpr size_t size() const { return length; }

    [[nodiscard]] constexpr size_t remaining() const { return length - offset; }

    [[nodiscard]] constexpr uint8_t const *get_data() const { return data; };

    void zero_fill();

    Err read(std::string &ret);

    Err write(const std::string &val);

    template<trivial_type T>
    Err write(const T &val) {
        if (offset + sizeof(T) > data_size) {
            Err result = expand(sizeof(T));
            if (!result) return result;
        }

        memcpy((void *) (data + offset), (void *) &val, sizeof(T));
        offset += sizeof(T);

        if (offset > length) {
            length = offset;
        }

        return {};
    }

    template<trivial_type T>
    Err read(T &ret) {
        if (offset + sizeof(T) > length) {
            return Err("End of buffer encountered while reading.");
        }

        ret = *(T *) (data + offset);
        offset += sizeof(T);
        return {};
    }

    template<typename ...T>
    Err write(const T &...vals) {
        Err result{};
        ((!(result = write(vals))) && ...);
        return result;
    }

    template<typename...T>
    Err read(T &...vals) {
        Err result{};
        ((!(result = read(vals))) && ...);
        return result;
    }

};

#endif //RAYMARCHER_BUF_H
