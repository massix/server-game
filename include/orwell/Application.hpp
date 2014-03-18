
#pragma once

#include <string>
#include <vector>
#include <memory>

namespace orwell {
class Server;
class BroadcastServer;
}

class Application
{
public:
	virtual ~Application() {};
	static Application & GetInstance();

	void run(int argc, char *argv[]);
	bool stop();

	void clean();
private:
	Application();
	Application(Application const & iRight);
	Application & operator=(Application const & iRight);
	
	// Initialization functions
	bool initApplication(int argc, char *argv[]);
	bool initConfigurationFile();
	bool initLogger();
	bool initServer();
	std::vector<std::string> & tokenizeRobots(std::string const & iRobotsString);
	
	// Instance of the server running
	orwell::Server * m_server;
	// Broadcast server for UDP discovery
	orwell::BroadcastServer * m_broadcastServer;
	
	// Configurations retrieved either from rc file or from command line
	// Command line has the priority over the rc file
	uint32_t m_pullerPort;
	uint32_t m_publisherPort;
	uint32_t m_agentPort;
	uint32_t m_ticInterval;
	std::string m_rcFilePath;
	std::vector<std::string> m_robotsList;
	std::vector<std::string> m_robotsForContext;
	bool m_consoleDebugLogs;

};
