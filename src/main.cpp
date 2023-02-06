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

const char *fragmentShaderSource = 
	"#version 330 core \n"
    "out vec4 FragColour; \n"
    "void main() \n"
    "{\n"
    "   FragColour = vec4(0.0f, 0.6f, 0.85f, 1.0); \n"
    "}\0";


/* A Vertex Buffer Object (VBO) and Vertex Array Object (VAO) to batch send data to the gpu at once. 
   This is with a buffer ID using the glGenBuffers function.
*/
GLuint vboId, vaoId;

// Shader references and shader program reference.
GLint vertexShaderId, fragmentShaderId, shaderProgramId;

// Triangle vertices, note the range being between [-1, 1] which is required by OpenGL.
// This range is known as 'normalized device coordinates' - See README
// Since OpenGL works in a 3D space, and we want to render a 2D triangle, the 'Z' value is 0.
float vertices[] = {
	// x, y, z
    -0.2f, -0.2f, 0.0f,
     0.2f, -0.2f, 0.0f,
     0.0f,  0.3f, 0.0f
};  

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);

void storeVertexDataOnGpu();
void applyVertexShader();
void applyFragmentShader();
void buildShaderProgram();
void draw();

void debug(GLint shaderRef, size_t mode);
void processInput(GLFWwindow* window);

int main() 
{
    std::cout << "Hello, GLFW!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chaos", NULL, NULL);
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
	// Shaders
	applyVertexShader();
	applyFragmentShader();
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

	glBindVertexArray(vaoId);

	/*
		takes as its first argument the OpenGL primitive type we would like to draw.
		The second argument specifies the starting index of the vertex array we'd like to draw; we just leave this at 0. 
		The last argument specifies how many vertices we want to draw, 
			which is 3 (we only render 1 triangle from our data, which is exactly 3 vertices long).
	*/
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

/*
	Core OpenGL requires that we use a Vertex Array Object or VAO so it knows what to do with our vertex inputs. 
	If we fail to bind a VAO, OpenGL will most likely refuse to draw anything.

	The VAO stores our vertex attribute configuration and which VBO to use. 
	Usually when you have multiple objects you want to draw, you first generate/configure all 
	the VAOs (and thus the required VBO and attribute pointers) and store those for later use. 
	The moment we want to draw one of our objects, we take the corresponding VAO, bind it, then draw the object and unbind the VAO again.
*/
void storeVertexDataOnGpu()
{
	/* The first parameter here describes the size or amount of vertex buffer objects to store in the gpu's memory.
	   If greater than 1, we need to update or VAO above to an array.
	   This will return a list of IDs corresponding to each generated VBO and store them in our VAO object.
	*/
	glGenVertexArrays(1, &vaoId);

	glGenBuffers(1, &vboId);

	/* 1. bind Vertex Array Object
	   OpenGL has many types of buffer objects and the buffer type of a vertex buffer object is GL_ARRAY_BUFFER. 
	   OpenGL allows us to bind to several buffers at once as long as they have a different buffer type. 
	*/
	glBindVertexArray(vaoId);

	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	/*
		From this point on any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to configure 
		the currently bound buffer, which is VBO. 
		Then we can make a call to the glBufferData function that copies the previously defined vertex data 
		into the buffer's memory.
		`glBufferData(...)` is a function specifically targeted to copy user-defined data into the currently bound buffer. 

		The fourth parameter specifies how we want the graphics card to manage the given data. This can take 3 forms:
		GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
		GL_STATIC_DRAW: the data is set only once and used many times.
		GL_DYNAMIC_DRAW: the data is changed a lot and used many times.
		Correctly selecting the above will allow the GPU to optimize for reads and writes of the vertex data.
	*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/*
		3. Set our vertex attributes pointers
		
			We have to specify how OpenGL should interpret the vertex data before rendering.
		    Vertices must be in contigious memory such that the linking below can apply on all vertex sequentially.
		    See README diagram, fig 1.2.

		    Each vertex attribute takes its data from memory managed by a VBO and 
		    which VBO it takes its data from (you can have multiple VBOs) is determined by the VBO currently bound to GL_ARRAY_BUFFER 
		    when calling glVertexAttribPointer.
			
		    Since the previously defined VBO is still bound before calling glVertexAttribPointer vertex attribute 0 
		    is now associated with its vertex data.

			a. The first parameter specifies which vertex attribute we want to configure. 
				Remember that we specified the location of the position vertex attribute in the vertex shader with layout (location = 0). 
				This sets the location of the vertex attribute to 0 and since we want to pass data to this vertex attribute, we pass in 0.

			b. The next argument specifies the size of the vertex attribute. The vertex attribute is a vec3 so it is composed of 3 values.

			c. The third argument specifies the type of the data which is GL_FLOAT (a vec* in GLSL consists of floating point values).

			d. The next argument specifies if we want the data to be normalized. 
				If we're inputting integer data types (int, byte) and we've set this to GL_TRUE, 
				the integer data is normalized to 0 (or -1 for signed data) and 1 when converted to float. 
				This is not relevant for us since our data type is float so we'll leave this at GL_FALSE.
			
			e. The fifth argument is known as the `stride` and tells us the space between consecutive vertex attributes. 
				Since the next set of position data is located exactly 3 times the size of a float away we specify that value as the stride. 
				Note that since we know that the array is tightly packed / contigious (there is no space between the next vertex attribute value) 
				we could've also specified the stride as 0 to let OpenGL determine the stride (this only works when values are tightly packed). 
				whenever we have more vertex attributes we have to carefully define the spacing between each vertex attribute.

			f. the last parameter is of type void* and thus requires that weird cast. 
				this is the offset of where the position data begins in the buffer. 
				since the position data is at the start of the data array this value is just 0.
	*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	/*
		Enable the vertex attribute with glEnableVertexAttribArray giving the vertex attribute location as its argument; 
		vertex attributes are disabled by default.
	*/
	glEnableVertexAttribArray(0);  

	// TODO SEE IF WE CAN MOVE LINKING VERTEX DATA METHOD HERE.
	// SEE: https://learnopengl.com/Getting-started/Hello-Triangle#:~:text=Drawing%20an%20object%20in%20OpenGL%20would%20now%20look%20something%20like%20this%3A
}

void applyVertexShader()
{
	/*
		Similar to the above, creating the shader below will instantiate memory on the GPU
		and return an `if` reference to the shader.
		
		We provide the type of shader we want to create as an argument to glCreateShader. 
		Since we're creating a vertex shader we pass in GL_VERTEX_SHADER.
	*/
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

	/*
		We attach the shader source code to the shader object and compile the shader.
		`glShaderSource` takes the shader object to compile to as its first argument.
		The second argument specifies how many strings we're passing as source code, which is only one.
		The third parameter is the actual source code of the vertex shader,
		and we can leave the 4th parameter to NULL.
	*/
	glShaderSource(vertexShaderId, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderId);

	debug(vertexShaderId, 1);
}

/*
	The fragment shader is the second and final shader we're going to create for rendering a triangle. 
	The fragment shader is all about calculating the color output of your pixels. 
	To keep things simple the fragment shader will always output an orange-ish color.
*/
void applyFragmentShader()
{
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShaderId);

	debug(fragmentShaderId, 1);
}

/*
	A shader program object is the final linked version of multiple shaders combined. 
	To use the recently compiled shaders we have to:
	1. link them to a shader program object 
	2. and then activate this shader program when rendering objects. 
	
	The activated shader program's shaders will be used when we issue render calls.

	When linking the shaders into a program it links the outputs of each shader to the inputs of the next shader. 
	This is also where you'll get linking errors if your outputs and inputs do not match.
*/
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

	debug(shaderProgramId, 2);
}

void debug(GLint shaderRef, size_t mode)
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