#include "main.h"

#ifndef CEILING_H
#define CEILING_H

class Ceiling
{
public:
    unsigned int VAO;
    glm::mat4 transformation_matrix;
    Ceiling() { transformation_matrix = glm::mat4(1.0f); };
    void createVAO();
    void draw(unsigned int shaderProgram);
};

#endif
