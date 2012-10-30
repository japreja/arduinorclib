/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** Retracts.cpp
** Retractable landing gear and doors functionality
**
** Author: Daniel van den Ouden
** Project: ArduinoRCLib
** Website: http://sourceforge.net/p/arduinorclib/
** -------------------------------------------------------------------------*/

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <wiring.h>
#endif

#include <Retracts.h>
#include <util.h>


namespace rc
{

// Public functions

Retracts::Retracts(Type p_type)
:
m_type(p_type),
m_doorsSpeed(100),
m_gearSpeed(100),
m_delay(0),
m_lastTime(0),
m_moveTo(0),
m_time(0),
m_doorsStart(0),
m_gearStart(0),
m_doorsEnd(m_doorsSpeed),
m_gearEnd(m_gearSpeed),
m_doorsPosition(-256),
m_gearPosition(-256)
{
	
}


void Retracts::setType(Type p_type)
{
	m_type = p_type;
}


Retracts::Type Retracts::getType() const
{
	return m_type;
}


void Retracts::setDoorsSpeed(uint16_t p_time)
{
	m_doorsSpeed = p_time;
	updateTimeline();
}


uint16_t Retracts::getDoorsSpeed() const
{
	return m_doorsSpeed;
}


void Retracts::setGearSpeed(uint16_t p_time)
{
	m_gearSpeed = p_time;
	updateTimeline();
}


uint16_t Retracts::getGearSpeed() const
{
	return m_gearSpeed;
}


void Retracts::setDelay(int16_t p_delay)
{
	m_delay = p_delay;
	updateTimeline();
}
	

int16_t Retracts::getDelay() const
{
	return m_delay;
}


void Retracts::down()
{
	m_moveTo = 0;
}


void Retracts::up()
{
	m_moveTo = m_doorsEnd > m_gearEnd ? m_doorsEnd : m_gearEnd;
}


void Retracts::openDoors()
{
	m_moveTo = m_doorsStart < m_gearStart ? m_doorsStart : m_gearStart;
}


void Retracts::closeDoors()
{
	m_moveTo = m_doorsEnd;
}


void Retracts::lowerGear()
{
	m_moveTo = m_gearStart;
}


void Retracts::raiseGear()
{
	m_moveTo = m_gearEnd;
}


bool Retracts::isUp() const
{
	return m_time >= (m_doorsEnd > m_gearEnd ? m_doorsEnd : m_gearEnd);
}


bool Retracts::isDown() const
{
	return m_time <= (m_doorsStart < m_gearStart ? m_doorsStart : m_gearStart);
}


bool Retracts::doorsAreOpen() const
{
	return m_time <= m_doorsStart;
}


bool Retracts::doorsAreClosed() const
{
	return m_time >= m_doorsEnd;
}


bool Retracts::gearIsRaised() const
{
	return m_time <= m_gearStart;
}


bool Retracts::gearIsLowered() const
{
	return m_time >= m_gearEnd;
}


void Retracts::update()
{
	unsigned long now = millis();
	uint16_t delta = static_cast<uint16_t>(now - m_lastTime);
	m_lastTime = now;
	
	if (m_moveTo == m_time)
	{
		// nothing to do!
		return;
	}
	
	// move the timeline
	if (m_moveTo > m_time)
	{
		m_time += delta;
		if (m_time > m_moveTo)
		{
			m_time = m_moveTo;
		}
	}
	else
	{
		m_time -= delta;
		if (m_time < 0)
		{
			m_time = 0;
		}
	}
	
	// calculate positions of doors and gear
	int16_t doorsTime = m_time - m_doorsStart;
	if (doorsTime < 0)
	{
		doorsTime = 0;
	}
	else if (doorsTime > m_doorsSpeed)
	{
		doorsTime = m_doorsSpeed;
	}
	
	int16_t gearTime = m_time - m_gearStart;
	if (gearTime < 0)
	{
		gearTime = 0;
	}
	else if (gearTime > m_gearSpeed)
	{
		gearTime = m_gearSpeed;
	}
	
	// convert time to servo positions
	switch (m_type)
	{
	default:
	case Type_NoDoor:
		m_gearPosition = rangeToNormalized(static_cast<uint16_t>(gearTime), m_gearSpeed);
		break;
		
	case Type_Single:
		m_gearPosition = m_doorsPosition = 
			(rangeToNormalized(static_cast<uint16_t>(gearTime), m_gearSpeed) + 
			 rangeToNormalized(static_cast<uint16_t>(doorsTime), m_doorsSpeed)) / 2;
		break;
		
	case Type_Dual:
		m_gearPosition = rangeToNormalized(static_cast<uint16_t>(gearTime), m_gearSpeed);
		m_doorsPosition = rangeToNormalized(static_cast<uint16_t>(doorsTime), m_doorsSpeed);
		break;
	}
}


int16_t Retracts::getDoorsPosition() const
{
	return m_doorsPosition;
}


int16_t Retracts::getGearPosition() const
{
	return m_gearPosition;
}


// Private functions

void Retracts::updateTimeline()
{
	m_gearStart = 0;
	m_gearEnd = m_gearStart + m_gearSpeed;
	
	m_doorsStart = m_gearEnd + m_delay;
	m_doorsEnd = m_doorsStart + m_doorsSpeed;
	
	// in the unlikely situation that the doors close completely before the gear
	// has been raised fully, we move the start of the doorclosing back a bit
	// this may happen if the delay has been set too low (negative)
	if (m_doorsEnd < m_gearEnd)
	{
		m_doorsEnd = m_gearEnd;
		m_doorsStart = m_doorsEnd - m_doorsSpeed;
	}
	
	// it may be possible (if the delay has been set too low) that the doorstart
	// falls before the start of the timeline, in that case move everything back a bit.
	if (m_doorsStart < 0)
	{
		int16_t up = 0 - m_doorsStart;
		m_doorsStart += up;
		m_gearStart  += up;
		m_doorsEnd   += up;
		m_gearEnd    += up;
	}
}

// namespace end
}

