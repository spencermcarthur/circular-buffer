#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include "circularbuffer/Spec.hpp"

static CircularBuffer::Spec LoadSpec(int argc, char* argv[]) {
    static auto Load = [](const std::filesystem::path& path,
                          CircularBuffer::Spec& spec) {
        std::ifstream fin(path);
        std::string tmp;

        std::getline(fin, spec.indexSharedMemoryName);
        std::getline(fin, spec.dataSharedMemoryName);
        std::getline(fin, tmp);
        spec.bufferCapacity = std::stoul(tmp);
    };

    CircularBuffer::Spec spec;
    std::filesystem::path specFile{std::filesystem::absolute(
        std::filesystem::path(argv[0]).parent_path() / "bufferconfig.txt")};

    if (std::filesystem::is_regular_file(specFile)) {
        Load(specFile, spec);
    } else if (argc != 2) {
        throw std::runtime_error(std::format("Usage: {} <config>\n", argv[0]));
    } else {
        try {
            Load(argv[1], spec);
        } catch (std::exception& e) {
            throw std::runtime_error(
                std::format("Failed to load spec file {}", argv[1]));
        }
    }

    return spec;
}
