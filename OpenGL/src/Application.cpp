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
#include <vector>

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

bool showOutline = false;
bool drawTransparentWindows = true;

constexpr int NUM_LIGHTS = 4;
glm::vec3 dirLightDirection(-0.2f, -1.0f, -0.3f);

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

std::vector<glm::vec3> windows = {
	glm::vec3(-1.5f, 0.0f, -0.48f),
	glm::vec3(1.5f, 0.0f, 0.51f),
	glm::vec3(0.0f, 0.0f, 0.7f),
	glm::vec3(-0.3f, 0.0f, -2.3f),
	glm::vec3(0.5f, 0.0f, -0.6f)
};


std::vector<Vertex> quadVertices = {
	// Position		Normal(unused), UV
	{{-1.0f, 1.0f, 0}, {0,0,0},	{0, 1.0f}},
	{{-1.0f, -1.0f, 0}, {0,0,0}, {0, 0}},
	{{1.0f, 1.0f, 0}, {0,0,0}, {1, 1} },
	{{1.0f, -1.0f, 0},{0,0,0}, {1, 0}}
};

std::vector<unsigned int> quadIndices = {
	0, 1, 2,
	2, 1, 3
};

glm::vec4 clearColor(0.1f, 0.1f, 0.1f, 1.0f);

Shader* basicLitShader = nullptr;
Shader* colorShader = nullptr;
Shader* spriteShader = nullptr;
Shader* postProcessShader = nullptr;

Model* actor	= nullptr;
Model* cube		= nullptr;
Model* plane	= nullptr;

Texture* windowTexture = nullptr;

Mesh* screenQuad = nullptr;

unsigned int fbo, fboColorBuffer, rbo;

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

void DrawScene()
{
	// Phong shading information
	basicLitShader->Bind();
	basicLitShader->SetUniformMat4f("view", camera->GetView());
	basicLitShader->SetUniformMat4f("projection", camera->GetProjection());
	basicLitShader->SetUniform3f("viewPos", camera->GetPosition());
				  
	basicLitShader->SetUniform1f("material.shininess", 32.0f);
				  
	basicLitShader->SetUniform3f("directionalLight.direction", dirLightDirection);
	basicLitShader->SetUniform3f("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
	basicLitShader->SetUniform3f("directionalLight.diffuse", 0.5f, 0.5f, 0.5f);
	basicLitShader->SetUniform3f("directionalLight.specular", 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		std::string iStr = std::to_string(i);
		basicLitShader->SetUniform3f("pointLights[" + iStr + "].position", pointLightPositions[i]);
		basicLitShader->SetUniform3f("pointLights[" + iStr + "].ambient", 0.2f, 0.2f, 0.2f);
		basicLitShader->SetUniform3f("pointLights[" + iStr + "].diffuse", pointLightColors[i]);
		basicLitShader->SetUniform3f("pointLights[" + iStr + "].specular", 1.0f, 1.0f, 1.0f);
					  
		basicLitShader->SetUniform1f("pointLights[" + iStr + "].constant", 1.0f);
		basicLitShader->SetUniform1f("pointLights[" + iStr + "].linear", 0.09f);
		basicLitShader->SetUniform1f("pointLights[" + iStr + "].quadratic", 0.032f);
	}

	basicLitShader->SetUniform3f("spotLight.direction", camera->GetForward());
	basicLitShader->SetUniform3f("spotLight.position", camera->GetPosition());
	basicLitShader->SetUniform3f("spotLight.ambient", 0.2f, 0.2f, 0.2f);
	basicLitShader->SetUniform3f("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
	basicLitShader->SetUniform3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
	basicLitShader->SetUniform1f("spotLight.cutoff", glm::cos(glm::radians(12.5f)));
	basicLitShader->SetUniform1f("spotLight.outerCutoff", glm::cos(glm::radians(17.5f)));

	if (showOutline)
	{
		// Outline effect!
		glEnable(GL_STENCIL_TEST);
		// glStencilOp determines what to do based on when the buffer test passes
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // if the test passes, replace value with ref below in glStencilFunc(which is 1)
		// glStencilFunc determines when a stencil buffer test passes
		glStencilFunc(GL_ALWAYS, 1, 0xff); // the test always passes
		// glStencilMask's argument is ANDed with the value of the stencil buffer
		glStencilMask(0xff); // enable the stencil buffer
	}

	// Draw the actor(s)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0, -1.5f, 0));
		model = glm::scale(model, glm::vec3(0.2f, 0.2, 0.2f));
		basicLitShader->SetUniformMat4f("model", model);
		actor->Draw(*basicLitShader);


		if (showOutline)
		{
			// Finish the outline effect
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0x00); // No writing to stencil buffer
			glDisable(GL_DEPTH_TEST);
			colorShader->Bind();
			colorShader->SetUniformMat4f("view", camera->GetView());
			colorShader->SetUniformMat4f("projection", camera->GetProjection());
			colorShader->SetUniformMat4f("model", glm::scale(model, glm::vec3(1.02f, 1.02f, 1.02f)));
			colorShader->SetUniform3f("emission", 0.0f, 0.0, 1.0f);
			actor->Draw(*colorShader);

			glEnable(GL_DEPTH_TEST);
			glStencilMask(0xff);
		}
	}

	// Windows
	if (drawTransparentWindows)
	{
		glDisable(GL_CULL_FACE); // We want windows visible from both angles!

		spriteShader->Bind();
		spriteShader->SetUniformMat4f("view", camera->GetView());
		spriteShader->SetUniformMat4f("projection", camera->GetProjection());
		spriteShader->SetUniform1i("diffuse", 0);
		windowTexture->Bind(0);

		// We need to draw transparent objects from furthest away to nearest because
		// the depth buffer can't help us here
		std::map<float, glm::vec3> sortedWindows;
		for (int i = 0; i < windows.size(); ++i)
		{
			float distance = glm::length(camera->GetPosition() - windows[i]);
			sortedWindows[distance] = windows[i];
		}

		for (std::map<float, glm::vec3>::reverse_iterator it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it)
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, it->second);
			spriteShader->SetUniformMat4f("model", model);
			plane->Draw(*spriteShader);
		}

		glEnable(GL_CULL_FACE);

	}


	// Light visualizers
	{
		colorShader->Bind();
		colorShader->SetUniformMat4f("view", camera->GetView());
		colorShader->SetUniformMat4f("projection", camera->GetProjection());
		for (int i = 0; i < NUM_LIGHTS; ++i)
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			colorShader->SetUniformMat4f("model", model);
			colorShader->SetUniform3f("emission", pointLightColors[i]);
			cube->Draw(*colorShader);
		}
	}
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

	std::cout << glGetString(GL_VERSION) << "\n";

	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_CULL_FACE);

	// Frame buffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create an empty texture and attach it to the frame buffer as a color buffer
	glGenTextures(1, &fboColorBuffer);
	glBindTexture(GL_TEXTURE_2D, fboColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorBuffer, 0);

	// Render buffers - like textures but faster (and write-only)
	// Create a render buffer and attach it as a combo depth/stencil buffer
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete";
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create the fullscreen quad resources
	screenQuad = new Mesh(quadVertices, quadIndices, std::vector<Texture*>());

	basicLitShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/BasicLit.fs");
	colorShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/Color.fs");
	spriteShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/Sprite.fs");
	postProcessShader = new Shader("res/shaders/PostProcess.vs", "res/shaders/EdgeDetection.fs");

	actor = new Model("res/models/nanosuit/nanosuit.obj");
	cube = new Model("res/models/cube/cube.obj");
	plane = new Model("res/models/plane/plane.obj");

	windowTexture = new Texture("res/textures/blending_transparent_window.png", aiTextureType_DIFFUSE);

	camera = new Camera(glm::vec3(0, 0, 3), 2.5f, width, height);

	while (!glfwWindowShouldClose(window))
	{

		ProcessInput(window);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// First Pass
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		DrawScene();

		// Second Pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // use the regular frame buffer now
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); // As we're just rendering a quad, color is the only thing that needs to be cleared!
		postProcessShader->Bind();
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fboColorBuffer);
		screenQuad->Draw(*postProcessShader);


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	delete camera;

	// TODO delete all buffers and heap allocated memory!
	glDeleteFramebuffers(1, &fbo);
}

int main(void)
{
	int result = RunApp();
	glfwTerminate();
	return result;
}

