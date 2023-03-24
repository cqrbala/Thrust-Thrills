#include "main.h"

#ifndef FLOOR_H
#define FLOOR_H

class Floor
{
public:
    unsigned int VAO;
    glm::mat4 transformation_matrix;
    Floor() { transformation_matrix = glm::mat4(1.0f); };
    void createVAO();
    void draw(unsigned int shaderProgram);
};

#endif
