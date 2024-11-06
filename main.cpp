#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include "dependente/glew/glew.h"

// Include GLFW
#include "dependente/glfw/glfw3.h"

// Include GLM
#include "dependente/glm/glm.hpp"
#include "dependente/glm/gtc/matrix_transform.hpp"
#include "dependente/glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "dependente/stb_image.h"

#include "shader.hpp"

// Variables
GLFWwindow* window;
const int width = 1024, height = 1024;

GLuint backgroundTextures[2];
int currentBackgroundIndex = 0;
glm::vec3 squarePosition = glm::vec3(-0.55f, -0.5f, 0.0f); // Starting position of the movable square (moved to the left)
float squareSpeed = 0.01f;
glm::vec3 housePosition = glm::vec3(-0.6f, -0.5f, 0.0f); // Move the house closer to the center

// Function to load textures using stb_image
GLuint loadTexture(const char* filepath) {
    stbi_set_flip_vertically_on_load(true); // Flip texture vertically on load
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture" << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return textureID;
}

// Key callback function to handle movement
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_A) {
            squarePosition.x -= squareSpeed;
        }
        else if (key == GLFW_KEY_D) {
            squarePosition.x += squareSpeed;
        }
    }
}

// Render the hardcoded background
void renderBackground(GLuint programID) {
    glUseProgram(programID);
    glUniform1i(glGetUniformLocation(programID, "isBackground"), 1);

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };

    glBindTexture(GL_TEXTURE_2D, backgroundTextures[currentBackgroundIndex]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &quadVertices[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &quadVertices[2]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, quadIndices);

    glUniform1i(glGetUniformLocation(programID, "isBackground"), 0);
}

// Render the hardcoded square
void renderSquare(GLuint programID) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), squarePosition);
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.5f, 0.0f, 0.5f, 1.0f);

    float squareVertices[] = {
        -0.05f,  0.05f, 0.0f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.0f,
         0.05f, -0.05f, 0.0f, 0.0f,
         0.05f,  0.05f, 0.0f, 0.0f
    };
    unsigned int squareIndices[] = { 0, 1, 2, 2, 3, 0 };

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &squareVertices[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, squareIndices);
}

// Render the hardcoded house
void renderHouse(GLuint programID) {
    // House body (square)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), housePosition);
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.31f, 0.27f, 0.31f, 1.0f); // Dark purple color for house body (#4F454F)

    float houseBodyVertices[] = {
        -0.2f,  0.3f, 0.0f, 0.0f, // top-left
        -0.2f, -0.05f, 0.0f, 0.0f, // bottom-left
         0.2f, -0.05f, 0.0f, 0.0f, // bottom-right
         0.2f,  0.3f, 0.0f, 0.0f   // top-right
    };
    unsigned int houseBodyIndices[] = { 0, 1, 2, 2, 3, 0 };

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &houseBodyVertices[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, houseBodyIndices);

    // Roof (triangle)
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(0.0f, 0.05f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.17f, 0.16f, 0.2f, 1.0f); // Dark gray color for roof (#2C2A34)

    float roofVertices[] = {
        -0.2f, 0.25f, 0.0f, 0.0f, // left
         0.2f, 0.25f, 0.0f, 0.0f, // right
         0.0f,  0.5f, 0.0f, 0.0f  // top
    };
    unsigned int roofIndices[] = { 0, 1, 2 };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &roofVertices[0]);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, roofIndices);

    // Door (smaller rectangle)
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(0.0f, -0.1f, 0.0f)); // Position the door near the bottom center
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.17f, 0.16f, 0.2f, 1.0f); // Darker brown color for the door

    float doorVertices[] = {
        -0.06f, 0.2f, 0.0f, 0.0f, // top-left of door
        -0.06f, 0.05f, 0.0f, 0.0f, // bottom-left of door
         0.06f, 0.05f, 0.0f, 0.0f, // bottom-right of door
         0.06f, 0.2f, 0.0f, 0.0f   // top-right of door
    };
    unsigned int doorIndices[] = { 0, 1, 2, 2, 3, 0 };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &doorVertices[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, doorIndices);

    // Left Window (small square)
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(-0.1f, 0.1f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.91f, 0.3f, 0.21f, 1.0f); // Red color for window (#E84C35)
    float leftWindowVertices[] = {
        -0.04f, 0.125f, 0.0f, 0.0f, // top-left of window
        -0.04f, 0.05f, 0.0f, 0.0f, // bottom-left of window
         0.04f, 0.05f, 0.0f, 0.0f, // bottom-right of window
         0.04f, 0.125f, 0.0f, 0.0f   // top-right of window
    };
    unsigned int windowIndices[] = { 0, 1, 2, 2, 3, 0 };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &leftWindowVertices[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);

    // Right Window (small square)
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(0.1f, 0.1f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(glGetUniformLocation(programID, "color"), 0.91f, 0.3f, 0.21f, 1.0f); // Red color for window (#E84C35)

    float rightWindowVertices[] = {
        -0.04f, 0.125f, 0.0f, 0.0f, // top-left of window
        -0.04f, 0.05f, 0.0f, 0.0f, // bottom-left of window
         0.04f, 0.05f, 0.0f, 0.0f, // bottom-right of window
         0.04f, 0.125f, 0.0f, 0.0f   // top-right of window
    };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &rightWindowVertices[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);

    // Draw Crosses on Windows
    glUniform4f(glGetUniformLocation(programID, "color"), 0.17f, 0.16f, 0.2f, 1.0f); // Black color for cross

    // Left Window Cross
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(-0.1f, 0.1f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));

    float verticalBar[] = {
        -0.005f, 0.125f, 0.0f, 0.0f, // top
        -0.005f, 0.05f, 0.0f, 0.0f,  // bottom
         0.005f, 0.05f, 0.0f, 0.0f,  // bottom
         0.005f, 0.125f, 0.0f, 0.0f  // top
    };
    float horizontalBar[] = {
        -0.04f, 0.095f, 0.0f, 0.0f, // left
         0.04f, 0.095f, 0.0f, 0.0f, // right
         0.04f, 0.085f, 0.0f, 0.0f, // right
        -0.04f, 0.085f, 0.0f, 0.0f  // left
    };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &verticalBar[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &horizontalBar[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);

    // Right Window Cross
    model = glm::translate(glm::mat4(1.0f), housePosition + glm::vec3(0.1f, 0.1f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(programID, "transform"), 1, GL_FALSE, glm::value_ptr(model));

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &verticalBar[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), &horizontalBar[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, windowIndices);
}

int main(void) {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(width, height, "Lab 4", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    // Specify the size of the rendering window
    glViewport(0, 0, width, height);

    // Load background textures
    backgroundTextures[0] = loadTexture("M:/DOCUMENTS_2.0/poli_stuff/y3_s1/acg/ACG_DEMO_2D/ACG_DEMO_2D/Lab4/images/house3.jpg");
    backgroundTextures[1] = loadTexture("M:/DOCUMENTS_2.0/poli_stuff/y3_s1/acg/ACG_DEMO_2D/ACG_DEMO_2D/Lab4/images/img.jpg");

    // Load shader program
    GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

    // Set the key callback
    glfwSetKeyCallback(window, key_callback);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Check square position to switch backgrounds and toggle house visibility
        if (squarePosition.x > 0.6f) {
            currentBackgroundIndex = 1; // Switch to the second background
        }
        else if (squarePosition.x < -0.6f) {
            currentBackgroundIndex = 0; // Switch back to the first background
        }

        // Render background
        renderBackground(programID);

        // Render house only if the first background is active
        if (currentBackgroundIndex == 0) {
            renderHouse(programID);
        }

        // Render the movable square
        renderSquare(programID);

        // Swap buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(2, backgroundTextures);
    glDeleteProgram(programID);
    glfwTerminate();

    return 0;
}
