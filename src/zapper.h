#include "main.h"

#ifndef ZAPPER_H
#define ZAPPER_H

class Zapper
{
public:
    int i = 1;
    float rotation_angle = 0;
    float zapper_x = 0.725f, zapper_y = 0.0f;
    unsigned int VAO;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    glm::mat4 transformation_matrix;
    Zapper()
    {
        transformation_matrix = glm::mat4(1.0f);
    };
    void createVAO();
    void draw(unsigned int shaderProgram);
};

#endif