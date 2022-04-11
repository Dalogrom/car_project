#pragma once
#include <fstream>
#include <string>
#include <sstream>
//#include <malloc.h>
#include <glad.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

// Window dimensions
const GLuint WINDOW_WIDTH = 650, WINDOW_HEIGHT = 650;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
ShaderProgramSource ParseShader(const std::string& filepath);
unsigned int CompileShader(unsigned int type, const std::string& source);
unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);