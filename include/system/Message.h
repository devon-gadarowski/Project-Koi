#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>

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
	SetCameraDirection,
	SetLighting,
	GetModelData,
	AddModel
};

class Message
{
    public:
        MessageType type;

		Message(MessageType type)
		{
			this->type = type;
		}

		virtual ~Message() {};
};

class EmptyMessage : public Message
{
	public:
		EmptyMessage(MessageType type) : Message(type) {}
		~EmptyMessage() {}
};

class StringMessage : public Message
{
	public:
		StringMessage(MessageType type, std::string & data) : Message(type)
		{
			this->data = data;
		}

		~StringMessage() {}

        std::string data;
};

class IntegerMessage : public Message
{
	public:
		IntegerMessage(MessageType type, int data) : Message(type)
		{
			this->data = data;
		}

		~IntegerMessage() {}
		
        int data;
};

class PointerMessage : public Message
{
	public:
		PointerMessage(MessageType type, void * data) : Message(type)
		{
			this->data = data;
		}

		~PointerMessage() {}
		
        void * data;
};

class VectorMessage : public Message
{
	public:
		VectorMessage(MessageType type, float * data, int length) : Message(type)
		{
			this->data = (float *) malloc(sizeof(float) * length);
			this->length = length;

			for (int i = 0; i < length; i++)
				this->data[i] = data[i];
		}

		~VectorMessage()
		{
			free(data);
		}

	int length;
	float * data;
};

#endif