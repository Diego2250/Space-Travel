#include "glm/glm.hpp"
#include <vector>
#include "uniform.h"

enum ShaderType {
    ROCOSO,
    GASEOSO,
    ESTRELLA,
    LUNA,
    VOLCANICO,
    CRISTAL,
    HIELO
};

class Model {
public:
    glm::mat4 modelMatrix;
    std::vector<glm::vec3> vertices;
    Uniform uniforms;
    ShaderType currentShader;
};