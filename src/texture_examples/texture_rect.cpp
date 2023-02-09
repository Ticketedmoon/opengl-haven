#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"

#include <filesystem>

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
	"layout (location = 1) in vec3 aColor; \n"
	"layout (location = 2) in vec2 aTexCoord; \n"
	"out vec3 ourColor; \n"
	"out vec2 TexCoord; \n"
    "void main() \n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); \n"
	"   ourColor = aColor; \n"
    "	TexCoord = aTexCoord; \n"
    "}\0";

const char *fragmentShaderSource = 
	"#version 330 core \n"
    "out vec4 FragColour; \n"
	"in vec3 ourColor; \n"
	"in vec2 TexCoord; \n"
	"uniform sampler2D ourTexture; \n"
    "void main() \n"
    "{\n"
    // "   FragColour = vec4(ourColor, 1.0); \n"
	"    FragColour = texture(ourTexture, TexCoord); \n"
	"   "
    "}\0";

/* 
	A Vertex Buffer Object (VBO) and Vertex Array Object (VAO) to batch send data to the gpu at once. 
	This is with a buffer ID using the glGenBuffers function.
*/
unsigned int vboId, vaoId, eboId, texture;

// Shader references and shader program reference.
unsigned int vertexShaderId, fragmentShaderId, shaderProgramId;

/* 
	Triangle vertices, note the range being between [-1, 1] which is required by OpenGL.
	This range is known as 'normalized device coordinates' - See README
	Since OpenGL works in a 3D space, and we want to render a 2D triangle, the 'Z' value is 0.
*/
float vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   10.0f, 10.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   10.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 10.0f    // top left 
};

// Specify the order at which we want to draw these vertices in.
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0 // TODO play around with this
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
void buildShaderProgram();
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

	glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
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

	/*
		The fragment shader is the second and final shader we're going to create for rendering a triangle. 
		The fragment shader is all about calculating the color output of your pixels. 
	*/
	fragmentShaderId = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	buildShaderProgram();
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
	glUseProgram(shaderProgramId);

	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(vaoId);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;

    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(std::filesystem::path("resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
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