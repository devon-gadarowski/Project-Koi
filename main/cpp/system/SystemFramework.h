
#ifndef SYSTEM_FRAMEWORK_H
#define SYSTEM_FRAMEWORK_H

#include <Log.h>

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <chrono>

class Message;
class MessageBus;
class System;

enum MessageType
{
	Initialize,
	Shutdown,
	Update,
	Exit,
	SystemRegistered,
	ConsolePause,
	ConsoleResume,
	GLFWwindowCreated,
	LoadScene,
	SceneLoaded,
	SceneDestroyed,
	KeyPress,
	KeyRelease,
	SetMouseCursor,
	SetMouseDelta,
	MouseButtonPress,
	MouseButtonRelease,
	SetWindowFocus,
	SetCameraPosition,
	SetCameraDirection
};

class Message
{
	public:
		Message(MessageType t)
		{
			type = t;
			data = nullptr;
		}

		Message(MessageType t, void * d)
		{
			type = t;
			data = d;
		}

		MessageType type;
		void * data;
};

class MessageBus
{
	public:
		MessageBus();
		~MessageBus();

		void sendMessage(Message msg);
		void sendMessageNow(Message msg);
		void update();

		void registerSystem(System * s);
		void destroySystems();

		bool needsDestroying = false;

	private:
		std::queue<Message> msgQueue;
		std::vector<System *> registeredSystems;

		long lastUpdateTime;
};

typedef void (System::*message_method_t)(void *);

class System
{
	public:
		void handleMessage(Message msg)
		{
			auto action = messageActions.find(msg.type);

			if (action != messageActions.end())
				(this->*(action->second))(msg.data);
		}

		void setMessageCallback(MessageType type, message_method_t methodName)
		{
			messageActions[type] = methodName;
		}

		virtual void update(long elapsedTime) = 0;
		virtual ~System() {};

		MessageBus * msgBus;
		std::unordered_map<MessageType, message_method_t> messageActions;
};

#endif
