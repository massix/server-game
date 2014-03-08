#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"

#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>

#include "orwell/support/GlobalLogger.hpp"

using namespace log4cxx;
using namespace std;

static orwell::Server * ServerPtr;
static void signal_handler(int signum)
{
	ServerPtr->stop();
}

int main()
{
	std::string aString;

	orwell::support::GlobalLogger("server_web", "orwell.log", true);

	zmq::context_t aContext(1);
	std::string const aPullerUrl("tcp://*:9000");
	std::string const aPublisherUrl("tcp://*:9001");
	orwell::Server aServer(aContext, aPullerUrl, aPublisherUrl, 500);
	aServer.accessContext().addRobot("Gipsy Danger");
	aServer.accessContext().addRobot("Goldorak");
	aServer.accessContext().addRobot("Securitron");
	
	// This is needed to handle the signal
	ServerPtr = &aServer;

	// Register the signal handler
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	
	pid_t aChildProcess = fork();
	switch (aChildProcess)
	{
		case 0:
		{
			ORWELL_LOG_INFO("Child started, pid: " << aChildProcess);
			orwell::BroadcastServer aBroadcastServer(aPullerUrl, aPublisherUrl);
			aBroadcastServer.runBroadcastReceiver();
			
			// The child can stop here
			return 0;
		}
		default:
		{
			ORWELL_LOG_INFO("Father continued, pid: " << aChildProcess);
			aServer.loop();
			break;
		}
	}
	
	// Let's wait for everything to be over.
	int status;
	while (waitpid(aChildProcess, &status, WNOHANG) == 0)
	{
		ORWELL_LOG_INFO("Waiting for child " << aChildProcess << " to complete..");
		sleep(1);
	}

	orwell::support::GlobalLogger::Clear();
	return 0;
}

