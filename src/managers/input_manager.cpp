#include "input_manager.h"

namespace manager
{
	std::function<void(GLFWwindow*, int, int, int, int)> g_KeyCallback;
	std::function<void(GLFWwindow*, int, int, int)> g_ButtonCallback;
	std::function<void(GLFWwindow*, double, double)> g_CursorPosCallback;

	namespace glfw_callack
	{
		void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if(g_KeyCallback)
				g_KeyCallback(window, key, scancode, action, mods);
		}

		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
		{
			if (g_ButtonCallback)
				g_ButtonCallback(window, button, action, mod);
		}

		void CursorPosCallback(GLFWwindow* window, double xPos, double yPos)
		{
			if (g_CursorPosCallback)
				g_CursorPosCallback(window, xPos, yPos);
		}
	}

	void InputManager::Setup(vk::VulkanApp& app)
	{
		VulkanApp = &app;

		glfwSetKeyCallback(app.GlfwWindow, glfw_callack::KeyCallback);
		glfwSetMouseButtonCallback(app.GlfwWindow, glfw_callack::MouseButtonCallback);
		glfwSetCursorPosCallback(app.GlfwWindow, glfw_callack::CursorPosCallback);

		g_KeyCallback = 
			[&](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				if(key >= KeysState.CurrentState.size())
				{
					LOGE("Invalid keys state length set!");
					return;
				}

				if (action == GLFW_PRESS)
					KeysState.CurrentState[key] = true;
				else if (action == GLFW_RELEASE)
					KeysState.CurrentState[key] = false;
			};
		g_ButtonCallback = 
			[&](GLFWwindow* window, int button, int action, int mod)
			{
				if (button >= ButtonsState.CurrentState.size())
				{
					LOGE("Invalid buttons state length set!");
					return;
				}

				if (action == GLFW_PRESS)
					ButtonsState.CurrentState[button] = true;
				else if (action == GLFW_RELEASE)
					ButtonsState.CurrentState[button] = false;
			};
		g_CursorPosCallback = 
			[&](GLFWwindow* window, double xPos, double yPos)
			{	
				CurrentCursorPosition = glm::vec2(xPos, yPos);
			};

		double posX, posY;
		glfwGetCursorPos(app.GlfwWindow, &posX, &posY);
		CurrentCursorPosition = glm::vec2(posX, posY);
		LastCursorPosition = CurrentCursorPosition;
	}

	void InputManager::UpdateGestures()
	{
		CurrentCursorOffset = CurrentCursorPosition - LastCursorPosition;

		if (CurrentCursorOffset.x != 0.0f)
			GestruresState[(size_t)input::Gesture::MouseX] = true;
		else
			GestruresState[(size_t)input::Gesture::MouseX] = false;
		if (CurrentCursorOffset.y != 0.0f)
			GestruresState[(size_t)input::Gesture::MouseY] = true;
		else
			GestruresState[(size_t)input::Gesture::MouseY] = false;

		LastCursorPosition = CurrentCursorPosition;
	}

	void InputManager::Update()
	{
		KeysState.ClearChangedStates();
		ButtonsState.ClearChangedStates();

		if (KeysState.CurrentState != KeysState.LastState)
		{
			auto changedKeys = KeysState.CurrentState ^ KeysState.LastState;
			KeysState.ChangedToOne = changedKeys & KeysState.CurrentState;
			KeysState.ChangedToZero = changedKeys & ~KeysState.CurrentState;
		}
		if (ButtonsState.CurrentState != ButtonsState.LastState)
		{
			auto changedButtons = ButtonsState.CurrentState ^ ButtonsState.LastState;
			ButtonsState.ChangedToOne = changedButtons & ButtonsState.CurrentState;
			ButtonsState.ChangedToZero = changedButtons & ~ButtonsState.CurrentState;
		}

		KeysState.LastState = KeysState.CurrentState;
		ButtonsState.LastState = ButtonsState.CurrentState;

		UpdateGestures();
	}
}