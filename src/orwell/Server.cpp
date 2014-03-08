#include "orwell/Server.hpp"

#include <string>
#include <cstring>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <unistd.h>

#include <zmq.hpp>

#include "MissingFromTheStandard.hpp"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/com/ConnectionMode.hpp"
#include "orwell/com/RawMessage.hpp"
#include "orwell/callbacks/ProcessDecider.hpp"
#include "orwell/callbacks/ProcessTimer.hpp"

#define UDP_MESSAGE_LIMIT 512

using orwell::com::Receiver;
using orwell::com::Sender;
using orwell::callbacks::ProcessDecider;
using orwell::callbacks::ProcessTimer;

namespace orwell
{

Server::Server(
		zmq::context_t & ioZmqContext,
		std::string const & iPullUrl,
		std::string const & iPublishUrl,
		long const iTicDuration)
	: _zmqContext(ioZmqContext)
	, _puller(std::make_shared< Receiver >(
				iPullUrl,
				ZMQ_PULL,
				orwell::com::ConnectionMode::BIND,
				_zmqContext,
				0))
	, _publisher(std::make_shared< Sender >(
				iPublishUrl,
				ZMQ_PUB,
				orwell::com::ConnectionMode::BIND,
				_zmqContext,
				0))
	, _game()
	, _decider(_game, _publisher)
	, _ticDuration( boost::posix_time::milliseconds(iTicDuration) )
	, _previousTic(boost::posix_time::second_clock::local_time() )
	, _mainLoopRunning(false)
	, _forcedStop(false)
{
}

Server::~Server()
{
}

bool Server::processMessageIfAvailable()
{
	bool aProcessedMessage = false;
	orwell::com::RawMessage aMessage;
	if (_puller->receive(aMessage))
	{
		_decider.process(aMessage);
		aProcessedMessage = true;
	}
	return aProcessedMessage;
}

void Server::loopUntilOneMessageIsProcessed()
{
	bool aMessageHasBeenProcessed = false;
	boost::posix_time::time_duration aDuration;
	boost::posix_time::ptime aCurrentTic;

	while (not aMessageHasBeenProcessed)
	{
		aCurrentTic = boost::posix_time::second_clock::local_time();
		aDuration = aCurrentTic - _previousTic;
		if ( aDuration < _ticDuration )
		{
			if ( not processMessageIfAvailable() )
			{
				usleep(10 * 1000);
			}
			else
			{
				aMessageHasBeenProcessed = true;
			}
		}
		else if (_forcedStop)
		{
			break;
		}
		else
		{
			ProcessTimer aProcessTimer(_publisher, _game);
			aProcessTimer.execute();
			_previousTic = aCurrentTic;
		}
	}
}

void Server::loop()
{
	_mainLoopRunning = true;
	
	while (_mainLoopRunning)
	{
		loopUntilOneMessageIsProcessed();
	}
}
	
void Server::stop()
{
	ORWELL_LOG_INFO("Terminating server main loop");
	_forcedStop = true;
	_mainLoopRunning = false;
}

orwell::game::Game & Server::accessContext()
{
	return _game;
}

}

