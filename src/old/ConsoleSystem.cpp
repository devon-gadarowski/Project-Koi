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

	setMessageCallback(ProcessCommand, (message_method_t) &ConsoleSystem::processCommand);

	registerCommands();
}

ConsoleSystem::~ConsoleSystem()
{
	//stop(nullptr);
	DEBUG("CONSOLE_SYSTEM - ConsoleSystem Destroyed");
}

void ConsoleSystem::registerCommands()
{
	commands[hashCode("exit")] = &ConsoleSystem::exit;
	commands[hashCode("loadscene")] = &ConsoleSystem::loadScene;
	commands[hashCode("load")] = &ConsoleSystem::loadScene;
}

void ConsoleSystem::processCommand(Message * msg)
{
	std::string command = (dynamic_cast<StringMessage *> (msg))->data;

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

void ConsoleSystem::exit(std::vector<std::string> args)
{
	msgBus->sendMessage(Shutdown);
	msgBus->sendMessage(Exit);
}

void ConsoleSystem::loadScene(std::vector<std::string> args)
{
	if (args.size() < 2)
		return;

	args[1].append(".json");

	msgBus->sendMessageNow(LoadScene, args[1]);
}
