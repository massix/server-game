#include "orwell/com/RawMessage.hpp"

#include <zmq.hpp>
#include <string>

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/Server.hpp"

#include "Common.hpp"

#include <log4cxx/ndc.h>

#include <unistd.h>
#include <mutex>
#include <thread>

using namespace log4cxx;

using namespace orwell::com;
using namespace orwell::messages;
using namespace std;

int g_status = 0;

static void ExpectWelcome(
		string const & iPlayerName,
		string const & iExpectedRobotName,
		Sender & ioPusher,
		Receiver & ioSubscriber)
{
	Hello aHelloMessage;
	aHelloMessage.set_name( iPlayerName );
	aHelloMessage.set_port( 80 );
	aHelloMessage.set_ip( "localhost" );
	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	ioPusher.send(aMessage);

	RawMessage aResponse ;
	if ( not Common::ExpectMessage("Welcome", ioSubscriber, aResponse) )
	{
		cout <<  "error : expected Welcome" << endl;
		g_status = -1;
	}

	Welcome aWelcome;
	aWelcome.ParsePartialFromString(aResponse._payload);

	if ( aWelcome.robot() != iExpectedRobotName )
	{
		cout << "error : expected another robot name : " << endl;
		g_status = -2;
	}
}

static void client()
{
	log4cxx::NDC ndc("client");
	zmq::context_t aContext(1);
	usleep(6 * 1000);
	ORWELL_LOG_INFO("create pusher");
	Sender aPusher("tcp://127.0.0.1:9000", ZMQ_PUSH, orwell::com::ConnectionMode::CONNECT, aContext);
	ORWELL_LOG_INFO("create subscriber");
	Receiver aSubscriber("tcp://127.0.0.1:9001", ZMQ_SUB, orwell::com::ConnectionMode::CONNECT, aContext);
	usleep(6 * 1000);

	ExpectWelcome("jambon", "Gipsy Danger", aPusher, aSubscriber);
	ExpectWelcome("fromage", "Goldorak", aPusher, aSubscriber);
	ExpectWelcome("poulet", "Securitron", aPusher, aSubscriber);

	Hello aHelloMessage;
	aHelloMessage.set_name("rutabagas");
	aHelloMessage.set_port( 80 );
	aHelloMessage.set_ip( "localhost" );

	RawMessage aMessage("randomid", "Hello", aHelloMessage.SerializeAsString());
	aPusher.send(aMessage);

	RawMessage aResponse;
	if ( not Common::ExpectMessage("Goodbye", aSubscriber, aResponse) )
	{
		ORWELL_LOG_ERROR("error : expected Goodbye");
		g_status = -1;
	}
	ORWELL_LOG_INFO("quit client");
}


static void const server(std::shared_ptr< orwell::Server > ioServer)
{
	log4cxx::NDC ndc("server");
	for (int i = 0 ; i < 4 ; ++i)
	{
		ioServer->loopUntilOneMessageIsProcessed();
	}
	ORWELL_LOG_INFO("quit server");
}

int main()
{
	orwell::support::GlobalLogger("hello", "test_hello.log");
	zmq::context_t aContext(1);
	log4cxx::NDC ndc("hello");
	std::shared_ptr< orwell::Server > aServer =
		std::make_shared< orwell::Server >(aContext, "tcp://*:9000", "tcp://*:9001", 500);
	ORWELL_LOG_INFO("server created");
	aServer->accessContext().addRobot("Gipsy Danger");
	aServer->accessContext().addRobot("Goldorak");
	aServer->accessContext().addRobot("Securitron");
	std::thread aServerThread(server, aServer);
	std::thread aClientThread(client);
	aClientThread.join();
	aServerThread.join();
	orwell::support::GlobalLogger::Clear();
	return g_status;
}

