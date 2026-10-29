#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

inline std::string readTextFile(std::filesystem::path const& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("Unable to open file: " + path.string());
    }
    return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
}
