#ifndef RAYMARCHER_BUFFER_H
#define RAYMARCHER_BUFFER_H

#include <glad/glad.h>
#include <utils/buf.h>

namespace compute {
    class ComputeBuffer {
        // GPU Data
        GLuint ssbo_id;

        // CPU Data
        Buffer buf;

    public:
        Err init();

        explicit ComputeBuffer(size_t size);

        template<trivial_type ...T>
        Err write(const T &...vals) {
            return buf.write(vals...);
        }

        template<trivial_type ...T>
        Err read(T &...vals) {
            return buf.read(vals...);
        }

        constexpr void rewind() { buf.rewind(); }

        constexpr void reset() { buf.reset(); }

        [[nodiscard]] constexpr size_t size() const { return buf.size(); }

        [[nodiscard]] constexpr size_t remaining() const { return buf.remaining(); }

        [[nodiscard]] constexpr GLuint id() const { return ssbo_id; }

        [[nodiscard]] constexpr uint8_t const *data() const { return buf.get_data(); }

        constexpr void zero_fill() { buf.zero_fill(); }

        void bind() const;

        void transfer_to_gpu() const;
    };
}

#endif //RAYMARCHER_BUFFER_H
