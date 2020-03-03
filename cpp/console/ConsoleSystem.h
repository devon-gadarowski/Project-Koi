
#ifndef CONSOLE_SYSTEM_H
#define CONSOLE_SYSTEM_H

#include <SystemFramework.h>

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
		void start(void * data);
		void stop(void * data);
		void pause(void * data);
		void resume(void * data);

	private:
		std::thread * console = nullptr;
		bool consolePause = false;
		std::unordered_map<long, command_method_t> commands;

		void registerCommands();
		void readConsoleInputs();
		void processCommand(void * data);

		static std::vector<std::string> parseCommandArgs(std::string);
		static long hashCode(std::string);

		// Command Events
		void init(std::vector<std::string> args);
		void shutdown(std::vector<std::string> args);
		void exit(std::vector<std::string> args);
		void loadScene(std::vector<std::string> args);
};

#endif
