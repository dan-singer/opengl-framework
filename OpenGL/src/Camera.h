#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
	glm::mat4 m_Projection;
	glm::mat4 m_View;
	float m_Pitch = 0.0f;
	float m_Yaw = 0.0f;
	float m_Fov = 45.0f;
	float m_Speed;
	int m_Width;
	int m_Height;
	glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_Forward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
public:
	Camera(glm::vec3 position, float speed, int screenWidth, int screenHeight) :
		m_Speed(speed), m_Width(screenWidth), m_Height(screenHeight)
	{
		RecalculateProjection();
		RecalculateDirection();
	}
	glm::mat4 GetProjection() { return m_Projection; }
	glm::mat4 GetView() { return m_View; }

	glm::vec3 GetPosition() { return m_Position; }
	glm::vec3 SetPosition(glm::vec3 pos) 
	{ 
		m_Position = pos; 
		RecalculateDirection(); 
	}

	glm::vec3 GetForward() { return m_Forward; }
	glm::vec3 GetUp() { return m_Up; }

	float GetFOV() { return m_Fov; }
	void AdjustFOV(float delta)
	{
		m_Fov += delta;
		RecalculateProjection();
	}
	void SetFOV(float fov)
	{
		m_Fov = fov;
		RecalculateProjection();
	}

	float GetYaw() { return m_Yaw; }
	void SetYaw(float yaw) 
	{ 
		m_Yaw = yaw; 
		RecalculateDirection();
	}

	float GetPitch() { return m_Pitch; }
	void SetPitch(float Pitch)
	{
		m_Pitch = Pitch;
		RecalculateDirection();
	}

	void RecalculateDirection()
	{
		glm::quat q(glm::vec3(glm::radians(m_Pitch), glm::radians(m_Yaw), 0.0f));
		m_Forward = glm::normalize(q * glm::vec3(0, 0, -1.0f));
		m_Up = glm::normalize(q * glm::vec3(0, 1.0f, 0));
		m_View = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
	}

	void RecalculateProjection()
	{
		m_Projection = glm::perspective(glm::radians(m_Fov), (float)m_Width / m_Height, 0.01f, 100.0f);
	}


	float GetSpeed() { return m_Speed; }

	void Translate(glm::vec3 delta)
	{
		m_Position += delta;
		RecalculateDirection();
	}


};