#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "stdlib.h"
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "./headers/shader.hpp"
#include "./headers/model.hpp"

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void joystick_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
void storeVertexDataOnGpu();
void draw(Shader& shader);
void buildPositionData();

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
bool imGuiMode = false;

float fov = 45.0f;

Joystick joystick = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

unsigned int vaoId;
unsigned int vboId, eboId;

glm::vec3 cameraPos   = glm::vec3(0.0f, 1.0f,  2.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
unsigned int polygonMode = 0;

int TOTAL_VERTICES_PER_TILE = 6;
int TOTAL_TILES = 1;
int MAX_TOTAL_TILES = 500;

std::vector<glm::vec3> cubePositions;

float vertices[] = {
    -0.5f, 0.0f, 0.0f,
    -0.5f, 0.0f, 1.0f,
    0.5f,  0.0f, 0.0f,
    0.5f,  0.0f, 1.0f,
};

int indices[] = {
    0, 1, 2,
    1, 3, 2
};

int main() 
{
    std::cout << "Hello, Plane!" << std::endl;

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
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    // Register mouse callback - Each time mouse moves this will be called with the (x,y) coords of the mouse.
    glfwSetCursorPosCallback(window, mouse_callback); 

    // Scroll callback (change fov of perspective project based on y coordinate)
    glfwSetScrollCallback(window, scroll_callback); 

    // Default polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	render(window);

    // Cleanup
	glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &eboId);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
    Shader shader("src/examples/terrain/data/shaders/shader.vs", "src/examples/terrain/data/shaders/shader.fs");

    bool show_window = true;
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

	storeVertexDataOnGpu();
    buildPositionData();

	while(!glfwWindowShouldClose(window))
	{
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        joystickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);

        if (joystickPresent)
        {
            int axesCount;
            const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
            joystick = {axes[0], axes[1], axes[2], axes[3], axes[4], axes[5]};
        }

		// Input
		processInput(window);

        // ImGui Windows
        if (show_window)
        {
            // Imgui
            ImGui::Begin("My Window", &show_window);   

            // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from ImGuI!");
            ImGui::SliderInt("Total vertices", &TOTAL_TILES, 0, MAX_TOTAL_TILES);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Confirm"))
            {
	            storeVertexDataOnGpu();
                buildPositionData();
            }

            if (ImGui::Button("Close"))
            {
                show_window = false;
            }
            ImGui::End();
        }

        ImGui::Render();

        // Clear the screen with a colour
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		// Rendering commands
		draw(shader);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();    
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if (imGuiMode) 
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } 
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        imGuiMode = !imGuiMode;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        if (polygonMode == 0)
        {
            std::cout << "lines" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            polygonMode = 1;
        }
        else if (polygonMode == 1)
        {
            std::cout << "points" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
            polygonMode = 2;
        }
        else 
        {
            std::cout << "fill" << std::endl;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            polygonMode = 0;
        }
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
    {
		glfwSetWindowShouldClose(window, true);
	}

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    const float cameraSpeed = 10.0f * deltaTime; // adjust accordingly
                                                //
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || joystick.leftY < -0.3)
    {
        cameraPos += (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || joystick.leftY > 0.3)
    {
        cameraPos -= (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || joystick.leftX < -0.3)
    {
        cameraPos -= (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || joystick.leftX > 0.3)
    {
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
    if (imGuiMode) 
    {
        return;
    }
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

void draw(Shader& shader)
{
    shader.use();

    glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    view = glm::lookAt(cameraPos, // Camera Pos
                       cameraPos + cameraFront, // Target Pos
                       cameraUp); // Up Vector
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -8.0f));
    shader.setMat4("view", view);

    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 3000.0f);
    shader.setMat4("projection", projection);
    
    for (int i = 0; i < cubePositions.size(); i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions.at(i));
        shader.setMat4("model", model);

        glBindVertexArray(vaoId);
        glDrawElements(GL_TRIANGLES, TOTAL_VERTICES_PER_TILE, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void storeVertexDataOnGpu()
{
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glGenBuffers(1, &eboId);
    
    glBindVertexArray(vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void buildPositionData()
{
    if (cubePositions.size() > 0)
    {
        cubePositions.clear();
    }

    int halfTilesTotal = (TOTAL_TILES / 2) + 1;
    std::cout << TOTAL_TILES << ", " << halfTilesTotal << std::endl;

    for (int i = 0; i < halfTilesTotal; i++)
    {
        for (int j = 0; j < halfTilesTotal; j++)
        {
            cubePositions.emplace_back(glm::vec3(i, 0.0f, j));
        }
    }

    for (int i = 0; i < cubePositions.size(); i++)
    {
        std::cout << "item: " << cubePositions.at(i).x << ", " << cubePositions.at(i).z << std::endl;
    }
}
