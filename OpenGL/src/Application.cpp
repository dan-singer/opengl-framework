#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Shader.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.h"

#include <assimp/Importer.hpp>

// https://www.khronos.org/opengl/wiki/OpenGL_Error
void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}


Camera* camera = nullptr;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
int width = 960;
int height = 540;
bool firstMouse = true;
float lastX = width / 2.0f;
float lastY = height / 2.0f;

bool modulateColors = false;

constexpr int NUM_LIGHTS = 4;
glm::vec3 dirLightDirection(-0.2f, -1.0f, -0.3f);

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!camera) {
		return;
	}
	if (firstMouse) 
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoff = xpos - lastX;
	float yoff = ypos - lastY;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoff *= sensitivity;
	yoff *= sensitivity;

	float newYaw = camera->GetYaw() - xoff;
	float newPitch = camera->GetPitch() - yoff;

	if (newPitch > 89.0f)
		newPitch = 89.0f;
	if (newPitch < -89.0f)
		newPitch = -89.0f;


	camera->SetYaw(newYaw);
	camera->SetPitch(newPitch);

}

void ScrollCallback(GLFWwindow* window, double xoff, double yoff)
{
	if (!camera) {
		return;
	}
	camera->AdjustFOV(-yoff);
	if (camera->GetFOV() < 1.0f) {
		camera->SetFOV(1.0f);
	}
	else if (camera->GetFOV() > 45.0f) {
		camera->SetFOV(45.0f);
	}
}


void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		return;
	}

	if (!camera)
		return;

	const float cameraSpeed = camera->GetSpeed() * deltaTime; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->Translate(cameraSpeed * camera->GetForward());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->Translate(-cameraSpeed * camera->GetForward());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->Translate(-glm::normalize(glm::cross(camera->GetForward(), camera->GetUp())) * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->Translate(glm::normalize(glm::cross(camera->GetForward(), camera->GetUp())) * cameraSpeed);
}

int RunApp()
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	// OpenGL context must have been created before initializing GLEW!
	if (glewInit() != GLEW_OK)
		std::cout << "Error!\n";

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	glEnable(GL_DEPTH_TEST);

	std::cout << glGetString(GL_VERSION) << "\n";

	glm::vec3 pointLightPositions[NUM_LIGHTS] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::vec3 pointLightColors[NUM_LIGHTS] = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.7f, 0.8f, 0.9f),
		glm::vec3(1.0f, 0.0f, 0.2f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	};

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	Shader basicLitShader("res/shaders/BasicLit.vs", "res/shaders/BasicLit.fs");
	Shader colorShader("res/shaders/BasicLit.vs", "res/shaders/Color.fs");

	Model actor("res/models/nanosuit/nanosuit.obj");
	Model cube("res/models/cube/cube.obj");

	camera = new Camera(glm::vec3(0, 0, 3), 2.5f, width, height);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ProcessInput(window);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Basic Lit Shader (Phong)
		basicLitShader.Bind();
		basicLitShader.SetUniformMat4f("view", camera->GetView());
		basicLitShader.SetUniformMat4f("projection", camera->GetProjection());
		basicLitShader.SetUniform3f("viewPos", camera->GetPosition());

		basicLitShader.SetUniform1f("material.shininess", 32.0f);

		basicLitShader.SetUniform3f("directionalLight.direction", dirLightDirection);
		basicLitShader.SetUniform3f("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
		basicLitShader.SetUniform3f("directionalLight.diffuse", 0.5f, 0.5f, 0.5f);
		basicLitShader.SetUniform3f("directionalLight.specular", 1.0f, 1.0f, 1.0f);

		for (int i = 0; i < NUM_LIGHTS; ++i)
		{
			std::string iStr = std::to_string(i);
			basicLitShader.SetUniform3f("pointLights[" + iStr + "].position", pointLightPositions[i]);
			basicLitShader.SetUniform3f("pointLights[" + iStr + "].ambient", 0.2f, 0.2f, 0.2f);
			basicLitShader.SetUniform3f("pointLights[" + iStr + "].diffuse", pointLightColors[i]);
			basicLitShader.SetUniform3f("pointLights[" + iStr + "].specular", 1.0f, 1.0f, 1.0f);

			basicLitShader.SetUniform1f("pointLights[" + iStr + "].constant", 1.0f);
			basicLitShader.SetUniform1f("pointLights[" + iStr + "].linear", 0.09f);
			basicLitShader.SetUniform1f("pointLights[" + iStr + "].quadratic", 0.032f);
		}

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0, -2.0f, 0));
		model = glm::scale(model, glm::vec3(0.2f, 0.2, 0.2f));
		basicLitShader.SetUniformMat4f("model", model);
		actor.Draw(basicLitShader);

		basicLitShader.SetUniform3f("spotLight.direction", camera->GetForward());
		basicLitShader.SetUniform3f("spotLight.position", camera->GetPosition());
		basicLitShader.SetUniform3f("spotLight.ambient", 0.2f, 0.2f, 0.2f);
		basicLitShader.SetUniform3f("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
		basicLitShader.SetUniform3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
		basicLitShader.SetUniform1f("spotLight.cutoff", glm::cos(glm::radians(12.5f)));
		basicLitShader.SetUniform1f("spotLight.outerCutoff", glm::cos(glm::radians(17.5f)));


		colorShader.Bind();
		colorShader.SetUniformMat4f("view", camera->GetView());
		colorShader.SetUniformMat4f("projection", camera->GetProjection());
		for (int i = 0; i < NUM_LIGHTS; ++i)
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			colorShader.SetUniformMat4f("model", model);
			colorShader.SetUniform3f("emission", pointLightColors[i]);
			cube.Draw(colorShader);
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	delete camera;

}

int main(void)
{
	int result = RunApp();
	glfwTerminate();
	return result;
}

