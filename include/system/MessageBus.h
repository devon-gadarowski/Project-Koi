#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <chrono>
#include <queue>

#include "Message.h"
#include "System.h"

class MessageBus
{
	public:
        MessageBus()
        {
            auto now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
            auto epoch = now_ms.time_since_epoch();
            auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
            lastUpdateTime = value.count();
        }

        ~MessageBus()
        {
            destroySystems();
        }

        void update()
        {
            auto now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
            auto epoch = now_ms.time_since_epoch();
            auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
            long elapsedTime = value.count() - lastUpdateTime;
            lastUpdateTime = value.count();

            while (!(msgQueue.empty()))
            {
                Message * msg = msgQueue.front();
                sendMessageNow(msg);
                msgQueue.pop();
            }

            for (auto& system : registeredSystems)
                system->update(elapsedTime);
        }

        void registerSystem(System * s)
        {
            s->msgBus = this;

            registeredSystems.push_back(s);

            s->init();
        }

        void destroySystems()
        {
            for (auto& system : registeredSystems)
                delete system;

            registeredSystems.clear();
        }

		void sendMessage(MessageType type)
		{
			sendMessage(new EmptyMessage(type));
		}

        void sendMessage(MessageType type, void * data)
        {
            sendMessage(new PointerMessage(type, data));
        }

        void sendMessage(MessageType type, int data)
        {
            sendMessage(new IntegerMessage(type, data));
        }

        void sendMessage(MessageType type, std::string & str)
        {
            sendMessage(new StringMessage(type, str));
        }

        void sendMessage(MessageType type, float * data, int length)
        {
            sendMessage(new VectorMessage(type, data, length));
        }

        void sendMessage(Message * msg)
		{
			msgQueue.push(msg);
		}

        void sendMessageNow(MessageType type)
		{
			sendMessageNow(new EmptyMessage(type));
		}

        void sendMessageNow(MessageType type, void * data)
        {
            sendMessageNow(new PointerMessage(type, data));
        }

        void sendMessageNow(MessageType type, int data)
        {
            sendMessageNow(new IntegerMessage(type, data));
        }

        void sendMessageNow(MessageType type, std::string & str)
        {
            sendMessageNow(new StringMessage(type, str));
        }

        void sendMessageNow(MessageType type, float * data, int length)
        {
            sendMessageNow(new VectorMessage(type, data, length));
        }

		void sendMessageNow(Message * msg)
		{
			if (msg->type == Exit)
			{
				this->needsDestroying = true;
			}

			for (auto& system : registeredSystems)
				system->handleMessage(msg);

            delete msg;
		}

		bool needsDestroying = false;

	private:
		std::queue<Message *> msgQueue;
		std::vector<System *> registeredSystems;

		long lastUpdateTime;
};
#endif