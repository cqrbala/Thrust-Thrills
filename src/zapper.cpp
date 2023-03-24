#include "zapper.h"

void Zapper::createVAO()
{
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    float vertices[] = {
        0.7f, 0.15f, 0.0f,
        0.75f, 0.15f, 0.0f,
        0.7f, -0.15f, 0.0f,
        0.75f, -0.15f, 0.0f};

    unsigned int ebo_indices[] = {
        0, 1, 2,
        1, 2, 3};

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // init the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // init the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebo_indices), ebo_indices, GL_STATIC_DRAW);

    // declare attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Zapper::draw(unsigned int shaderProgram)
{
    glUseProgram(shaderProgram);
    // transformation_matrix = glm::translate(transformation_matrix, glm::vec3(this->x, this->y, this->z));

    rotation_angle = i++ * 2.0f;

    transformation_matrix = glm::translate(transformation_matrix, glm::vec3(zapper_x, zapper_y, 0));
    transformation_matrix = glm::rotate(transformation_matrix, glm::radians(rotation_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transformation_matrix = glm::translate(transformation_matrix, glm::vec3(-zapper_x, -zapper_y, 0));

    transformation_matrix = glm::translate(transformation_matrix, glm::vec3(this->x, this->y, this->z));

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformation_matrix));
    transformation_matrix = glm::mat4(1.0f);

    int vertexColorLocation = glGetUniformLocation(shaderProgram, "greenCol");
    glUniform4f(vertexColorLocation, 0.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}