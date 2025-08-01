#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <iostream>

GLuint VAO, VBO, EBO;

void rectangleModel() {
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f, // Bottom left
		 0.5f, -0.5f, 0.0f, // Bottom right
		 0.5f,  0.5f, 0.0f, // Top right
		-0.5f,  0.5f, 0.0f  // Top left
	};

	GLuint indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // unbind VAO (keeps EBO binding inside VAO)
}


const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 model;
void main() {
    gl_Position = model * vec4(position, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // orange color
}
)";

GLuint shaderID, uniformModel, uniformProjection, uniformView;
float paddleX = 0.0f;  // 0 = center

void addShader(GLuint theProgram, const char* shaderCode, GLenum shaderType) {
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);

	if (!result) {
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

void compileShader(const char* vertexCode, const char* fragmentCode) {
	shaderID = glCreateProgram();

	if (!shaderID) {
		printf("Error creating shader program!!\n");
		return;
	}

	addShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	addShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);

	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n", eLog);
		return;
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);

	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}

	uniformModel = glGetUniformLocation(shaderID, "model");
}

bool keys[1024];

void handleKeys(GLFWwindow* window, int key, int code, int action, int mode){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 0 && key < 1024){
		if (action == GLFW_PRESS){
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)		{
			keys[key] = false;
		}
	}
}


int main() {
	
	if(!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "2D OpenGL Brick Breaker", nullptr, nullptr);

	if(!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if(glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	int bufferWidth, bufferHeight;

	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

	glViewport(0, 0, bufferWidth, bufferHeight);

	compileShader(vertexShaderSource, fragmentShaderSource);
	rectangleModel();

	glfwSetKeyCallback(window, handleKeys);


	while(!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderID);

		if (keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_A]) {
			paddleX -= 0.02f;  // move left
		}
		if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_D]) {
			paddleX += 0.02f;  // move right
		}

		// Clamp paddleX so it stays inside the viewport
		float halfPaddle = 0.15f; // scale.x / 2
		if (paddleX < -1.0f + halfPaddle) paddleX = -1.0f + halfPaddle;
		if (paddleX > 1.0f - halfPaddle) paddleX = 1.0f - halfPaddle;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(paddleX, -0.8f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.1f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, &model[0][0]);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}