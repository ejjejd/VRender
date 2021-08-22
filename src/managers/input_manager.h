#pragma once
#include <bitset>

#include "vrender.h"
#include "vulkan/vulkan_app.h"
#include "input/input_map.h"

namespace manager
{
	constexpr size_t InputMangerKeysStateLength = GLFW_KEY_LAST + 1;
	constexpr size_t InputManagerButtonsStateLength = GLFW_MOUSE_BUTTON_LAST + 1;
	constexpr size_t InputManagerGestureStateLength = (size_t)input::Gesture::Last;

	template<size_t Size>
	struct State
	{
		std::bitset<Size> CurrentState;
		std::bitset<Size> LastState;
		std::bitset<Size> ChangedToOne;
		std::bitset<Size> ChangedToZero;

		inline void ClearChangedStates()
		{
			ChangedToOne.reset();
			ChangedToZero.reset();
		}
	};

	class InputManager
	{
	private:
		State<InputMangerKeysStateLength> KeysState;
		State<InputManagerButtonsStateLength> ButtonsState;
		std::bitset<InputManagerGestureStateLength> GestruresState;

		glm::vec2 CurrentCursorPosition;
		glm::vec2 LastCursorPosition;
		glm::vec2 CurrentCursorOffset;

		vk::VulkanApp* VulkanApp;

		void UpdateGestures();
	public:
		void Setup(vk::VulkanApp& app);

		void Update();

		inline bool IsKeyPressed(const input::Key key) const
		{
			size_t keyId = (size_t)key;

			if (keyId >= InputMangerKeysStateLength)
			{
				LOGE("You trying to check invalid key!");
				return false;
			}

			return KeysState.ChangedToOne[keyId];
		}

		inline bool IsKeyReleased(const input::Key key) const
		{
			size_t keyId = (size_t)key;

			if (keyId >= InputMangerKeysStateLength)
			{
				LOGE("You trying to check invalid key!");
				return false;
			}

			return KeysState.ChangedToZero[keyId];
		}


		inline bool IsKeyStillPressed(const input::Key key) const
		{
			size_t keyId = (size_t)key;

			if (keyId >= InputMangerKeysStateLength)
			{
				LOGE("You trying to check invalid key!");
				return false;
			}

			return KeysState.CurrentState[keyId];
		}

		inline bool IsButtonPressed(const input::Button button) const
		{
			size_t buttonId = (size_t)button;

			if (buttonId >= InputManagerButtonsStateLength)
			{
				LOGE("You trying to check invalid button!");
				return false;
			}

			return ButtonsState.ChangedToOne[buttonId];
		}

		inline bool IsButtonReleased(const input::Button button) const
		{
			size_t buttonId = (size_t)button;

			if (buttonId >= InputManagerButtonsStateLength)
			{
				LOGE("You trying to check invalid button!");
				return false;
			}

			return ButtonsState.ChangedToZero[buttonId];
		}

		inline bool IsButtonStillPressed(const input::Button button) const
		{
			size_t buttonId = (size_t)button;

			if (buttonId >= InputManagerButtonsStateLength)
			{
				LOGE("You trying to check invalid button!");
				return false;
			}
			
			return ButtonsState.CurrentState[buttonId];
		}

		inline bool IsGesturePerformed(const input::Gesture gesture) const
		{
			size_t gestureId = (size_t)gesture;

			if (gestureId >= InputManagerGestureStateLength)
			{
				LOGE("You trying to check invalid gesture!");
				return false;
			}

			return GestruresState[gestureId];
		}

		inline glm::vec2 GetCursorPositions() const
		{
			return CurrentCursorPosition;
		}

		inline glm::vec2 GetCursorOffset() const
		{
			return CurrentCursorOffset;
		}
	};
}