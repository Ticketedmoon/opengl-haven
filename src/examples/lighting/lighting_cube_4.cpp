/*
 * This file has phon lighting applied and can be adjusted using the UP/DOWN arrow keys.
 * The light source will rotate around the xz-axis using sin/cos 
 *
 * Note: Specular lighting will only apply on the cube face adjacent to the light source.
 *       This needs to be checked conditionally since openGL's reflect() glsl function does not ignore back-facing normal vectors.
 */

#include <cstdint>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../../headers/stb_image.h"

#include <filesystem>

const char *vertexShaderSource = 
	"#version 330 core \n"
	"layout (location = 0) in vec3 aPos; \n"
	"layout (location = 1) in vec2 aTexCoord; \n"
	"layout (location = 2) in vec3 aNormal; \n"
	"out vec2 TexCoord; \n"
	"out vec3 Normal; \n"
	"out vec3 FragmentPos; \n"
    "uniform mat4 model; \n"
    "uniform mat4 view; \n"
    "uniform mat4 projection; \n"
    "void main() \n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); \n"
    "   FragmentPos = vec3(model * vec4(aPos, 1.0));             \n"
    "	TexCoord = aTexCoord; \n"
    "	Normal = mat3(transpose(inverse(model))) * aNormal; \n"
    "}\0";

const char *fragmentShaderSource = 
	"#version 330 core \n"
    "out vec4 myOutput; \n"
	"in vec2 TexCoord; \n"
	"in vec3 Normal; \n"
	"in vec3 FragmentPos; \n"
	"uniform sampler2D ourTexture; \n"
	"uniform sampler2D ourTexture2; \n"
	"uniform vec3 lightColour; \n"
	"uniform vec3 lightPos; \n"
	"uniform float ambientStrength; \n"
	"uniform vec3 cameraPos; \n"
    "                                                                           \n"
    "void main() \n"
    "{\n"
    "    vec3 objectColour = vec3(1.0f, 0.5f, 0.30f);                                \n"
    "    vec3 ambient = lightColour * ambientStrength;                         \n"

    "    vec3 norm = normalize(Normal);                           \n"
    "    vec3 lightDir = normalize(lightPos - FragmentPos);                           \n"
    "    float diff = max(dot(norm, lightDir), 0.0);                           \n"
    "    vec3 diffuse = diff * lightColour;                           \n"

    "    float specularStrength = 1.0;                           \n"
    "    vec3 cameraDirection = normalize(cameraPos - FragmentPos);                           \n"
    "    vec3 reflectDir = reflect(-lightDir, norm);                           \n"
    "    float spec = pow(max(dot(cameraDirection, reflectDir), 0.0), 128);                 \n"
    "    vec3 specular = specularStrength * spec * lightColour;                            \n"

    "    if (diff > 0.0)                           \n"
    "    {                                             \n"
    "        vec3 result = objectColour * (ambient + diffuse + specular);                           \n"
    "        myOutput = vec4(result, 1.0);                          \n"
    "    }                                              \n"
    "    else                                              \n"
    "    {                                             \n"
    "        vec3 result = objectColour * (ambient + diffuse);                           \n"
    "        myOutput = vec4(result, 1.0);                          \n"
    "    }                                              \n"
    "}\0";

const char *lightSourceVertexShader = 
	"#version 330 core \n"
	"layout (location = 0) in vec3 aPos; \n"
    "uniform mat4 model; \n"
    "uniform mat4 view; \n"
    "uniform mat4 projection; \n"
    "void main() \n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); \n"
    "}\0";

const char *lightSourceFragmentShader = 
	"#version 330 core  \n"
    "out vec4 myOutput; \n"
    "                   \n"
    "void main()        \n"
    "{\n"
    "    myOutput = vec4(1.0);"
    "}\0";

uint32_t vboId;
uint32_t cubeVaoIds[2];
uint32_t textures[2];
uint32_t vertexShaderId, fragmentShaderId, lightSourceVertexShaderId, lightSourceFragmentShaderId;
uint32_t cubeShaderProgramId, lightSourceShaderProgramId;

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};


size_t WINDOW_WIDTH = 1280;
size_t WINDOW_HEIGHT = 720;

float lastMouseX = WINDOW_WIDTH / 2;
float lastMouseY = WINDOW_HEIGHT / 2;

float pitch = 0.0f;
float yaw = -90.0f;

bool firstMouse = true;

float fov = 45.0f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 cubePos(0.0f, 0.0f, 0.0f);
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

static float ambientStrength = 0.5f;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
uint32_t applyShader(GLenum shaderType, const char *source);
uint32_t buildShaderProgram(uint32_t vShaderId, uint32_t fShaderId);
void storeVertexDataOnGpu();
void draw();
void loadTexture(std::string path, uint32_t textureId, GLenum rgbTypeA, GLenum rgbTypeB);
void debug(uint32_t shaderRef, size_t mode);

int main() 
{
    std::cout << "Hello, GLFW!" << std::endl;

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

	glDeleteVertexArrays(2, cubeVaoIds);
    glDeleteBuffers(1, &vboId);
    glDeleteProgram(cubeShaderProgramId);
    glDeleteProgram(lightSourceShaderProgramId);

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	vertexShaderId = applyShader(GL_VERTEX_SHADER, vertexShaderSource);
	fragmentShaderId = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	lightSourceVertexShaderId = applyShader(GL_VERTEX_SHADER, lightSourceVertexShader);
	lightSourceFragmentShaderId = applyShader(GL_FRAGMENT_SHADER, lightSourceFragmentShader);

	cubeShaderProgramId = buildShaderProgram(vertexShaderId, fragmentShaderId);
	lightSourceShaderProgramId = buildShaderProgram(lightSourceVertexShaderId, lightSourceFragmentShaderId);
	storeVertexDataOnGpu();

	while(!glfwWindowShouldClose(window))
	{
		// Input
		processInput(window);

		// Rendering commands
		draw();

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


    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (ambientStrength < 1.0f)
        {
            ambientStrength += 0.025;
        }
	}

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (ambientStrength > 0.0f)
        {
            ambientStrength -= 0.025;
        }
	}

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    /*
        If we want to move forward or backwards we add or subtract the direction vector from the position vector
        scaled by some speed value. If we want to move sideways we do a cross product to create a right vector 
        and we move along the right vector accordingly. This creates the familiar strafe effect when using the camera.
        
        Note: that we normalize the resulting right vector. If we wouldn't normalize this vector, the resulting cross product 
        may return differently sized vectors based on the cameraFront variable. If we would not normalize the vector we would 
        move slow or fast based on the camera's orientation instead of at a consistent movement speed.
    */
    const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= (cameraSpeed * cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += (glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed);
    }
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

    /*
        Note that we multiply the offset values by a sensitivity value. 
        If we omit this multiplication the mouse movement would be way too strong; 
        fiddle around with the sensitivity value to your liking.
    */
    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    /*
        add some constraints to the camera so users won't be able to make weird camera movements 
        (also causes a LookAt flip once direction vector is parallel to the world up direction). 
        
        The pitch needs to be constrained in such a way that users won't be able to look higher than 89 degrees 
        (at 90 degrees we get the LookAt flip) and also not below -89 degrees. 
        This ensures the user will be able to look up to the sky or below to his feet but not further.
    */
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
    {
        fov = 1.0f;
    }
    if (fov > 45.0f)
    {
        fov = 45.0f; 
    }
}

void draw()
{
	// Clear the screen with a colour
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
	glUseProgram(cubeShaderProgramId);

    // Lighting attributes required in cube fragment shader.
    glUniform1f(glGetUniformLocation(cubeShaderProgramId, "ambientStrength"), ambientStrength);
    glUniform3f(glGetUniformLocation(cubeShaderProgramId, "lightColour"), 1.0f, 1.0f, 1.0f);
    // Note: Can use 3fv uniform to pass glm::vec* objects, but require value_ptr call
    
    glUniform3fv(glGetUniformLocation(cubeShaderProgramId, "cameraPos"), 1, glm::value_ptr(cameraPos));

    // 3d
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

    view = glm::lookAt(cameraPos, // Camera Pos
                       cameraPos + cameraFront, // Target Pos
                       cameraUp); // Up Vector

    view = glm::translate(view, glm::vec3(1.0f, 0.0f, -2.0f));
	glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePos);
    //float angle = 30.0f; 
    //model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
    glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(cubeVaoIds[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    float lightPosRadius = 2.0f;
    glm::vec3 newLightPos = glm::vec3(lightPosRadius * sin(glfwGetTime()), lightPos.y, 1.5f * cos(glfwGetTime()));
    glUniform3fv(glGetUniformLocation(cubeShaderProgramId, "lightPos"), 1, glm::value_ptr(newLightPos));

    // Light source
    glUseProgram(lightSourceShaderProgramId);

    glUniformMatrix4fv(glGetUniformLocation(lightSourceShaderProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShaderProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));

    float angle = 30.0f; 
    model = glm::mat4(1.0f);
    model = glm::translate(model, newLightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube

    //float radius = 10.0f;
    //model = glm::translate(model, glm::vec3(sin(glfwGetTime()) * radius, 0.0f, sin(glfwGetTime()) * radius));
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShaderProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(cubeVaoIds[1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void storeVertexDataOnGpu()
{
	glGenBuffers(1, &vboId);

	glGenVertexArrays(2, &cubeVaoIds[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVaoIds[0]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
    glGenTextures(2, textures);

	// Active which texture we want
	loadTexture("resources/textures/container.jpg", 0, GL_RGB, GL_RGB);
	loadTexture("resources/textures/container_2.png", 1, GL_RGBA, GL_RGBA);

	// Super Important to set up uniform locations for shaders to have visibility of our textures.
	// Note: It's required to use the shaderProgram before assigning these uniform locations.
	glUseProgram(cubeShaderProgramId);
	glUniform1i(glGetUniformLocation(cubeShaderProgramId, "ourTexture"), 0); 
	glUniform1i(glGetUniformLocation(cubeShaderProgramId, "ourTexture2"), 1);

    /*
     * Create separate VAO for the light source.
     * A light source is simply another object in the scene that casts light on other objects from a location.
     */
	glGenVertexArrays(1, &cubeVaoIds[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
    // No need to add bufferData here, we can reuse the buffer data already present from the call earlier.
	glBindVertexArray(cubeVaoIds[1]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

uint32_t applyShader(GLenum shaderType, const char *source)
{
	uint32_t shaderReferenceId = glCreateShader(shaderType);
	glShaderSource(shaderReferenceId, 1, &source, NULL);
	glCompileShader(shaderReferenceId);
	debug(shaderReferenceId, shaderType == GL_VERTEX_SHADER ? 1 : 2);
	return shaderReferenceId;
}

uint32_t buildShaderProgram(uint32_t vShaderId, uint32_t fShaderId)
{
	// Same `id` reference patern as with the complation of the Vertex and Fragment shaders. 
	uint32_t shaderProgramId = glCreateProgram();

	// Self-explanatory, link the shaders to the `shaderProgram`
	glAttachShader(shaderProgramId, vShaderId);
	glAttachShader(shaderProgramId, fShaderId);
	glLinkProgram(shaderProgramId);

	// Delete the shader objects once we've linked them into the program object; we no longer need them anymore.
	glDeleteShader(vShaderId);
	glDeleteShader(fShaderId);

	debug(shaderProgramId, 0);
    return shaderProgramId;
}

void loadTexture(std::string path, uint32_t textureId, GLenum rgbTypeA, GLenum rgbTypeB)
{
    glBindTexture(GL_TEXTURE_2D, textures[textureId]); // all upcoming GL_TEXTURE_2D operations now have an effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(std::filesystem::path(path).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, rgbTypeA, width, height, 0, rgbTypeB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void debug(uint32_t shaderRef, size_t mode)
{
	int success;
	char infoLog[512];
	if (mode > 0)
	{
		glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(shaderRef, 512, NULL, infoLog);
			std::string shaderName;
			if (mode == 1)
			{
				shaderName = "VERTEX";
			}
			else if (mode == 2)
			{
				shaderName = "FRAGMENT";
			}
			else {
				shaderName == "NO DEBUG SHADER NAME";
			}
			std::cout << "ERROR::SHADER::" << shaderName << "::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	} else if (mode == 0)
	{
		glGetProgramiv(shaderRef, GL_LINK_STATUS, &success);
		if(!success)
		{
    		glGetProgramInfoLog(shaderRef, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
}
