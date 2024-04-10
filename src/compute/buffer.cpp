#include <compute/buffer.h>

namespace compute {
    ComputeBuffer::ComputeBuffer(const size_t size) : buf(size) {

    }

    Err ComputeBuffer::init() {
        glGenBuffers(1, &ssbo_id);
        return {};
    }

    void ComputeBuffer::bind() const {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    }

    void ComputeBuffer::transfer_to_gpu() const {
        bind();
        glBufferData(GL_SHADER_STORAGE_BUFFER, buf.size(), buf.get_data(), GL_DYNAMIC_READ);
    }
}