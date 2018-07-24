/*
 * DateScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <iterator>
#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DateScript.hpp"

DateScript::DateScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result DateScript::run()
{
  static const ArgParser::Descriptor descriptors[] = {
      {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
      {"-a", "STRING", "set alarm to time described by STRING", 1, Arguments::alarmSetter},
      {"-s", "STRING", "set time described by STRING, format +\"%H:%M:%S %d.%m.%Y\"", 1, Arguments::timeSetter}
  };

  bool argumentsParsed;
  const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
      std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

  if (arguments.help)
  {
    ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
    return E_OK;
  }
  else if (!argumentsParsed)
  {
    return E_VALUE;
  }
  else if (arguments.alarm != nullptr)
  {
    return setAlarm(arguments.alarm);
  }
  else if (arguments.time != nullptr)
  {
    return setTime(arguments.time);
  }
  else
  {
    return showTime();
  }
}

Result DateScript::setAlarm(const char *inputString)
{
  time64_t timestamp;
  Result res;

  if (stringToTimestamp(inputString, &timestamp))
    res = time().setAlarm(timestamp * 1000000);
  else
    res = E_VALUE;

  return res;
}

Result DateScript::setTime(const char *inputString)
{
  time64_t timestamp;
  Result res;

  if (stringToTimestamp(inputString, &timestamp))
    res = time().setTime(timestamp * 1000000);
  else
    res = E_VALUE;

  return res;
}

Result DateScript::showTime()
{
  RtDateTime currentTime;
  rtMakeTime(&currentTime, time().getTime() / 1000000);

  const auto fill = tty().fill();
  const auto width = tty().width();

  tty() << Terminal::Fill{'0'} << Terminal::Width{2};
  tty() << static_cast<unsigned int>(currentTime.hour);
  tty() << ":" << static_cast<unsigned int>(currentTime.minute);
  tty() << ":" << static_cast<unsigned int>(currentTime.second);
  tty() << " " << static_cast<unsigned int>(currentTime.day);
  tty() << "." << static_cast<unsigned int>(currentTime.month);
  tty() << Terminal::Width{4};
  tty() << "." << static_cast<unsigned int>(currentTime.year);
  tty() << Terminal::EOL;
  tty() << width << fill;

  return E_OK;
}

bool DateScript::stringToTimestamp(const char *inputString, time64_t *result)
{
  unsigned int hour, minute, second, day, month, year;

  if (sscanf(inputString, "%u:%u:%u %u.%u.%u", &hour, &minute, &second, &day, &month, &year) == 6)
  {
    const RtDateTime dateTime{
        static_cast<uint16_t>(year),
        static_cast<uint8_t>(month),
        static_cast<uint8_t>(day),
        static_cast<uint8_t>(hour),
        static_cast<uint8_t>(minute),
        static_cast<uint8_t>(second)
    };

    return rtMakeEpochTime(result, &dateTime) == E_OK;
  }
  else
    return false;
}
