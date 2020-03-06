#ifndef SYSTEM_H
#define SYSTEM_H

#include <unordered_map>

#include "Message.h"
#include "MessageBus.h"

class System;
class MessageBus;

typedef void (System::*message_method_t)(Message *);

class System
{
	public:
		void handleMessage(Message * msg)
		{
			auto action = messageActions.find(msg->type);

			if (action != messageActions.end())
				(this->*(action->second))(msg);
		}

		void setMessageCallback(MessageType type, message_method_t methodName)
		{
			messageActions[type] = methodName;
		}

        virtual void init() {};
		virtual void update(long elapsedTime) = 0;
		virtual ~System() {};

		MessageBus * msgBus;
		std::unordered_map<MessageType, message_method_t> messageActions;
};
#endif