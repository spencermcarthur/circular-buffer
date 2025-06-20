#include "circularbuffer/IWrapper.hpp"

#include <format>
#include <stdexcept>

#include "Macros.hpp"
#include "SharedMemory.hpp"
#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Spec.hpp"
#include "circularbuffer/State.hpp"
#include "spdlog/spdlog.h"

namespace CircularBuffer {

IWrapper::IWrapper(const Spec &spec) {
    SetupSpdlog();

    // Load/map shared memory regions
    m_StateRegion = new SharedMemory(spec.indexSharedMemoryName, sizeof(State));
    m_DataRegion =
        new SharedMemory(spec.dataSharedMemoryName, spec.bufferCapacity);

    // Reinterpret state region as struct and verify
    m_State = m_StateRegion->AsStruct<State>();
    if (m_State == nullptr) {
        // Fail
        CONSTEXPR_SV fmt =
            "({}:{}) Reinterpretation of index shared memory as struct failed";
        SPDLOG_ERROR(fmt.substr(8));
        throw std::runtime_error(std::format(fmt, __FILE__, __LINE__));
    }

    // Reinterpret buffer region as span and verify
    m_Buffer = m_DataRegion->AsSpan<DataT>();
    if (m_Buffer.data() == nullptr || m_Buffer.empty()) {
        // Fail
        CONSTEXPR_SV fmt =
            "({}:{}) Reinterpretation of buffer data region as span failed";
        SPDLOG_ERROR(fmt.substr(8));
        throw std::runtime_error(std::format(fmt, __FILE__, __LINE__));
    }

    // Set local index
    m_LocalIndex = 0;
}

IWrapper::~IWrapper() {
    m_State = nullptr;

    delete m_DataRegion;
    delete m_StateRegion;
}

}  // namespace CircularBuffer
