#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "stdlib.h"
#include <iostream>
#include <cstdint>
#include <filesystem>

#include "./headers/shader.hpp"
#include "./headers/model.hpp"

struct Joystick {
    float leftX;
    float leftY;
    float L2;
    float rightX;
    float rightY;
    float R2;
};

// Variables
size_t WINDOW_WIDTH = 1280;
size_t WINDOW_HEIGHT = 720;

float lastMouseX = WINDOW_WIDTH / 2;
float lastMouseY = WINDOW_HEIGHT / 2;

float pitch = 0.0f;
float yaw = -90.0f;

bool joystickPresent = false;
bool firstMouse = true;

float fov = 45.0f;

Joystick joystick = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

unsigned int vboId, vaoId, eboId;

float vertices[] = {
    // Back
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.0f,  0.5f, -0.5f,  1.0f, 1.0f,

     0.0f,  0.5f, -0.5f,  0.0f, 0.0f,
     1.0f,  0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  1.5f, -0.5f,  1.0f, 1.0f,

     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     1.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     1.0f,  0.5f, -0.5f,  1.0f, 1.0f,

    // Front
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.0f,  0.5f, 0.0f,  1.0f, 1.0f,

     0.0f,  0.5f, 0.0f,  0.0f, 0.0f,
     1.0f,  0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  1.5f, 0.0f,  1.0f, 1.0f,

     0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     1.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     1.0f,  0.5f, 0.0f,  1.0f, 1.0f,
};

int indices[] = {
    // back

    // left, right, top
    0, 1, 2, // left
    3, 4, 5, // top
    6, 7, 8, // right

    // front
    // left, right, top
    9, 10, 11, // left
    12, 13, 14, // top
    15, 16, 17, // right

    // left wall
    5, 14, 0,
    0, 9, 14,
    // right wall
    5, 14, 7,
    7, 16, 14,
    // floor
    0, 9, 7,
    7, 9, 16,
    // inner left wall
    1, 3, 11,
    1, 10, 11,
    // inner right wall
    6, 8, 15,
    8, 15, 17,
    // ceiling wall
    3, 4, 12,
    4, 12, 13
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f)
};

glm::vec3 translations[100];

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

unsigned int TOTAL_VERTICES = 54;

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void joystick_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
void storeVertexDataOnGpu();
void draw(Shader& shader, Model& planetModel, Model& asteroidModel);

int main() 
{
    std::cout << "Hello, Cosmos!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Application", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

	// Open Window
    glfwMakeContextCurrent(window);

	// Load openGL functions for this specific openGL Implementation via GLAD
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}  

	// Viewport dictates how we want to display the data and coordinates with respect to the window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    /* 
        Tell GLFW that it should hide the cursor but still capture it. 
        Capturing a cursor means that, once the application has focus, the mouse cursor stays within the center of the window.
        Wherever we move the mouse it won't be visible and it should not leave the window. This is perfect for an FPS camera system.
    */
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* 
       Enable depth buffer
       The depth is stored within each fragment (as the fragment's z value) and whenever the fragment wants to output its color, 
       OpenGL compares its depth values with the z-buffer. If the current fragment is behind the other fragment it is discarded, 
       otherwise overwritten. This process is called depth testing and is done automatically by OpenGL.
    */
    glEnable(GL_DEPTH_TEST);

    // Register mouse callback - Each time mouse moves this will be called with the (x,y) coords of the mouse.
    glfwSetCursorPosCallback(window, mouse_callback); 

    // Scroll callback (change fov of perspective project based on y coordinate)
    glfwSetScrollCallback(window, scroll_callback); 

	render(window);

	glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
    // build intancing offset array
    int index = 0;
    float offset = 0.1f;
    for (int y = -200; y < 200; y += 40)
    {
        for (int x = -200; x < 200; x += 40)
        {
            glm::vec3 object;
            object.x = (float)x / 10.0f + offset;
            object.y = (float)y / 10.0f + offset;
            translations[index] = object;
            index++;
        }
    }

	storeVertexDataOnGpu();

    Shader ourShader("src/examples/instancing/advanced/asteroid_field/data/shaders/shader.vs", "src/examples/instancing/advanced/asteroid_field/data/shaders/shader.fs");

    char* planetModelPath = "src/examples/instancing/advanced/asteroid_field/data/planet/planet.obj";
    Model planetModel(planetModelPath);

    char* asteroidPlanetPath = "src/examples/instancing/advanced/asteroid_field/data/asteroid/rock.obj";
    Model asteroidModel(asteroidPlanetPath);

	while(!glfwWindowShouldClose(window))
	{
        joystickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);

        if (joystickPresent)
        {
            int axesCount;
            const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
            joystick = {axes[0], axes[1], axes[2], axes[3], axes[4], axes[5]};
        }

		// Input
		processInput(window);

		// Rendering commands
		draw(ourShader, planetModel, asteroidModel);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();    
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || joystick.leftY < -0.3)
    {
        const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
        cameraPos += (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || joystick.leftY > 0.3)
    {
        const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
        cameraPos -= (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || joystick.leftX < -0.3)
    {
        const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
        cameraPos -= (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || joystick.leftX > 0.3)
    {
        const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
        cameraPos += (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }

    if (joystickPresent)
    {
        if (joystick.R2 > -0.3)
        {
            fov -= (joystick.R2 + 1);
        }
        if (joystick.L2 > -0.3)
        {
            fov += (joystick.L2);
        }
    }

    if (fov < 1.0f)
    {
        fov = 1.0f;
    }
    if (fov > 45.0f)
    {
        fov = 45.0f; 
    }

    joystick_callback(window, joystick.rightX, joystick.rightY);
}

/*
    1. Calculate the mouse's offset since the last frame.
    2. Add the offset values to the camera's yaw and pitch values.
    3. Add some constraints to the minimum/maximum pitch values.
    4. Calculate the direction vector.
*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos; // reversed since y-coordinates range from bottom to top
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
    {
        pitch =  89.0f;
    }
    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void joystick_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (xpos > 0.3 || xpos < -0.3 || ypos > 0.3 || ypos < -0.3)
    {
        lastMouseX -= xpos;
        lastMouseY += ypos;
        
        const float sensitivity = 3.0f;
        float xoffset = xpos * sensitivity;
        float yoffset = ypos * sensitivity;

        yaw   += xoffset;
        pitch -= yoffset;

        if (pitch > 89.0f)
        {
            pitch =  89.0f;
        }
        if (pitch < -89.0f)
        {
            pitch = -89.0f;
        }

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
}

void draw(Shader& ourShader, Model& planetModel, Model& asteroidModel)
{
	// Clear the screen with a colour
	// glClearColor(0.0f, 0.0f, 0.5f, 0.2f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
    ourShader.use();

    // 3d
    glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    float radius = 30.0f;
    float camX = static_cast<float>(sin(glfwGetTime()) * radius);
    float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

    view = glm::lookAt(cameraPos, // Camera Pos
                       cameraPos + cameraFront, // Target Pos
                       cameraUp); // Up Vector
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -8.0f));
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

	glBindVertexArray(vaoId);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[0]);

    float angle = 10.0f * (1.5f); 
    model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    ourShader.setMat4("model", model);
    planetModel.Draw(ourShader);

    angle = 30.0f * (1.5f); 
    model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[0]);
    model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));
    ourShader.setMat4("model", model);
    asteroidModel.Draw(ourShader);

    glDrawElementsInstanced(GL_TRIANGLES, TOTAL_VERTICES, GL_UNSIGNED_INT, 0, 100);
    glBindVertexArray(0); 
}

void storeVertexDataOnGpu()
{
    uint32_t totalInstances = sizeof(translations) / sizeof(glm::vec3);
    std::cout << totalInstances << std::endl;

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * totalInstances, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &eboId);
	glBindVertexArray(vaoId);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttribDivisor(2, 1);
}

