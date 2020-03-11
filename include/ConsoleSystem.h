
#ifndef CONSOLE_SYSTEM_H
#define CONSOLE_SYSTEM_H

#include <system/Log.h>
#include <system/System.h>

#include <thread>

class ConsoleSystem;

typedef void (ConsoleSystem::*console_method_t)();
typedef void (ConsoleSystem::*command_method_t)(std::vector<std::string> args);

class ConsoleSystem : public System
{
	public:
		void update(long elapsedTime);

		ConsoleSystem();
		~ConsoleSystem();

		// Message Events
		void processCommand(Message * msg);


	private:
		std::thread * console = nullptr;
		bool consolePause = false;
		std::unordered_map<long, command_method_t> commands;

		void registerCommands();

		static std::vector<std::string> parseCommandArgs(std::string);
		static long hashCode(std::string);

		// Command Events
		void exit(std::vector<std::string> args);
		void loadScene(std::vector<std::string> args);
};

#endif
