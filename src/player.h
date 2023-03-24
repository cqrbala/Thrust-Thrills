#include "main.h"

#ifndef PLAYER_H
#define PLAYER_H

class Player
{
public:
    unsigned int VAO;
    float player_x = -0.825f, player_y = -0.1f;
    float player_width = 0.25f, player_height = 0.2f;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    glm::mat4 transformation_matrix;
    Player()
    {
        transformation_matrix = glm::mat4(1.0f);
    };
    void createVAO();
    void draw(unsigned int shaderProgram);
};

#endif