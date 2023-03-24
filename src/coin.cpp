#include "coin.h"

void Coin::createVAO()
{
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    int n = 200;

    float vertices[9 * n];
    for (int i = 0; i < n; ++i)
    {
        vertices[9 * i] = (0.07 * cos(i * 2 * 3.14159 / n));
        vertices[9 * i + 1] = (0.07 * sin(i * 2 * 3.14159 / n));
        vertices[9 * i + 2] = 0.0;

        vertices[9 * i + 3] = (0.07 * cos((i + 1) * 2 * 3.14159 / n));
        vertices[9 * i + 4] = (0.07 * sin((i + 1) * 2 * 3.14159 / n));
        vertices[9 * i + 5] = 0.0;

        vertices[9 * i + 6] = 0.0;
        vertices[9 * i + 7] = 0.0;
        vertices[9 * i + 8] = 0.0;
    }

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // init the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // init the EBO
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebo_indices), ebo_indices, GL_STATIC_DRAW);

    // declare attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}

void Coin::draw(unsigned int shaderProgram)
{
    glUseProgram(shaderProgram);
    transformation_matrix = glm::mat4(1.0f);

    transformation_matrix = glm::translate(transformation_matrix, glm::vec3(this->x, this->y, this->z));

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformation_matrix));

    int vertexColorLocation = glGetUniformLocation(shaderProgram, "greenCol");
    glUniform4f(vertexColorLocation, 0.0f, 1.0f, 0.0f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 600);
    glBindVertexArray(0);
}
