#include <utils/buf.h>

#include <cstring>
#include <fstream>

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

Err Buffer::write_to_file(const std::filesystem::path &path) {
    std::ofstream file(path.c_str(), std::ios::binary);

    if (!file.is_open() || file.fail()) {
        return Err("Failed to open file.");
    }

    file.write(reinterpret_cast<const char *>(data), length);

    if (file.fail()) {
        return Err("Failed to write buffer to file");
    }

    return {};
}

Err Buffer::read_from_file(const std::filesystem::path &path) {
    free_data();

    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);

    if (!file.is_open() || file.fail()) {
        return Err("Failed to open file.");
    }

    length = file.tellg();
    file.seekg(0, std::ios::beg);

    data = static_cast<uint8_t *>(malloc(length));

    if (data == nullptr) {
        return Err("Out of memory when allocating buffer.");
    }

    if (!file.read(reinterpret_cast<char *>(data), length)) {
        return Err("Failed to read from file.");
    }

    return {};
}

Err Buffer::read(std::string &ret) {
    uint16_t str_len;

    if (Err result = read(str_len)) {
        return result;
    }

    ret = std::string(data + offset, data + offset + str_len);
    offset += str_len;

    return {};
}

Err Buffer::write(const std::string &val) {
    const uint16_t str_len = val.size();

    if (Err result = write(str_len)) {
        return result;
    }

    Err err;
    if (str_len + offset > data_size && (err = expand(str_len))) {
        return err.add("Failed to write string to buffer.");
    }

    memcpy(data + offset, val.c_str(), str_len);
    offset += str_len;

    if (offset > length) {
        length = offset;
    }

    return {};
}
