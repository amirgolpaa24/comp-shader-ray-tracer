#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Error callback function
void errorCallback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

// Camera settings
float radius = 5.0f;      // Distance from the sphere center
float yaw = -90.0f;       // Horizontal angle
float pitch = 0.0f;       // Vertical angle
glm::vec3 cameraPos;      // Calculated camera position
glm::vec3 cameraDir;      // Calculated camera direction
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 sphereCenter = glm::vec3(0.0f, 0.0f, -5.0f); // Center of the sphere
glm::vec3 lightPos = glm::vec3(2.0f, 2.0f, -3.0f);

// Mouse callback function
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static double lastX = 400, lastY = 300;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to prevent flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

std::string loadShaderSource(const std::string& filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filePath);
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
    return shaderStream.str();
}

GLuint compileShader(const GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const GLchar* shaderSource = source.c_str();
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

int main() {
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Compute Shader - Orbiting Camera", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Make the cursor visible and register callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Setup texture
    GLuint texOutput;
    glGenTextures(1, &texOutput);
    glBindTexture(GL_TEXTURE_2D, texOutput);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindImageTexture(0, texOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Load and compile shaders
    GLuint computeShader = compileShader(GL_COMPUTE_SHADER, loadShaderSource("../shader.comp"));
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, loadShaderSource("../vertex_shader.glsl"));
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, loadShaderSource("../fragment_shader.glsl"));
    GLuint renderProgram = linkProgram(vertexShader, fragmentShader);

    // Setup full screen quad VAO and VBO
    GLuint quadVAO, quadVBO;
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Update camera position based on yaw, pitch, and radius
        cameraPos.x = sphereCenter.x + radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraPos.y = sphereCenter.y + radius * sin(glm::radians(pitch));
        cameraPos.z = sphereCenter.z + radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDir = glm::normalize(sphereCenter - cameraPos);

        // Pass updated camera position and direction to the compute shader
        GLuint cameraPosLocation = glGetUniformLocation(computeProgram, "cameraPos");
        GLuint cameraDirLocation = glGetUniformLocation(computeProgram, "cameraDir");
        GLuint cameraUpLocation = glGetUniformLocation(computeProgram, "cameraUp");
        GLuint lightPosLocation = glGetUniformLocation(computeProgram, "lightPos");
        GLuint screenSizeLocation = glGetUniformLocation(computeProgram, "screenSize");

        glUseProgram(computeProgram);
        glUniform3f(cameraPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(cameraDirLocation, cameraDir.x, cameraDir.y, cameraDir.z);
        glUniform3f(cameraUpLocation, cameraUp.x, cameraUp.y, cameraUp.z);
        glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);
        glUniform2i(screenSizeLocation, 800, 600);

        // Dispatch compute shader
        glDispatchCompute(50, 38, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Render the fullscreen quad
        glUseProgram(renderProgram);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, texOutput);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Swap buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(renderProgram);
    glDeleteProgram(computeProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}