#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "stdlib.h"
#include <iostream>

// Temporarily declare
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void render(GLFWwindow* window);

int main() 
{
    std::cout << "Hello, GLFW!" << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int WINDOW_WIDTH = 800;
	int WINDOW_HEIGHT = 600;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
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
		glfwSwapBuffers(window);
		glfwPollEvents();    
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
