#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../../headers/stb_image.h"

#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// Variables
size_t WINDOW_WIDTH = 1000;
size_t WINDOW_HEIGHT = 720;

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
	"uniform sampler2D ourTexture2; \n"
    "void main() \n"
    "{\n"
	/*
		The final output color is now the combination of two texture lookups. 
		GLSL's built-in mix function takes two values as input and linearly interpolates between them based on its third argument. 
		If the third value is 0.0 it returns the first input; if it's 1.0 it returns the second input value. 
		A value of 0.2 will return 80% of the first input color and 20% of the second input color, 
		resulting in a mixture of both our textures.
	*/
	"    FragColour = mix(texture(ourTexture, TexCoord), texture(ourTexture2, TexCoord), 0.5); \n"
	//" FragColour = texture(ourTexture2, TexCoord); \n"
	"   "
    "}\0";

unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0 // TODO play around with this
};

unsigned int vboId, vaoId, eboId;
unsigned int textures[2];
unsigned int vertexShaderId, fragmentShaderId, shaderProgramId;

// Function Declarations.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);
unsigned int applyShader(GLenum shaderType, const char *source);
void buildShaderProgram();
void storeVertexDataOnGpu(float vertices[], unsigned int verticeListSize);
void draw();
void loadTexture(std::string path, unsigned int textureId, GLenum rgbTypeA, GLenum rgbTypeB);
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
	vertexShaderId = applyShader(GL_VERTEX_SHADER, vertexShaderSource);
	fragmentShaderId = applyShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	buildShaderProgram();

	float xPos = 0.5f;
	float yPos = 0.5f;
	float moveIncrement = -0.005f;

	while(!glfwWindowShouldClose(window))
	{
		// Input
		processInput(window);

		// Rendering commands
		float vertices[] = {
			// positions          // colors           // texture coords
			-xPos,  (yPos * 2), 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
			-xPos,  yPos, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
			-(xPos * 2), yPos, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
			-(xPos * 2),  (yPos * 2), 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
		};

		storeVertexDataOnGpu(vertices, sizeof(vertices));

		draw();

		xPos += moveIncrement;
		yPos += moveIncrement;
		if ((yPos >= 0.5|| xPos >= 0.5) || (yPos <= -0.5 || xPos <= -0.5))
		{
			moveIncrement = -moveIncrement;
		} 

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


void draw()
{
	// Clear the screen with a colour
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	// Every shader and rendering call after the `glUseProgram` call will now use this program object (and thus the shaders).
	glUseProgram(shaderProgramId);
	
	glBindVertexArray(vaoId);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void storeVertexDataOnGpu(float vertices[], unsigned int verticeListSize)
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &eboId);

	glBindVertexArray(vaoId);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, verticeListSize, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

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

    /* Set texture filtering parameters
	 * When scaling up use nearest/nearest neighbour/point filtering.
	 * When scaling down use `nearest_minmap_linear` filtering:
	 * - linearly interpolates between the two mipmaps that most closely match the size of a pixel 
	 * - and samples the interpolated level via nearest neighbor interpolation.
	 * ------
	 * - Close objects to viewer use 'nearest' filtering, 
	 * - Distant objects use 'linear' filtering
	 * ------
	 * 
	 * OpenGL uses a concept called mipmaps that is basically a collection of texture images where 
	 * each subsequent texture is twice as small compared to the previous one. 
	 * The idea behind mipmaps should be easy to understand: after a certain distance threshold 
	 * from the viewer, OpenGL will use a different mipmap texture that best suits the distance to the object. 
	 * Because the object is far away, the smaller resolution will not be noticeable to the user. 
	 * OpenGL is then able to sample the correct texels, and there's less cache memory involved 
	 * when sampling that part of the mipmaps.
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(std::filesystem::path(path).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
		/*
		 * 1. The first argument specifies the texture target; setting this to GL_TEXTURE_2D means this operation will generate 
			  a texture on the currently bound texture object at the same target (so any textures bound to targets GL_TEXTURE_1D 
			  or GL_TEXTURE_3D will not be affected).
		 * 2. The second argument specifies the mipmap level for which we want to create a texture for if you want to set each 
			  mipmap level manually, but we'll leave it at the base level which is 0.
		 * 3. The third argument tells OpenGL in what kind of format we want to store the texture. 
			  Our image has only RGB values so we'll store the texture with RGB values as well.
		 * 4/5. The 4th and 5th argument sets the width and height of the resulting texture. 
			    We stored those earlier when loading the image so we'll use the corresponding variables. 
		 * 6. The next argument should always be 0 (some legacy stuff).
		 * 7/8. The 7th and 8th argument specify the format and datatype of the source image. 
			    We loaded the image with RGB values and stored them as chars (bytes) so we'll pass in the corresponding values.
		 * 8. The last argument is the actual image data.
		*/
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