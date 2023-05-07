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
bool imGuiMode = false;

float fov = 45.0f;

Joystick joystick = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

unsigned int vaoId;

glm::vec3 cameraPos   = glm::vec3(0.0f, 20.0f,  200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

unsigned int TOTAL_VERTICES = 54;
unsigned int ASTEROID_AMOUNT = 10000;

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void joystick_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
void storeVertexDataOnGpu(Model& rock);
void draw(Shader& rockShader, Model& rockModel);

glm::mat4* modelMatrices;

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
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    // Register mouse callback - Each time mouse moves this will be called with the (x,y) coords of the mouse.
    glfwSetCursorPosCallback(window, mouse_callback); 

    // Scroll callback (change fov of perspective project based on y coordinate)
    glfwSetScrollCallback(window, scroll_callback); 

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
    Shader rockShader("src/examples/instancing/advanced/asteroid_field/data/shaders/rock_shader.vs", "src/examples/instancing/advanced/asteroid_field/data/shaders/shader.fs");

    char* rockModelPath = "src/examples/instancing/advanced/asteroid_field/data/asteroid/rock.obj";
    Model rockModel(rockModelPath);

    bool show_window = true;

	storeVertexDataOnGpu(rockModel);

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();    

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
            if (ImGui::Button("Close Me"))
            {
                show_window = false;
            }
            ImGui::End();
        }

        ImGui::Render();

		// Rendering commands
		draw(rockShader, rockModel);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
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

    const float cameraSpeed = 50.0f * deltaTime; // adjust accordingly
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

void draw(Shader& rockShader, Model& rockModel)
{
	// Clear the screen with a colour
	// glClearColor(0.0f, 0.0f, 0.5f, 0.2f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    // 4d
    glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    float radius = 30.0f;
    float camX = static_cast<float>(sin(glfwGetTime()) * radius);
    float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

    view = glm::lookAt(cameraPos, // Camera Pos
                       cameraPos + cameraFront, // Target Pos
                       cameraUp); // Up Vector
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -8.0f));
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 3000.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));

    float angleOverTime = glfwGetTime() * 0.1f; 
    model = glm::rotate(model, angleOverTime, glm::vec3(0.0f, 1.0f, 1.0f));

    rockShader.use();
    rockShader.setMat4("projection", projection);
    rockShader.setMat4("view", view);
    rockShader.setInt("texture_diffuse1", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockModel.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
        glBindVertexArray(rockModel.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rockModel.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, ASTEROID_AMOUNT);
        glBindVertexArray(0);
    }
}

void storeVertexDataOnGpu(Model& rock)
{
    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    modelMatrices = new glm::mat4[ASTEROID_AMOUNT];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float radius = 150.0;
    float offset = 25.0f;
    for (unsigned int i = 0; i < ASTEROID_AMOUNT; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)ASTEROID_AMOUNT * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;

        displacement = (rand() % (int)(100 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
                                       //
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;

        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = static_cast<float>((rand() % 100) / 100.0 + 0.05);
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }
    
    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, ASTEROID_AMOUNT * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    std::cout << "Total Rock Meshes: " << rock.meshes.size() << std::endl;
    for (unsigned int i = 0; i < rock.meshes.size(); i++)
    {
        unsigned int VAO = rock.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
}

