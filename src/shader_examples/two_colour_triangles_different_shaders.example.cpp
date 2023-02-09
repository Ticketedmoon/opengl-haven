#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

// Variables
size_t WINDOW_WIDTH = 800;
size_t WINDOW_HEIGHT = 600;

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
    "void main() \n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); \n"
    "}\0";

const char *fragmentShaderSourceA = 
	"#version 330 core \n"
    "out vec4 FragColour; \n"
    "void main() \n"
    "{\n"
    "   FragColour = vec4(0.0f, 0.6f, 0.85f, 1.0); \n"
    "}\0";

const char *fragmentShaderSourceB = 
	"#version 330 core \n"
    "out vec4 FragColour; \n"
    "void main() \n"
    "{\n"
    "   FragColour = vec4(1.0f, 1.0f, 0.0f, 1.0); \n"
    "}\0";

/* 
	A Vertex Buffer Object (VBO) and Vertex Array Object (VAO) to batch send data to the gpu at once. 
	This is with a buffer ID using the glGenBuffers function.
*/
unsigned int vbos[2], vaos[2], eboId;

// Shader references and shader program reference.
unsigned int vertexShaderId, fragmentShaderIdA, fragmentShaderIdB, shaderProgramIdA, shaderProgramIdB;

float verticesA[] = {
    -0.6f, 0.0f, 0.0f,
    -0.3f, 0.0f, 0.0f, 
    -0.45f, 0.3f, 0.0f,
};

float verticesB[] = {
    0.6f, 0.0f, 0.0f,
    0.3f, 0.0f, 0.0f, 
    0.45f, 0.3f, 0.0f,
};

// Specify the order at which we want to draw these vertices in.
unsigned int indices[] = {
	0, 1, 2,
	3, 4, 5,
};

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);

/*
	Creating the shader below will instantiate memory on the GPU
	and return an `id` reference to the shader.
	
	We must provide the type of shader we want to create as an argument to glCreateShader. 

	We attach the shader source code to the shader object and compile the shader.
	`glShaderSource` takes the shader object to compile to as its first argument.
	The second argument specifies how many strings we're passing as source code, which is only one.
	The third parameter is the actual source code of the vertex shader,
	and we can leave the 4th parameter to NULL.
*/
unsigned int applyShader(GLenum shaderType, const char *source);

/*
	A shader program object is the final linked version of multiple shaders combined. 
	To use the recently compiled shaders we have to:
	1. link them to a shader program object 
	2. and then activate this shader program when rendering objects. 
	
	The activated shader program's shaders will be used when we issue render calls.

	When linking the shaders into a program it links the outputs of each shader to the inputs of the next shader. 
	This is also where you'll get linking errors if your outputs and inputs do not match.
*/
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
    glDeleteProgram(shaderProgramIdA);
    glDeleteProgram(shaderProgramIdB);

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	/*
		The vertex shader manages the vertex points on the screen and the associated vertex attributes.
	*/
	vertexShaderId = applyShader(GL_VERTEX_SHADER, vertexShaderSource);
	fragmentShaderIdA = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSourceA);
	fragmentShaderIdB = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSourceB);
	
	shaderProgramIdA = buildShaderProgram(fragmentShaderIdA);
	shaderProgramIdB = buildShaderProgram(fragmentShaderIdB);

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

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
	glUseProgram(shaderProgramIdA);
	glBindVertexArray(vaos[0]);
	glEnableVertexAttribArray(0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); // Unbind here for succinctness

	glUseProgram(shaderProgramIdB);
	glBindVertexArray(vaos[1]);
	glEnableVertexAttribArray(0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void storeVertexDataOnGpu()
{
	glGenVertexArrays(2, vaos);

	glGenBuffers(2, vbos);
	glGenBuffers(1, &eboId);
	
	// Array Object becomes active after binding, creating the object.
	glBindVertexArray(vbos[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesA), verticesA, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(vbos[1]);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesB), verticesB, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

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