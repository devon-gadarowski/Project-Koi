#ifndef SYSTEM_H
#define SYSTEM_H

#include <queue>
#include <vector>
#include <unordered_map>

#include <system/Message.h>

class System;
class MessageBus;

typedef void (System::*message_method_t)(Message *);

class MessageBus
{
	public:
    MessageBus() {}
    virtual ~MessageBus() {}

    void registerSystem(System * s);
    void destroySystems();

    void sendMessage(MessageType type);
    void sendMessage(MessageType type, void * data);
    void sendMessage(MessageType type, int data);
    void sendMessage(MessageType type, std::string str);
    void sendMessage(MessageType type, float * data, int length);
    void sendMessage(Message * msg);

    void sendMessageNow(MessageType type);
    void sendMessageNow(MessageType type, void * data);
    void sendMessageNow(MessageType type, int data);
    void sendMessageNow(MessageType type, std::string str);
    void sendMessageNow(MessageType type, float * data, int length);
    void sendMessageNow(Message * msg);

	protected:
    std::queue<Message *> msgQueue;
    std::vector<System *> registeredSystems;
};

class System
{
	public:
	System() {}
	virtual ~System() {};

	virtual void handleMessage(Message * msg);
	void setMessageCallback(MessageType type, message_method_t methodName);

	virtual void init() {}
	virtual void update(long elapsedTime) = 0;

	MessageBus * app = nullptr;
	std::unordered_map<MessageType, message_method_t> messageActions;
};

#endif