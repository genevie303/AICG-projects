#pragma once

#include <cstdint>

struct Primitive {
    enum class Type : int { Sphere = 0, Triangle = 1 };

    Type type{Type::Sphere};
    uint32_t shapeIndex{0};
    uint32_t materialIndex{0};
};
