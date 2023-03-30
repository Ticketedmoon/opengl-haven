#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>
#include <math.h>

// Variables
size_t WINDOW_WIDTH = 800;
size_t WINDOW_HEIGHT = 600;

const char *vertexShaderSource = 
	"#version 330 core \n"
    "layout (location = 0) in vec3 aPos; \n"
    "layout (location = 1) in vec3 aColour; \n"
    "out vec3 outputColour; \n"
    "void main() \n"
    "{\n"
    "   gl_Position = vec4(aPos.xyz, 1.0); \n" // 'swizzling'
    "   outputColour = aColour; \n" // 'swizzling'
    "}\0";

const char *fragmentShaderSource = 
	"#version 330 core \n"
    "out vec4 FragColour; \n"
	"in vec3 outputColour; \n"
    "void main() \n"
    "{\n"
    "   FragColour = vec4(outputColour, 1.0); \n"
    "}\0";

unsigned int vbos[1], vaos[1];

// Shader references and shader program reference.
unsigned int vertexShaderId, fragmentShaderId, shaderProgramId;

float vertices[] = {
	// Position Data      Colour data
    -0.3f, -0.3f, 0.0f,   1.0f, 0.0f, 0.0f,
     0.3f, -0.3f, 0.0f,   0.0f, 1.0f, 0.0f,
     0.0f,  0.6f, 0.0f,    0.0f, 0.0f, 1.0f
};

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);

unsigned int applyShader(GLenum shaderType, const char *source);
unsigned int buildShaderProgram(unsigned int fragmentShaderId);
void storeVertexDataOnGpu();
void draw();
void debug(unsigned int shaderRef, size_t mode);
void processInput(GLFWwindow* window);

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

	render(window);

	glDeleteVertexArrays(1, vaos);
    glDeleteBuffers(1, vbos);
    glDeleteProgram(shaderProgramId);

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	/*
		The vertex shader manages the vertex points on the screen and the associated vertex attributes.
	*/
	vertexShaderId = applyShader(GL_VERTEX_SHADER, vertexShaderSource);
	fragmentShaderId = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	
	shaderProgramId = buildShaderProgram(fragmentShaderId);

	storeVertexDataOnGpu();

	/*
		Main loop.
		From this point on we have everything set up: we initialized the vertex data in a buffer using a vao & vbo, 
		set up a vertex and fragment shader and told OpenGL how to link the vertex data to the vertex shader's vertex attributes. 
	*/
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
}

/*
	To draw our objects of choice, OpenGL provides us with the glDrawArrays function that draws primitives using the currently active shader, 
	the previously defined vertex attribute configuration 
	and with the VBO's vertex data (indirectly bound via the VAO).
*/
void draw()
{
	// Clear the screen with a colour
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Inject colour value into `uniform` which is scopred to shaderProgram.
	glUseProgram(shaderProgramId);

	glBindVertexArray(vaos[0]);

	// TODO Should these be in draw loop or storeVertexDataOnGpu(...)
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0); // Unbind here for succinctness
}

void storeVertexDataOnGpu()
{
	glGenVertexArrays(1, vaos);

	glGenBuffers(1, vbos);
	
	// Array Object becomes active after binding, creating the object.
	glBindVertexArray(vbos[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // Remember: do NOT unbind the EBO / call the following while a VAO is active as the bound EBO IS stored in the VAO; keep the EBO bound.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 
}

unsigned int applyShader(GLenum shaderType, const char *source)
{
	unsigned int shaderReferenceId = glCreateShader(shaderType);
	glShaderSource(shaderReferenceId, 1, &source, NULL);
	glCompileShader(shaderReferenceId);
	debug(shaderReferenceId, 1);
	return shaderReferenceId;
}

unsigned int buildShaderProgram(unsigned int fragmentShaderId)
{
	// Same `id` reference patern as with the complation of the Vertex and Fragment shaders. 
	unsigned int shaderProgramId = glCreateProgram();

	// Self-explanatory, link the shaders to the `shaderProgram`
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragmentShaderId);

	glLinkProgram(shaderProgramId);

	// Delete the shader objects once we've linked them into the program object; we no longer need them anymore.
	// glDeleteShader(vertexShaderId);
	// glDeleteShader(fragmentShaderId); 

	debug(shaderProgramId, 2);

	return shaderProgramId;
}

void debug(unsigned int shaderRef, size_t mode)
{
	int success;
	char infoLog[512];
	if (mode == 1)
	{
		glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(shaderRef, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	} else if (mode == 2)
	{
		glGetProgramiv(shaderRef, GL_LINK_STATUS, &success);
		if(!success)
		{
    		glGetProgramInfoLog(shaderRef, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
}