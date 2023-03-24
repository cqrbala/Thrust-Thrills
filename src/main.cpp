#include "main.h"
#include "floor.h"
#include "ceiling.h"
#include "player.h"
#include "coin.h"
#include "zapper.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
bool coin_collision_detection(Player player, Coin curr_coin);
bool linerec(Player player, Zapper zapper);
bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
float alter_y();
float alter_y_zapper();
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

int score = 0;
int level = 1;
int result = 1;
int var = 15;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 transform;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "uniform vec4 greenCol;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = greenCol;\n"
                                   "}\n\0";

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;

Floor new_floor;
Ceiling new_ceiling;
Player new_player;
Coin coin1;
Coin coin2;
Coin coin3;
Coin coin4;
Coin coin5;
Coin coin6;
Coin coin7;
Coin coin8;
Coin coin9;

Zapper zapper1;
Zapper zapper2;
Zapper zapper3;
Zapper zapper4;
Zapper zapper5;
Zapper zapper6;

double vel = 0;
double pos = 0;

unsigned int VAO, VBO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // compile and setup the shader
    // ----------------------------
    Shader shader("../src/text.vs", "../src/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "../fonts/Valorax-lg25V.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else
    {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)};
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------------ starts now

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("../src/shader.vs", "../src/shader.fs");

    float vertices[] = {
        // positions          // colors           // texture coords
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f,   // top right
        1.0f, -1.2f, 0.0f, 0.0f, 1.0f,  // bottom right
        -1.0f, -1.2f, 0.0f, 1.0f, 1.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 1.0f, 0.0f   // top left
    };
    unsigned int indices[] = {
        0, 2, 1, // first triangle
        0, 3, 2  // second triangle
    };

    unsigned int T1_VBO, T1_VAO, T1_EBO;
    glGenVertexArrays(1, &T1_VAO);
    glGenBuffers(1, &T1_VBO);
    glGenBuffers(1, &T1_EBO);

    glBindVertexArray(T1_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, T1_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, T1_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load and create a texture
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load("../textures/calm.jpg", &width, &height, &nrChannels, STBI_rgb);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    unsigned int texture_2;

    glGenTextures(1, &texture_2);
    glBindTexture(GL_TEXTURE_2D, texture_2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *player_data = stbi_load("../textures/player.png", &width, &height, &nrChannels, STBI_rgb_alpha);
    if (player_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, player_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(player_data);

    new_floor.createVAO();
    new_ceiling.createVAO();
    new_player.createVAO();

    coin1.createVAO();
    coin2.createVAO();
    coin3.createVAO();

    zapper1.createVAO();
    zapper2.createVAO();
    zapper3.createVAO();

    double prev_frame = glfwGetTime();
    double acc = -9.8;
    int num = 0;

    // render loop
    // -----------
    if (level == 1)
    {
        double start_time = glfwGetTime();
        int first_time = 1;
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            processInput(window);

            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw our first triangle
            // glUseProgram(shaderProgram);
            new_floor.draw(shaderProgram);
            new_ceiling.draw(shaderProgram);

            // render container
            ourShader.use();
            ourShader.setInt("ourTexture", 0);
            // bind Texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(T1_VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            double present_frame = glfwGetTime();
            double dt = present_frame - prev_frame;
            vel += ((acc)*dt);
            pos += (vel * dt * 0.008);

            if (new_player.y > -0.69f)
            {
                new_player.y = new_player.y + pos;
                new_player.player_y = new_player.player_y + pos;
            }

            // render container
            ourShader.use();

            ourShader.setInt("ourTexture", 1);
            // bind Texture
            glBindTexture(GL_TEXTURE_2D, texture_2);
            glActiveTexture(GL_TEXTURE1);

            new_player.draw(shaderProgram);
            prev_frame = present_frame;

            if (first_time)
            {
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                coin1.x += 1.2f;
                coin1.coin_x += 1.2f;
            }
            coin1.x -= 0.008f;
            coin1.coin_x -= 0.008f;
            if (coin1.x < -1.0f)
            {
                coin1.x += 2.2f;
                coin1.coin_x += 2.2f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
            }
            coin1.draw(shaderProgram);

            if (first_time)
            {
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                coin2.x += 1.2f;
                coin2.coin_x += 1.2f;
            }
            coin2.x -= 0.008f;
            coin2.coin_x -= 0.008f;
            if (coin2.x < -1.0f)
            {
                coin2.x += 2.2f;
                coin2.coin_x += 2.2f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
            }
            coin2.draw(shaderProgram);

            if (first_time)
            {
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                coin3.x += 1.2f;
                coin3.coin_x += 1.2f;
            }
            coin3.x -= 0.008f;
            coin3.coin_x -= 0.008f;
            if (coin3.x < -1.0f)
            {
                coin3.x += 2.2f;
                coin3.coin_x += 2.2f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
            }
            coin3.draw(shaderProgram);

            if (coin_collision_detection(new_player, coin1))
            {
                coin1.x += 2.0f;
                coin1.coin_x += 2.0f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin2))
            {
                coin2.x += 2.0f;
                coin2.coin_x += 2.0f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin3))
            {
                coin3.x += 2.0f;
                coin3.coin_x += 2.0f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }

            zapper1.x -= 0.007f;
            zapper1.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }

            if (zapper1.zapper_x <= -1.3f)
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }
            if (linerec(new_player, zapper1))
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            zapper1.draw(shaderProgram);
            first_time = 0;
            string score_print;
            score_print = "Coins Collected: " + to_string(score);
            RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));

            string level_string = "Level: " + to_string(level);
            RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));

            double time_passed = glfwGetTime();
            string distance_travelled = "Distance travelled: " + to_string(int(time_passed - start_time));
            RenderText(shader, distance_travelled, 200.0f, 10.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));

            if ((int)(time_passed - start_time) == var)
            {
                level++;
                zapper1.x += 2.0f;
                zapper1.zapper_x += 2.0f;
                break;
            }

            // RenderText(shader, "Welcome to CG Course", 200.0f, 200.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    if (level == 2)
    {
        double start_time = glfwGetTime();
        int first_time = 1;
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            processInput(window);

            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw our first triangle
            // glUseProgram(shaderProgram);
            new_floor.draw(shaderProgram);
            new_ceiling.draw(shaderProgram);

            // render container
            ourShader.use();
            // ourShader.setInt("ourTexture", 0);
            //  bind Texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(T1_VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            double present_frame = glfwGetTime();
            double dt = present_frame - prev_frame;
            vel += ((acc)*dt);
            pos += (vel * dt * 0.008);

            if (new_player.y > -0.69f)
            {
                new_player.y = new_player.y + pos;
                new_player.player_y = new_player.player_y + pos;
            }

            new_player.draw(shaderProgram);
            prev_frame = present_frame;

            if (first_time)
            {
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                coin1.x += 1.2f;
                coin1.coin_x += 1.2f;
            }
            coin1.x -= 0.008f;
            coin1.coin_x -= 0.008f;
            if (coin1.x < -1.0f)
            {
                coin1.x += 2.2f;
                coin1.coin_x += 2.2f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
            }
            coin1.draw(shaderProgram);

            if (first_time)
            {
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                coin2.x += 1.2f;
                coin2.coin_x += 1.2f;
            }
            coin2.x -= 0.008f;
            coin2.coin_x -= 0.008f;
            if (coin2.x < -1.0f)
            {
                coin2.x += 2.2f;
                coin2.coin_x += 2.2f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
            }
            coin2.draw(shaderProgram);

            if (first_time)
            {
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                coin3.x += 1.2f;
                coin3.coin_x += 1.2f;
            }
            coin3.x -= 0.008f;
            coin3.coin_x -= 0.008f;
            if (coin3.x < -1.0f)
            {
                coin3.x += 2.2f;
                coin3.coin_x += 2.2f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
            }
            coin3.draw(shaderProgram);

            if (coin_collision_detection(new_player, coin1))
            {
                coin1.x += 2.0f;
                coin1.coin_x += 2.0f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin2))
            {
                coin2.x += 2.0f;
                coin2.coin_x += 2.0f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin3))
            {
                coin3.x += 2.0f;
                coin3.coin_x += 2.0f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }

            zapper1.x -= 0.007f;
            zapper1.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }

            zapper2.x -= 0.007f;
            zapper2.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper2.y = alter_y_zapper();
                zapper2.zapper_y = zapper2.y;
            }

            if (zapper1.zapper_x <= -1.3f)
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }
            if (zapper2.zapper_x <= -1.3f)
            {
                zapper2.x += 2.5f;
                zapper2.zapper_x += 2.5f;
                zapper2.y = alter_y_zapper();
                zapper2.zapper_y = zapper2.y;
            }
            if (linerec(new_player, zapper1))
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            if (linerec(new_player, zapper2))
            {
                zapper2.x += 2.5f;
                zapper2.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            zapper1.draw(shaderProgram);
            zapper2.draw(shaderProgram);
            first_time = 0;
            string score_print;
            score_print = "Coins Collected: " + to_string(score);
            RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
            string level_string = "Level: " + to_string(level);
            RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));

            double time_passed = glfwGetTime();
            string distance_travelled = "Distance travelled: " + to_string(int(time_passed - start_time));
            RenderText(shader, distance_travelled, 200.0f, 10.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));

            if ((int)(time_passed - start_time) == var + 5)
            {
                level++;
                zapper1.x += 2.0f;
                zapper1.zapper_x += 2.0f;
                zapper2.x += 2.0f;
                zapper2.zapper_x += 2.0f;
                break;
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    if (level == 3)
    {
        double start_time = glfwGetTime();
        int first_time = 1;
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            processInput(window);

            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw our first triangle
            // glUseProgram(shaderProgram);
            new_floor.draw(shaderProgram);
            new_ceiling.draw(shaderProgram);

            // render container
            ourShader.use();
            ourShader.setInt("ourTexture", 0);
            // bind Texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(T1_VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            double present_frame = glfwGetTime();
            double dt = present_frame - prev_frame;
            vel += ((acc)*dt);
            pos += (vel * dt * 0.008);

            if (new_player.y > -0.69f)
            {
                new_player.y = new_player.y + pos;
                new_player.player_y = new_player.player_y + pos;
            }

            new_player.draw(shaderProgram);
            prev_frame = present_frame;

            if (first_time)
            {
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                coin1.x += 1.2f;
                coin1.coin_x += 1.2f;
            }
            coin1.x -= 0.008f;
            coin1.coin_x -= 0.008f;
            if (coin1.x < -1.0f)
            {
                coin1.x += 2.2f;
                coin1.coin_x += 2.2f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
            }
            coin1.draw(shaderProgram);

            if (first_time)
            {
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                coin2.x += 1.2f;
                coin2.coin_x += 1.2f;
            }
            coin2.x -= 0.008f;
            coin2.coin_x -= 0.008f;
            if (coin2.x < -1.0f)
            {
                coin2.x += 2.2f;
                coin2.coin_x += 2.2f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
            }
            coin2.draw(shaderProgram);

            if (first_time)
            {
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                coin3.x += 1.2f;
                coin3.coin_x += 1.2f;
            }
            coin3.x -= 0.008f;
            coin3.coin_x -= 0.008f;
            if (coin3.x < -1.0f)
            {
                coin3.x += 2.2f;
                coin3.coin_x += 2.2f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
            }
            coin3.draw(shaderProgram);

            if (coin_collision_detection(new_player, coin1))
            {
                coin1.x += 2.0f;
                coin1.coin_x += 2.0f;
                coin1.y = alter_y();
                coin1.coin_y = coin1.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin2))
            {
                coin2.x += 2.0f;
                coin2.coin_x += 2.0f;
                coin2.y = alter_y();
                coin2.coin_y = coin2.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }
            if (coin_collision_detection(new_player, coin3))
            {
                coin3.x += 2.0f;
                coin3.coin_x += 2.0f;
                coin3.y = alter_y();
                coin3.coin_y = coin3.y;
                score++;
                string score_print;
                score_print = "Coins Collected: " + to_string(score);
                RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
                string level_string = "Level: " + to_string(level);
                RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));
            }

            zapper1.x -= 0.007f;
            zapper1.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }

            zapper2.x -= 0.007f;
            zapper2.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper2.y = alter_y_zapper();
                zapper2.zapper_y = zapper2.y;
            }

            zapper3.x -= 0.007f;
            zapper3.zapper_x -= 0.007f;
            if (first_time)
            {
                zapper3.y = alter_y_zapper();
                zapper3.zapper_y = zapper3.y;
            }

            if (zapper1.zapper_x <= -1.3f)
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                zapper1.y = alter_y_zapper();
                zapper1.zapper_y = zapper1.y;
            }
            if (zapper2.zapper_x <= -1.3f)
            {
                zapper2.x += 2.5f;
                zapper2.zapper_x += 2.5f;
                zapper2.y = alter_y_zapper();
                zapper2.zapper_y = zapper2.y;
            }
            if (zapper3.zapper_x <= -1.3f)
            {
                zapper3.x += 2.5f;
                zapper3.zapper_x += 2.5f;
                zapper3.y = alter_y_zapper();
                zapper3.zapper_y = zapper3.y;
            }
            if (linerec(new_player, zapper1))
            {
                zapper1.x += 2.5f;
                zapper1.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            if (linerec(new_player, zapper2))
            {
                zapper2.x += 2.5f;
                zapper2.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            if (linerec(new_player, zapper3))
            {
                zapper3.x += 2.5f;
                zapper3.zapper_x += 2.5f;
                level = 4;
                result = 0;
                break;
            }
            zapper1.draw(shaderProgram);
            zapper2.draw(shaderProgram);
            zapper3.draw(shaderProgram);
            first_time = 0;
            string score_print;
            score_print = "Coins Collected: " + to_string(score);
            RenderText(shader, score_print, 50.0f, 575.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));
            string level_string = "Level: " + to_string(level);
            RenderText(shader, level_string, 550.0f, 575.0f, 0.5f, glm::vec3(0.4, 0.4f, 0.4f));

            double time_passed = glfwGetTime();
            string distance_travelled = "Distance travelled: " + to_string(int(time_passed - start_time));
            RenderText(shader, distance_travelled, 200.0f, 10.0f, 0.5f, glm::vec3(0.0, 0.0f, 0.0f));

            if ((int)(time_passed - start_time) == var + 10)
            {
                level++;
                break;
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    if (level == 4)
    {
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            processInput(window);

            // render
            // ------
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw our first triangle
            // glUseProgram(shaderProgram);

            RenderText(shader, "Game over", 200.0f, 400.0f, 1.0f, glm::vec3(0.8, 0.8f, 0.8f));
            if (!result)
            {
                RenderText(shader, "L", 350.0f, 300.0f, 2.0f, glm::vec3(0.8, 0.8f, 0.8f));
                string score_text = "Final Score: " + to_string(score);
                RenderText(shader, score_text, 260.0f, 250.0f, 0.5f, glm::vec3(0.8, 0.8f, 0.8f));
                RenderText(shader, "What colour is grass ?", 200.0f, 200.0f, 0.5f, glm::vec3(0.8, 0.8f, 0.8f));
            }
            else
            {
                RenderText(shader, "W", 300.0f, 300.0f, 2.0f, glm::vec3(0.8, 0.8f, 0.8f));
                string score_text = "Final Score: " + to_string(score);
                RenderText(shader, score_text, 250.0f, 250.0f, 0.5f, glm::vec3(0.8, 0.8f, 0.8f));
                // RenderText(shader, "Noice", 285.0f, 200.0f, 0.75f, glm::vec3(0.8, 0.8f, 0.8f));
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (new_player.y > 0.89f)
        {
        }
        else
        {
            new_player.y += 0.0109f;
            new_player.player_y += 0.0109f;
            vel = 0;
            pos = 0;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

bool coin_collision_detection(Player player, Coin curr_coin)
{
    return abs(player.player_x - curr_coin.coin_x) * 2 < (player.player_width + (2 * curr_coin.radius)) &&
           (abs(player.player_y - curr_coin.coin_y) * 2 < (player.player_height + (2 * curr_coin.radius)));
}

bool linerec(Player player, Zapper zapper)
{
    float y1 = zapper.zapper_y + (0.07f * sin(glm::radians(zapper.rotation_angle)));
    float x1 = zapper.zapper_x + (0.07f * cos(glm::radians(zapper.rotation_angle)));
    float y2 = zapper.zapper_y - (0.07f * sin(glm::radians(zapper.rotation_angle)));
    float x2 = zapper.zapper_x - (0.07f * cos(glm::radians(zapper.rotation_angle)));

    float left = player.player_x - 0.125f;
    float right = player.player_x + 0.125f;
    float top = player.player_y + 0.2f;
    float bottom = player.player_y - 0.2f;

    bool left_line = lineLine(x1, y1, x2, y2, left, top, left, bottom);
    bool right_line = lineLine(x1, y1, x2, y2, right, top, right, bottom);
    bool top_line = lineLine(x1, y1, x2, y2, left, top, right, top);
    bool bottom_line = lineLine(x1, y1, x2, y2, left, bottom, right, bottom);

    if (left_line || right_line || top_line || bottom_line)
    {
        return true;
    }
    return false;
}

bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    float uA = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
    float uB = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1)
    {
        return true;
    }
    return false;
}

float alter_y()
{
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<> distr(-0.8, 0.8);
    return distr(gen);
}

float alter_y_zapper()
{
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<> distr(-0.6, 0.6);
    return distr(gen);
}
// render line of text
// -------------------
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}