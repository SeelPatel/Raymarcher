#include <utils/buf.h>

#include <cstring>

Buffer::Buffer(const size_t size) {
    data_size = size;
    offset = 0;
    length = 0;
    data = static_cast<uint8_t *>(malloc(data_size));
}

Buffer::~Buffer() {
    free_data();
}

void Buffer::free_data() {
    if (data) {
        free(data);
    }
}

Err Buffer::expand(const size_t amt) {
    size_t new_size = data_size;

    while (new_size < data_size + amt) {
        new_size *= 2;
    }

    data_size = new_size;
    data = static_cast<uint8_t *>(realloc(data, data_size));

    return (data == nullptr) ? Err("Could not allocate memory.") : Err();
}

void Buffer::zero_fill() {
    if (data == nullptr) return;
    std::memset(data, 0, data_size);
    offset = 0;
    length = data_size;
}

