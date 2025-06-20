#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Macros.hpp"

using MsgSize = CircularBuffer::MessageSizeT;

// Generates random wait times using exponential distribution
class WaitTimeGenerator {
    static constexpr uint64_t NANOS_PER_MILLI = 1'000'000;

public:
    explicit WaitTimeGenerator(double meanArrivalTimeMillis)
        : m_Dist(1.0 / meanArrivalTimeMillis) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
    }

    // Generate random exponential arrival time with nanosecond precision
    std::chrono::nanoseconds operator()() {
        double millis = m_Dist(m_Rng);
        return std::chrono::nanoseconds(
            static_cast<uint64_t>(millis * NANOS_PER_MILLI));
    }

private:
    std::default_random_engine m_Rng;
    std::exponential_distribution<double> m_Dist;
};

// Generates random message length using normal distribution
class MessageSizeGenerator {
public:
    explicit MessageSizeGenerator(double meanSize, double stdDev)
        : m_Dist(meanSize, stdDev) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
    }

    CB_EXPLICIT_DELETE_COPY_MOVE(MessageSizeGenerator);
    MsgSize operator()() { return Clip(m_Dist(m_Rng)); }

private:
    static MsgSize Clip(double value) {
        static constexpr MsgSize min = 1;
        static constexpr MsgSize max = std::numeric_limits<uint16_t>::max();

        MsgSize ret;
        if (value < min) {
            ret = min;
        } else if (value > max) {
            ret = max;
        } else {
            ret = value;
        }

        return ret;
    }

    std::default_random_engine m_Rng;
    std::normal_distribution<double> m_Dist;
};

// Generates random data
class RandomMessage {
    using data_t = std::byte;
    static constexpr uint8_t MIN_BYTE_VAL = std::numeric_limits<uint8_t>::min();
    static constexpr uint8_t MAX_BYTE_VAL = std::numeric_limits<uint8_t>::max();

public:
    explicit RandomMessage(MsgSize size) : m_Size(size) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
        m_Data = new data_t[m_Size]{};

        for (MsgSize i = 0; i < m_Size; i++) {
            m_Data[i] = data_t(m_Dist(m_Rng));
        }
    }

    ~RandomMessage() { delete[] m_Data; }

    CB_EXPLICIT_DELETE_CONSTRUCTORS(RandomMessage);

    [[nodiscard]] data_t* data() const { return m_Data; }
    [[nodiscard]] MsgSize size() const { return m_Size; }
    [[nodiscard]] std::span<data_t> buf() const {
        return {m_Data, static_cast<size_t>(m_Size)};
    }

    friend bool operator==(const RandomMessage& a, const RandomMessage& b) {
        auto m1 = a.buf();
        auto m2 = b.buf();

        if (m1.size() != m2.size()) {
            return false;
        }

        for (MsgSize i = 0; i < a.size(); i++) {
            if (m1[i] != m2[i]) {
                return false;
            }
        }

        return true;
    }

private:
    data_t* m_Data{nullptr};
    const MsgSize m_Size;
    std::default_random_engine m_Rng;
    std::uniform_int_distribution<int> m_Dist;
};
