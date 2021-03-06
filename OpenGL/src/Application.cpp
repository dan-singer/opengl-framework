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
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"

#include <assimp/Importer.hpp>
#include <vector>
#include "vendor/stb_image/stb_image.h"

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
float normalLength = 0.1f;

bool showOutline = false;
bool drawTransparentWindows = false;
bool usePostProcessing = false;
bool showNormals = true;


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
Shader* skyboxShader = nullptr;
Shader* normalShader = nullptr;

Model* actor	= nullptr;
Model* cube		= nullptr;
Model* plane	= nullptr;

Texture* windowTexture = nullptr;

Mesh* screenQuad = nullptr;

unsigned int fbo, fboColorBuffer, rbo, uboMatrices;

unsigned int cubemap;
std::vector<const char*> cubeMapPaths =
{
	"res/textures/skybox/right.jpg",
	"res/textures/skybox/left.jpg",
	"res/textures/skybox/top.jpg",
	"res/textures/skybox/bottom.jpg",
	"res/textures/skybox/front.jpg",
	"res/textures/skybox/back.jpg",
};

unsigned int loadCubemap(const std::vector<const char*>& paths)
{
	unsigned int cubemap;
	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	int width, height, numChannels;
	unsigned char* data;
	for (unsigned int i = 0; i < cubeMapPaths.size(); ++i)
	{
		data = stbi_load(cubeMapPaths[i], &width, &height, &numChannels, 0);
		if (data)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << paths[i] << std::endl;
			stbi_image_free(data);
		}

	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubemap;
}


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
	// Set Uniform buffer object data
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	glm::mat4 projection = camera->GetProjection();
	glm::mat4 view = camera->GetView();

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	// Phong shading information
	basicLitShader->Bind();
				  
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
		basicLitShader->SetUniform1f("material.reflectivity", 0.8f);
		actor->Draw(*basicLitShader, cubemap);


		if (showOutline)
		{
			// Finish the outline effect
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0x00); // No writing to stencil buffer
			glDisable(GL_DEPTH_TEST);
			colorShader->Bind();
			colorShader->SetUniformMat4f("model", glm::scale(model, glm::vec3(1.02f, 1.02f, 1.02f)));
			colorShader->SetUniform3f("emission", 0.0f, 0.0, 1.0f);
			actor->Draw(*colorShader);

			glEnable(GL_DEPTH_TEST);
			glStencilMask(0xff);
		}

		if (showNormals)
		{
			normalShader->Bind();
			normalShader->SetUniformMat4f("model", model);
			normalShader->SetUniform1f("normalLength", normalLength);
			actor->Draw(*normalShader);
		}
	}

	// Windows
	if (drawTransparentWindows)
	{
		glDisable(GL_CULL_FACE); // We want windows visible from both angles!

		spriteShader->Bind();
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

	// Skybox - render last to prevent overdraw
	{
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		skyboxShader->Bind();
		skyboxShader->SetUniformMat4f("projection", camera->GetProjection());
		glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(camera->GetView()));
		skyboxShader->SetUniformMat4f("view", viewNoTranslation);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
		cube->Draw(*skyboxShader);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
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
	glfwSwapInterval(0);

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

	cubemap = loadCubemap(cubeMapPaths);


	basicLitShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/BasicLit.fs");
	colorShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/Color.fs");
	spriteShader = new Shader("res/shaders/BasicLit.vs", "res/shaders/Sprite.fs");
	postProcessShader = new Shader("res/shaders/PostProcess.vs", "res/shaders/EdgeDetection.fs");
	skyboxShader = new Shader("res/shaders/Skybox.vs", "res/shaders/Skybox.fs");
	// This one uses a geometry shader
	normalShader = new Shader("res/shaders/Normals.vs", "res/shaders/Normals.fs", "res/shaders/Normals.gs");

	// Uniform Buffer Object Setup
	unsigned int ubiBasicLit = glGetUniformBlockIndex(basicLitShader->GetID(), "Matrices");
	unsigned int ubiColor = glGetUniformBlockIndex(colorShader->GetID(), "Matrices");
	unsigned int ubiSprite = glGetUniformBlockIndex(spriteShader->GetID(), "Matrices");
	unsigned int ubiNormal = glGetUniformBlockIndex(normalShader->GetID(), "Matrices");

	glUniformBlockBinding(basicLitShader->GetID(), ubiBasicLit, 0);
	glUniformBlockBinding(colorShader->GetID(), ubiColor, 0);
	glUniformBlockBinding(spriteShader->GetID(), ubiSprite, 0);
	glUniformBlockBinding(normalShader->GetID(), ubiNormal, 0);


	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Link the uniform buffer to the binding point 0
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));


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
		if (usePostProcessing)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glEnable(GL_DEPTH_TEST);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		DrawScene();

		if (usePostProcessing)
		{
			// Second Pass
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // use the regular frame buffer now
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT); // As we're just rendering a quad, color is the only thing that needs to be cleared!
			postProcessShader->Bind();
			glDisable(GL_DEPTH_TEST);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fboColorBuffer);
			screenQuad->Draw(*postProcessShader, 0);
		}



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

/* Refactoring List
	- Cubemap
	- Framebuffer
	- Light Classes
*/