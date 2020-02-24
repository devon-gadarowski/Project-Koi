
#include "SystemFramework.h"

MessageBus::MessageBus()
{
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto epoch = now_ms.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	lastUpdateTime = value.count();
}

MessageBus::~MessageBus()
{
	destroySystems();
}

void MessageBus::sendMessage(Message msg)
{
	msgQueue.push(msg);
}

void MessageBus::sendMessageNow(Message msg)
{
	if (msg.type == Exit)
	{
		this->needsDestroying = true;
	}

	for (auto& system : registeredSystems)
		system->handleMessage(msg);
}

void MessageBus::update()
{
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto epoch = now_ms.time_since_epoch();
	auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
	long elapsedTime = value.count() - lastUpdateTime;
	lastUpdateTime = value.count();

	for (auto& system : registeredSystems)
		system->update(elapsedTime);

	while (!(msgQueue.empty()))
	{
		sendMessageNow(msgQueue.front());
		msgQueue.pop();
	}
}

void MessageBus::registerSystem(System * s)
{
	s->msgBus = this;

	registeredSystems.push_back(s);

	this->sendMessageNow(Message(SystemRegistered, s));
}

void MessageBus::destroySystems()
{
	for (auto& system : registeredSystems)
		delete system;

	registeredSystems.clear();
}
