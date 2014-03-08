#pragma once

#include <memory>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "orwell/callbacks/ProcessDecider.hpp"

#include "orwell/game/Game.hpp"


namespace zmq
{
class context_t;
} // namespace zmq

namespace orwell
{

namespace com
{
class Receiver;
class Sender;
}

class Server
{
public:
	Server(
			zmq::context_t & ioZmqContext,
			std::string const & iPullUrl = "tcp://*:9000",
			std::string const & iPublishUrl = "tcp://*:9001",
			long const iTicDuration = 500); //milliseconds

	~Server();

	/// processMessageIfAvailable
	bool processMessageIfAvailable();
	///// run the broadcast receiver
	//void runBroadcastReceiver();
	/// Wait for 1 message and process it. Execute timed operations if needed.
	void loopUntilOneMessageIsProcessed();
	/// Loop eternaly to process all incoming messages.
	void loop();
	/// Correctly stop the server
	void stop();

	orwell::game::Game & accessContext();

private:
	zmq::context_t & _zmqContext;
	std::shared_ptr< com::Receiver > _puller;
	std::shared_ptr< com::Sender > _publisher;
	orwell::game::Game _game;
	orwell::callbacks::ProcessDecider _decider;

	boost::posix_time::time_duration const _ticDuration;
	boost::posix_time::ptime _previousTic;
	
	bool _mainLoopRunning;
	bool _forcedStop;
};

}

