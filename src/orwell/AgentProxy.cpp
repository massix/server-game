#include "orwell/AgentProxy.hpp"

#include "orwell/Application.hpp"
#include "orwell/Server.hpp"
#include "orwell/game/Player.hpp"
#include "orwell/game/Robot.hpp"
#include "orwell/com/Url.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include <sstream>
#include <functional>

namespace orwell
{

AgentProxy::AgentProxy(orwell::Application & ioApplication)
	: m_application(ioApplication)
{
}

void echo(std::string const & iMessage)
{
	ORWELL_LOG_DEBUG("iMessage = '" << iMessage << "'");
}

static void Dispatch(
		std::istringstream & ioStream,
		std::function< void() > iFunction,
		bool & oResult,
		std::string & ioReply)
{
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction();
		ORWELL_LOG_DEBUG("Dispatch OK");
		ioReply = "OK";
	}
}

static void DispatchArgument(
		std::istringstream & ioStream,
		std::function< void(std::string const &) > iFunction,
		bool & oResult,
		std::string & ioReply)
{
	std::string aArg;
	ioStream >> aArg;
	oResult = ioStream.eof();
	if (oResult)
	{
		iFunction(aArg);
		ORWELL_LOG_DEBUG("DispatchArgument OK");
		ioReply = "OK";
	}
}

bool AgentProxy::step(
		std::string const & iCommand,
		std::string & ioReply)
{
	bool aResult = false;
	std::string aAction;
	using std::placeholders::_1;
	std::istringstream aStream(iCommand);
	aStream >> aAction;
	if ("list" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aIgnored;
		aResult = aStream.eof();
		if (aResult)
		{
			if ("player" == aObject)
			{
				listPlayer(ioReply);
			}
			else if ("robot" == aObject)
			{
				listRobot(ioReply);
			}
			else
			{
				aResult = false;
			}
		}
	}
	else if ("start" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("game" == aObject)
		{
			Dispatch(
					aStream,
					std::bind(&AgentProxy::startGame, this),
					aResult,
					ioReply);
		}
	}
	else if ("stop" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("application" == aObject)
		{
			Dispatch(
					aStream,
					std::bind(&AgentProxy::stopApplication, this),
					aResult,
					ioReply);
		}
		else if ("game" == aObject)
		{
			Dispatch(
					aStream,
					std::bind(&AgentProxy::stopGame, this),
					aResult,
					ioReply);
		}
	}
	else if ("add" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::addRobot, this, _1),
					aResult,
					ioReply);
		}
		else if ("player" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::addPlayer, this, _1),
					aResult,
					ioReply);
		}
	}
	else if ("remove" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::removeRobot, this, _1),
					aResult,
					ioReply);
		}
		else if ("player" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::removePlayer, this, _1),
					aResult,
					ioReply);
		}
	}
	else if ("register" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::registerRobot, this, _1),
					aResult,
					ioReply);
		}
	}
	else if ("unregister" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		if ("robot" == aObject)
		{
			DispatchArgument(
					aStream,
					std::bind(&AgentProxy::unregisterRobot, this, _1),
					aResult,
					ioReply);
		}
	}
	else if ("set" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aName;
		aStream >> aName;
		std::string aProperty;
		aStream >> aProperty;
		std::string aValue;
		aStream >> aValue;
		if ("robot" == aObject)
		{
			Dispatch(
					aStream,
					std::bind(&AgentProxy::setRobot, this, aName, aProperty, aValue),
					aResult,
					ioReply);
		}
	}
	else if ("get" == aAction)
	{
		std::string aObject;
		aStream >> aObject;
		std::string aName;
		aStream >> aName;
		std::string aProperty;
		aStream >> aProperty;
		if ("robot" == aObject)
		{
			getRobot(aName, aProperty, ioReply);
			aResult = true;
		}
	}
	else if ("ping" == aAction)
	{
		ioReply = "pong";
		aResult = true;
	}
	ORWELL_LOG_DEBUG("Parsing result = " << aResult);
	if (not aResult)
	{
		ORWELL_LOG_WARN("Command not parsed sucessfully: '" << iCommand << "'");
		ioReply = "KO";
	}
	return aResult;
}

void AgentProxy::stopApplication()
{
	ORWELL_LOG_INFO("stop application");
	m_application.stop();
}

void AgentProxy::listRobot(std::string & ioReply)
{
	ORWELL_LOG_INFO("list robot");
	std::map< std::string, std::shared_ptr< orwell::game::Robot > > aRobots =
		m_application.accessServer()->accessContext().getRobots();
	ioReply = "Robots:\n";
	for (auto const & aPair : aRobots)
	{
		ioReply += "\t" + aPair.first + " -> ";
		ioReply += "name = " + aPair.second->getName() + " ; ";
		if (not aPair.second->getHasRealRobot())
		{
			ioReply += "not ";
		}
		ioReply += "registered ; ";
		ioReply += "video_url = " + aPair.second->getVideoUrl() + " ; ";
		bool aHasPlayer(aPair.second->getPlayer());
		if (aHasPlayer)
		{
			ioReply += "player = " + aPair.second->getPlayer()->getName() + "\n";
		}
		else
		{
			ioReply += "player = \n";
		}
	}
}


void AgentProxy::addRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("add robot " << iRobotName);
	m_application.accessServer()->accessContext().addRobot(iRobotName, m_application.popPort(), m_application.popPort());
}

void AgentProxy::removeRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("remove robot " << iRobotName);
	m_application.accessServer()->accessContext().removeRobot(iRobotName);
}

void AgentProxy::registerRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("register robot " << iRobotName);
	try
	{
		m_application.accessServer()->accessContext().accessRobot(iRobotName)
			->setHasRealRobot(true);
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::unregisterRobot(std::string const & iRobotName)
{
	ORWELL_LOG_INFO("unregister robot " << iRobotName);
	try
	{
		m_application.accessServer()->accessContext().accessRobot(iRobotName)
			->setHasRealRobot(false);
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::setRobot(
		std::string const & iRobotName,
		std::string const & iProperty,
		std::string const & iValue)
{
	ORWELL_LOG_INFO("set robot " << iRobotName <<
		" " << iProperty << " " << iValue);
	try
	{
		std::shared_ptr< orwell::game::Robot > aRobot =
			m_application.accessServer()->accessContext().accessRobot(iRobotName);
		if ("video_url" == iProperty)
		{
			aRobot->setVideoUrl(iValue);
		}
		else
		{
			ORWELL_LOG_WARN("Unknown property for a robot: '" << iProperty << "'");
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::getRobot(
		std::string const & iRobotName,
		std::string const & iProperty,
		std::string & oValue)
{
	ORWELL_LOG_INFO("get robot " << iRobotName << " " << iProperty);
	try
	{
		std::shared_ptr< orwell::game::Robot > aRobot =
			m_application.accessServer()->accessContext().accessRobot(iRobotName);
		if ("id" == iProperty)
		{
			oValue = aRobot->getRobotId();
			ORWELL_LOG_INFO("id = " << oValue);
		}
		else if ("video_url" == iProperty)
		{
			oValue = aRobot->getVideoUrl();
		}
		else if ("video_port" == iProperty)
		{
			oValue = boost::lexical_cast< std::string >(aRobot->getVideoRetransmissionPort());
			ORWELL_LOG_INFO("video retransmission port = " << oValue);
		}
		else if ("video_command_port" == iProperty)
		{
			oValue = boost::lexical_cast< std::string >(aRobot->getServerCommandPort());
			ORWELL_LOG_INFO("video retransmission port = " << oValue);
		}
		else
		{
			oValue = "KO";
			ORWELL_LOG_WARN("Unknown property for a robot: '" << iProperty << "'");
		}
	}
	catch (std::exception const & anException)
	{
		ORWELL_LOG_ERROR(anException.what());
	}
}

void AgentProxy::listPlayer(std::string & ioReply)
{
	ORWELL_LOG_INFO("list player");
	std::map< std::string, std::shared_ptr< orwell::game::Player > > aPlayers =
		m_application.accessServer()->accessContext().getPlayers();
	ioReply = "Players:\n";
	for (auto const & aPair : aPlayers)
	{
		ioReply += "\t" + aPair.first + " -> ";
		ioReply += "name = " + aPair.second->getName() + " ; ";
		bool aHasRobot(aPair.second->getRobot());
		if (aHasRobot)
		{
			ioReply += "robot = " + aPair.second->getRobot()->getName() + "\n";
		}
		else
		{
			ioReply += "robot = \n";
		}
	}
}


void AgentProxy::addPlayer(std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("add player " << iPlayerName);
	m_application.accessServer()->accessContext().addPlayer(iPlayerName);
}

void AgentProxy::removePlayer(std::string const & iPlayerName)
{
	ORWELL_LOG_INFO("remove player " << iPlayerName);
	m_application.accessServer()->accessContext().removePlayer(iPlayerName);
}

void AgentProxy::startGame()
{
	ORWELL_LOG_INFO("start game");
	m_application.accessServer()->accessContext().start();
}

void AgentProxy::stopGame()
{
	ORWELL_LOG_INFO("stop game");
	m_application.accessServer()->accessContext().stop();
}

// protected

}

