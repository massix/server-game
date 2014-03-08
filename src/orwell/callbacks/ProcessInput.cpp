#include "orwell/callbacks/ProcessInput.hpp"

#include "controller.pb.h"
#include "server-game.pb.h"

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/com/Sender.hpp"
#include "orwell/game/Game.hpp"
#include "orwell/com/RawMessage.hpp"

using orwell::messages::Input;
using orwell::com::RawMessage;

namespace orwell {
namespace callbacks {

ProcessInput::ProcessInput(
		std::shared_ptr< com::Sender > ioPublisher,
		game::Game & ioGame)
	: InterfaceProcess(ioPublisher, ioGame)
{
}

void ProcessInput::execute()
{
	std::string const & aDestination = getArgument("RoutingID").second;
	orwell::messages::Input const & anInputMsg = static_cast<orwell::messages::Input const &>(*_msg);

	ORWELL_LOG_INFO("ProcessInput::execute : simple relay");

	ORWELL_LOG_DEBUG("===Input Message===");
	ORWELL_LOG_DEBUG("Move : left=" << anInputMsg.move().left() << ",right=" <<  anInputMsg.move().right() << "-");
	ORWELL_LOG_DEBUG("Fire : w1=" << anInputMsg.fire().weapon1() << ",w2=" <<  anInputMsg.fire().weapon2() << "-");
	ORWELL_LOG_DEBUG("===End Input Message===");

	RawMessage aReply(aDestination, "Input", anInputMsg.SerializeAsString());
	_publisher->send( aReply );
}

}}

