
#include <ConsoleSystem.h>

#include <iostream>
#include <string>

#include <string.h>

void ConsoleSystem::update(long elapsedTime)
{

}

ConsoleSystem::ConsoleSystem()
{
	DEBUG("CONSOLE_SYSTEM - ConsoleSystem created");
	//setMessageCallback(SystemRegistered, (message_method_t) &ConsoleSystem::start);
	setMessageCallback(ConsolePause, (message_method_t) &ConsoleSystem::pause);
	setMessageCallback(ConsoleResume, (message_method_t) &ConsoleSystem::resume);

	setMessageCallback(ProcessCommand, (message_method_t) &ConsoleSystem::processCommand);

	registerCommands();
}

ConsoleSystem::~ConsoleSystem()
{
	stop(nullptr);
	DEBUG("CONSOLE_SYSTEM - ConsoleSystem Destroyed");
}

void ConsoleSystem::start(void * data)
{
	if (data != this)
		return;

	if (console != nullptr)
	{
		WARN("CONSOLE_SYSTEM - Console already running");
		return;
	}

	console = new std::thread(&ConsoleSystem::readConsoleInputs, this);
	DEBUG("CONSOLE_SYSTEM - Console started");
}

void ConsoleSystem::stop(void * data)
{
	if (console == nullptr)
	{
		WARN("CONSOLE_SYSTEM - No console currently running");
		return;
	}

	console->detach();
	delete console;
	console = nullptr;

	DEBUG("CONSOLE_SYSTEM - Console stopped");
}

void ConsoleSystem::pause(void * data)
{
	consolePause = true;
}

void ConsoleSystem::resume(void * data)
{
	consolePause = false;
}

void ConsoleSystem::registerCommands()
{
	commands[hashCode("init")] = &ConsoleSystem::init;
	commands[hashCode("initialize")] = &ConsoleSystem::init;
	commands[hashCode("shutdown")] = &ConsoleSystem::shutdown;
	commands[hashCode("stop")] = &ConsoleSystem::shutdown;
	commands[hashCode("exit")] = &ConsoleSystem::exit;
	commands[hashCode("loadscene")] = &ConsoleSystem::loadScene;
	commands[hashCode("load")] = &ConsoleSystem::loadScene;
}

void ConsoleSystem::readConsoleInputs()
{
	std::string command;
	while (!msgBus->needsDestroying)
	{
		getline(std::cin, command);

		processCommand(&command);

		consolePause = true;
		msgBus->sendMessage(Message(ConsoleResume));

		while (consolePause)
		{
			// Wait for command to process
		}
	}
}

void ConsoleSystem::processCommand(void * data)
{
	std::string command = *((std::string *) data);

	if (command.length() == 0)
		return;

	std::vector<std::string> args = parseCommandArgs(command);

	long hash = hashCode(args[0]);
	auto action = commands.find(hash);

	if (action != commands.end())
		(this->*(action->second))(args);
}

std::vector<std::string> ConsoleSystem::parseCommandArgs(std::string command)
{
	std::vector<std::string> args = {};

	int i = 0;
	int size = 0;
	while (i < command.length())
	{
		args.push_back(std::string());
		int j = 0;
		while (i < command.length())
		{
			if (command[i] == ' ' || command[i] == '\n')
			{
				i++;
				break;
			}

			args[size].push_back(command[i++]);
		}
		size++;
	}

	return args;
}

long ConsoleSystem::hashCode(std::string command)
{
	long hash = 0;
	long powah = 31;
	for (int i = 0; i < command.length(); i++)
		hash = hash * powah + command[i];

	return hash;
}

void ConsoleSystem::init(std::vector<std::string> args)
{

	if (args.size() < 2)
		msgBus->sendMessage(Message(Initialize));
	else
		msgBus->sendMessage(Message(Initialize, &args[1]));
}

void ConsoleSystem::shutdown(std::vector<std::string> args)
{
	if (args.size() < 2)
		msgBus->sendMessage(Message(Shutdown));
	else
		msgBus->sendMessage(Message(Shutdown, &args[1]));
}

void ConsoleSystem::exit(std::vector<std::string> args)
{
	msgBus->sendMessage(Message(Shutdown));
	msgBus->sendMessage(Message(Exit));
}

void ConsoleSystem::loadScene(std::vector<std::string> args)
{
	if (args.size() < 2)
		return;

	args[1].append(".txt");

	msgBus->sendMessageNow(Message(LoadScene, &args[1]));
}
