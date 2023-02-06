#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

// Variables

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
    "   FragColour = vec4(1.0f, 0.5f, 0.2f, 1.0); \n"
    "}\0";


GLint vertexShaderId;
GLint fragmentShaderId;

// Triangle vertices, note the range being between [-1, 1] which is required by OpenGL.
// This range is known as 'normalized device coordinates' - See README
// Since OpenGL works in a 3D space, and we want to render a 2D triangle, the 'Z' value is 0.
float vertices[] = {
	// x, y, z
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};  

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);

void storeVertexDataOnGpu();
void applyVertexShader();
void applyFragmentShader();
void buildShaderProgram();
void linkVertexAttributes();

void debug(GLint shaderRef, size_t mode);
void processInput(GLFWwindow* window);

int main() 
{
    std::cout << "Hello, GLFW!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int WINDOW_WIDTH = 800;
	int WINDOW_HEIGHT = 600;

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

	glfwTerminate();
    return 0;
}

void render(GLFWwindow* window)
{
	while(!glfwWindowShouldClose(window))
	{
		// Input
		processInput(window);

		// Rendering commands
		glClearColor(0.0f, 1.0f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Shaders
		applyVertexShader();
		applyFragmentShader();
		buildShaderProgram();
		linkVertexAttributes();

		/*
			From this point on we have everything set up: we initialized the vertex data in a buffer using a vertex buffer object, 
			set up a vertex and fragment shader and told OpenGL how to link the vertex data to the vertex shader's vertex attributes. 
		*/

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
	Core OpenGL requires that we use a Vertex Array Object or VAO so it knows what to do with our vertex inputs. 
	If we fail to bind a VAO, OpenGL will most likely refuse to draw anything.	
*/
void storeVertexDataOnGpu()
{
	// Generate a Vertex Buffer Object (VBO) to batch send data to the gpu at once. 
	// This is with a buffer ID using the glGenBuffers function.
	GLuint vboId;

	// The first parameter here describes the size or amount of vertex buffer objects to store in the gpu's memory.
	// If greater than 1, we need to update or VBO above to an array.
	// This will return a list of IDs corresponding to each generated VBO and store them in our VBO object.
	glGenBuffers(1, &vboId);

	// OpenGL has many types of buffer objects and the buffer type of a vertex buffer object is GL_ARRAY_BUFFER. 
	// OpenGL allows us to bind to several buffers at once as long as they have a different buffer type.
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
	GLint shaderProgram;
	shaderProgram = glCreateProgram();

	// Self-explanatory, link the shaders to the `shaderProgram`
	glAttachShader(shaderProgram, vertexShaderId);
	glAttachShader(shaderProgram, fragmentShaderId);
	glLinkProgram(shaderProgram);

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
	glUseProgram(shaderProgram);

	// Delete the shader objects once we've linked them into the program object; we no longer need them anymore.
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId); 

	debug(shaderProgram, 2);
}

/*
	We have to specify how OpenGL should interpret the vertex data before rendering.
	Vertices must be in contigious memory such that the linking below can apply on all vertex sequentially.
	See README diagram, fig 1.2.

	Each vertex attribute takes its data from memory managed by a VBO and 
	which VBO it takes its data from (you can have multiple VBOs) is determined by the VBO currently bound to GL_ARRAY_BUFFER 
	when calling glVertexAttribPointer.
	
	Since the previously defined VBO is still bound before calling glVertexAttribPointer vertex attribute 0 
	is now associated with its vertex data.
*/
void linkVertexAttributes()
{
	/*
		1. The first parameter specifies which vertex attribute we want to configure. 
			Remember that we specified the location of the position vertex attribute in the vertex shader with layout (location = 0). 
			This sets the location of the vertex attribute to 0 and since we want to pass data to this vertex attribute, we pass in 0.

		2. The next argument specifies the size of the vertex attribute. The vertex attribute is a vec3 so it is composed of 3 values.

		3. The third argument specifies the type of the data which is GL_FLOAT (a vec* in GLSL consists of floating point values).

		4. The next argument specifies if we want the data to be normalized. 
			If we're inputting integer data types (int, byte) and we've set this to GL_TRUE, 
			the integer data is normalized to 0 (or -1 for signed data) and 1 when converted to float. 
			This is not relevant for us since our data type is float so we'll leave this at GL_FALSE.
			
		5. The fifth argument is known as the `stride` and tells us the space between consecutive vertex attributes. 
			Since the next set of position data is located exactly 3 times the size of a float away we specify that value as the stride. 
			Note that since we know that the array is tightly packed / contigious (there is no space between the next vertex attribute value) 
			we could've also specified the stride as 0 to let OpenGL determine the stride (this only works when values are tightly packed). 
			Whenever we have more vertex attributes we have to carefully define the spacing between each vertex attribute.

		6. The last parameter is of type void* and thus requires that weird cast. 
			This is the offset of where the position data begins in the buffer. 
			Since the position data is at the start of the data array this value is just 0.
	*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	/*
		Enable the vertex attribute with glEnableVertexAttribArray giving the vertex attribute location as its argument; 
		vertex attributes are disabled by default.
	*/
	glEnableVertexAttribArray(0);  
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