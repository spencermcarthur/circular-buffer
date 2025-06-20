#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include "circularbuffer/Spec.hpp"

static CircularBuffer::Spec LoadSpec(std::string_view exeName) {
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
        std::filesystem::path(exeName).parent_path() / "bufferconfig.txt")};

    if (std::filesystem::is_regular_file(specFile)) {
        Load(specFile, spec);
    } else {
        throw std::runtime_error(
            std::format("Failed to load spec file {}", specFile.string()));
    }

    return spec;
}
