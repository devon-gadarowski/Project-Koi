#include <system/System.h>

void System::handleMessage(Message * msg)
{
	auto action = messageActions.find(msg->type);

	if (action != messageActions.end())
		(this->*(action->second))(msg);
}

void System::setMessageCallback(MessageType type, message_method_t methodName)
{
	messageActions[type] = methodName;
}

void MessageBus::registerSystem(System * s)
{
    s->app = this;

    registeredSystems.push_back(s);

    s->init();
}

void MessageBus::destroySystems()
{
    for (auto& system : registeredSystems)
        delete system;

    registeredSystems.clear();
}

void MessageBus::sendMessage(MessageType type)
{
    sendMessage(new EmptyMessage(type));
}

void MessageBus::sendMessage(MessageType type, void * data)
{
    sendMessage(new PointerMessage(type, data));
}

void MessageBus::sendMessage(MessageType type, int data)
{
    sendMessage(new IntegerMessage(type, data));
}

void MessageBus::sendMessage(MessageType type, std::string str)
{
    sendMessage(new StringMessage(type, str));
}

void MessageBus::sendMessage(MessageType type, float * data, int length)
{
    sendMessage(new VectorMessage(type, data, length));
}

void MessageBus::sendMessage(Message * msg)
{
    msgQueue.push(msg);
}

void MessageBus::sendMessageNow(MessageType type)
{
    sendMessageNow(new EmptyMessage(type));
}

void MessageBus::sendMessageNow(MessageType type, void * data)
{
    sendMessageNow(new PointerMessage(type, data));
}

void MessageBus::sendMessageNow(MessageType type, int data)
{
    sendMessageNow(new IntegerMessage(type, data));
}

void MessageBus::sendMessageNow(MessageType type, std::string str)
{
    sendMessageNow(new StringMessage(type, str));
}

void MessageBus::sendMessageNow(MessageType type, float * data, int length)
{
    sendMessageNow(new VectorMessage(type, data, length));
}

void MessageBus::sendMessageNow(Message * msg)
{
    for (auto& system : registeredSystems)
        system->handleMessage(msg);

    delete msg;
}