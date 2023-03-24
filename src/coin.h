#include "main.h"

#ifndef COIN_H
#define COIN_H

class Coin
{
public:
    float coin_x = 0.0f, coin_y = 0.0f;
    float radius = 0.07f;
    float x = 0, y = 0, z = 0;
    unsigned int VAO;
    glm::mat4 transformation_matrix;
    Coin() { transformation_matrix = glm::mat4(1.0f); };
    void createVAO();
    void draw(unsigned int shaderProgram);
};

#endif