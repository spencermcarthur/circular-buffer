#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>

#include "Macros.hpp"

class WaitTimeGenerator {
    static constexpr uint64_t NANOS_PER_MILLI = 1'000'000;

public:
    explicit WaitTimeGenerator(double meanArrivalRateMillis)
        : m_Dist(1.0 / meanArrivalRateMillis) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
    }

    EXPLICIT_DELETE_CONSTRUCTORS(WaitTimeGenerator);

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

class MessageSizeGenerator {
public:
    explicit MessageSizeGenerator(double meanSize) : m_Dist(1.0 / meanSize) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
    }

    EXPLICIT_DELETE_COPY_MOVE(MessageSizeGenerator);
    uint16_t operator()() { return Clip(m_Dist(m_Rng)); }

private:
    static uint16_t Clip(double value) {
        static constexpr uint16_t min = 1;
        static constexpr uint16_t max = std::numeric_limits<uint16_t>::max();

        uint16_t ret;
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
    std::exponential_distribution<double> m_Dist;
};

class RandomMessage {
    using data_t = std::byte;
    static constexpr uint8_t MIN_BYTE_VAL = 0;
    static constexpr uint8_t MAX_BYTE_VAL = 255;

public:
    explicit RandomMessage(uint16_t size) : m_Size(size) {
        m_Rng.seed(std::chrono::high_resolution_clock::now()
                       .time_since_epoch()
                       .count());
        m_Data = new data_t[m_Size]{};

        for (uint16_t i = 0; i < m_Size; i++) {
            m_Data[i] = data_t(m_Dist(m_Rng));
        }
    }

    ~RandomMessage() { delete[] m_Data; }

    EXPLICIT_DELETE_CONSTRUCTORS(RandomMessage);

    [[nodiscard]] data_t* data() const { return m_Data; }
    [[nodiscard]] uint16_t size() const { return m_Size; }
    [[nodiscard]] std::span<data_t> buf() const { return {m_Data, m_Size}; }

    friend bool operator==(const RandomMessage& a, const RandomMessage& b) {
        auto m1 = a.buf();
        auto m2 = b.buf();

        if (m1.size() != m2.size()) {
            return false;
        }

        for (size_t i = 0; i < a.size(); i++) {
            if (m1[i] != m2[i]) {
                return false;
            }
        }

        return true;
    }

private:
    data_t* m_Data{nullptr};
    const size_t m_Size;
    std::default_random_engine m_Rng;
    std::uniform_int_distribution<int> m_Dist;
};
