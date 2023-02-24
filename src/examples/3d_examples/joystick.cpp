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

struct Joystick {
    float leftX;
    float leftY;
    float L2;
    float rightX;
    float rightY;
    float R2;
};

// Variables
size_t WINDOW_WIDTH = 800;
size_t WINDOW_HEIGHT = 600;

float lastMouseX = WINDOW_WIDTH / 2;
float lastMouseY = WINDOW_HEIGHT / 2;

float pitch = 0.0f;
float yaw = -90.0f;

bool firstMouse = true;

float fov = 45.0f;

Joystick joystick = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};

/* Vertex shader
   Version at the top must match openGL version + we are using `core` profile.

   To set the output of the vertex shader we have to assign the position data 
   to the predefined gl_Position variable which is a `vec4` behind the scenes
   The last parameter is the width which we will default to 1 for each vector.
   TODO: Experiment with the parameters here.
   TODO: Move these to own shader `glsl` files.
*/
const char *vertexShaderSource = 
	"#version 330 core \n"
	"layout (location = 0) in vec3 aPos; \n"
	"layout (location = 1) in vec2 aTexCoord; \n"
	"out vec2 TexCoord; \n"
    "uniform mat4 model; \n"
    "uniform mat4 view; \n"
    "uniform mat4 projection; \n"
    "void main() \n"
    "{\n"
    // Note: The order of matrix multiplication here is important.
    "   gl_Position = projection * view * model * vec4(aPos, 1.0); \n"
    "	TexCoord = aTexCoord; \n"
    "}\0";

const char *fragmentShaderSource = 
	"#version 330 core \n"
    "out vec4 myOutput; \n"
	"in vec2 TexCoord; \n"
	"uniform sampler2D ourTexture; \n"
	"uniform sampler2D ourTexture2; \n"
    "void main() \n"
    "{\n"
	"    myOutput = mix(texture(ourTexture, TexCoord), texture(ourTexture2, TexCoord), 0.5); \n"
	"   "
    "}\0";

unsigned int vboId, vaoId;
unsigned int textures[2];
unsigned int vertexShaderId, fragmentShaderId, shaderProgramId;

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void joystick_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void render(GLFWwindow* window);
unsigned int applyShader(GLenum shaderType, const char *source);
void buildShaderProgram();
void storeVertexDataOnGpu();
void draw();
void loadTexture(std::string path, unsigned int textureId, GLenum rgbTypeA, GLenum rgbTypeB);
void debug(unsigned int shaderRef, size_t mode);

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

	glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
    glDeleteProgram(shaderProgramId);

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	vertexShaderId = applyShader(GL_VERTEX_SHADER, vertexShaderSource);
	fragmentShaderId = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	buildShaderProgram();
	storeVertexDataOnGpu();

	while(!glfwWindowShouldClose(window))
	{
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        //std::cout << "Joystick #1 Connection Status: " << (present == 1 ? "True" : "False") << std::endl;

        if (present)
        {
            int axesCount;
            const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
            // std::cout << "left X: " << axes[0] << std::endl;
            // std::cout << "left Y: " << axes[1] << std::endl;
            // std::cout << "L2: " << axes[2] << std::endl;

            // std::cout << "right X: " << axes[3] << std::endl;
            // std::cout << "right Y: " << axes[4] << std::endl;
            // std::cout << "R2: " << axes[5] << std::endl;

            joystick = {axes[0], axes[1], axes[2], axes[3], axes[4], axes[5]};
        }

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
    if (joystick.R2 > -0.3)
    {
        std::cout << fov << std::endl;
        fov -= (joystick.R2 + 1);
    }
    if (joystick.L2 > -0.3)
    {
        std::cout << fov << std::endl;
        fov += (joystick.L2);
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
    std::cout << xpos << ", " << ypos << std::endl;
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

void joystick_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (xpos > 0.3 || xpos < -0.3 || ypos > 0.3 || ypos < -0.3)
    {
        lastMouseX -= xpos;
        lastMouseY += ypos;
        std::cout << lastMouseX << ", " << lastMouseY << std::endl;
        
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
	glClearColor(0.0f, 1.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
	glUseProgram(shaderProgramId);

    // 3d
    glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    float radius = 30.0f;
    float camX = static_cast<float>(sin(glfwGetTime()) * radius);
    float camZ = static_cast<float>(cos(glfwGetTime()) * radius);

    /*
    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 1.5f), // Camera pos
        glm::vec3(0.0f, 0.0f, 0.0f), // Target pos
        glm::vec3(1.0f, 1.0f, 10.0f)); // Up vector (World-Space)
    */
    view = glm::lookAt(cameraPos, // Camera Pos
                       cameraPos + cameraFront, // Target Pos
                       cameraUp); // Up Vector

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -8.0f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vaoId);
	unsigned int totalCubes = sizeof(cubePositions)/sizeof(cubePositions[0]);
	for (int i = 0; i < totalCubes; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[i]);
		float angle = 10.0f * (i + 1); 
		model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model));
    	glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void storeVertexDataOnGpu()
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);

	glBindVertexArray(vaoId);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// load and create a texture 
    glGenTextures(2, textures);
	// Active which texture we want

	loadTexture("resources/textures/container.jpg", 0, GL_RGB, GL_RGB);
	loadTexture("resources/textures/container_2.png", 1, GL_RGBA, GL_RGBA);

	// Super Important to set up uniform locations for shaders to have visibility of our textures.
	// Note: It's required to use the shaderProgram before assigning these uniform locations.
	glUseProgram(shaderProgramId);
	glUniform1i(glGetUniformLocation(shaderProgramId, "ourTexture"), 0); 
	glUniform1i(glGetUniformLocation(shaderProgramId, "ourTexture2"), 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

unsigned int applyShader(GLenum shaderType, const char *source)
{
	unsigned int shaderReferenceId = glCreateShader(shaderType);
	glShaderSource(shaderReferenceId, 1, &source, NULL);
	glCompileShader(shaderReferenceId);
	debug(shaderReferenceId, shaderType == GL_VERTEX_SHADER ? 1 : 2);
	return shaderReferenceId;
}

void buildShaderProgram()
{
	// Same `id` reference patern as with the complation of the Vertex and Fragment shaders. 
	shaderProgramId = glCreateProgram();

	// Self-explanatory, link the shaders to the `shaderProgram`
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragmentShaderId);
	glLinkProgram(shaderProgramId);

	// Delete the shader objects once we've linked them into the program object; we no longer need them anymore.
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId); 

	debug(shaderProgramId, 0);
}

void loadTexture(std::string path, unsigned int textureId, GLenum rgbTypeA, GLenum rgbTypeB)
{
    glBindTexture(GL_TEXTURE_2D, textures[textureId]); // all upcoming GL_TEXTURE_2D operations now have an effect on this texture object

    // set the texture wrapping parameters
	
	// These two lines are only associated with GL_CLAMP_TO_BORDER 
	// float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

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

void debug(unsigned int shaderRef, size_t mode)
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