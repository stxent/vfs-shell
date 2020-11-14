/*
 * ListNodesScript.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include <xcore/fs/utils.h>
#include <xcore/realtime.h>

const std::array<ArgParser::Descriptor, 5> ListNodesScript::descriptors{
    {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-i", nullptr, "show index of each node", 0, Arguments::showInodesSetter},
        {"-h", nullptr, "print human readable sizes", 0, Arguments::humanReadableSetter},
        {"-l", nullptr, "show detailed information", 0, Arguments::longListingSetter},
        {nullptr, "ENTRY", "directory to be shown", 0, Arguments::incrementNodeCount}
    }
};

ListNodesScript::ListNodesScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument},
  m_arguments{ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
      descriptors.cbegin(), descriptors.cend())},
  m_result{E_OK}
{
}

Terminal &operator<<(Terminal &output, ListNodesScript::HumanReadableAccess access)
{
  const char buffer[] = {
      access.directory ? 'd' : '-',
      access.value & FS_ACCESS_READ ? 'r' : '-',
      access.value & FS_ACCESS_WRITE ? 'w' : '-',
      '\0'
  };

  output.write(buffer, sizeof(buffer));
  return output;
}

Terminal &operator<<(Terminal &output, ListNodesScript::HumanReadableLength length)
{
  static const std::array<char, 4> suffixes = {'\0', 'K', 'M', 'G'};

  size_t index = 0;
  FsLength previous = 0;
  FsLength remainder = length.value;

  while (index < suffixes.size() - 1 && remainder >= 1024)
  {
    ++index;
    previous = remainder;
    remainder /= 1024;
  }

  const bool fractionalPartExists = length.value && remainder < 10;
  const size_t initialWidth = output.width().value;
  const size_t integerWidth = (index ? initialWidth - 1 : initialWidth) - (fractionalPartExists ? 2 : 0);

  output << Terminal::Width{integerWidth};
  output << remainder;

  if (fractionalPartExists)
  {
    output << Terminal::Width{1};
    output << '.' << (previous % 1024) * 10 / 1024;
  }

  if (suffixes[index] != '\0')
    output << suffixes[index];

  output << Terminal::Width{initialWidth};
  return output;
}

Terminal &operator<<(Terminal &output, ListNodesScript::HumanReadableTime timestamp)
{
  RtDateTime convertedTime;
  rtMakeTime(&convertedTime, timestamp.value / 1000000);

  const auto fill = output.fill();
  const auto width = output.width();

  output << Terminal::Fill{'0'} << Terminal::Width{4};
  output << static_cast<unsigned int>(convertedTime.year);
  output << Terminal::Width{2};
  output << "-" << static_cast<unsigned int>(convertedTime.month);
  output << "-" << static_cast<unsigned int>(convertedTime.day);
  output << " " << static_cast<unsigned int>(convertedTime.hour);
  output << ":" << static_cast<unsigned int>(convertedTime.minute);
  output << ":" << static_cast<unsigned int>(convertedTime.second);
  output << Terminal::Width{3};
  output << "." << static_cast<unsigned int>(static_cast<unsigned int>((timestamp.value % 1000000) / 1000));
  output << width << fill;

  return output;
}

Result ListNodesScript::run()
{
  if (m_arguments.help)
  {
    ArgParser::help(tty(), name(), descriptors.cbegin(), descriptors.cend());
    return E_OK;
  }
  else
  {
    if (m_arguments.nodeCount)
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, descriptors.cbegin(), descriptors.cend(),
          [this](const char *key){ printDirectoryContent(key); });
    }
    else
    {
      printDirectoryContent(env()["PWD"]);
    }

    return m_result;
  }
}

void ListNodesScript::printDirectoryContent(const char *positionalArgument)
{
  if (m_result != E_OK)
    return;

  if (m_arguments.nodeCount > 1)
    tty() << positionalArgument << ":" << Terminal::EOL;

  char path[Settings::PWD_LENGTH];

  fsJoinPaths(path, env()["PWD"], positionalArgument);
  FsNode * const root = fsOpenNode(fs(), path);
  if (root == nullptr)
  {
    tty() << name() << ": " << positionalArgument << ": node not found" << Terminal::EOL;
    m_result = E_ENTRY;
    return;
  }

  FsNode * const child = static_cast<FsNode *>(fsNodeHead(root));
  fsNodeFree(root);
  if (child == nullptr)
  {
    tty() << name() << ": " << positionalArgument << ": node has no descendants" << Terminal::EOL;
    return;
  }

  Result res;

  do
  {
    char nodeName[Settings::ENTRY_NAME_LENGTH];
    FsAccess nodeAccess;

    if ((res = fsNodeRead(child, FS_NODE_NAME, 0, nodeName, sizeof(nodeName), nullptr)) != E_OK)
    {
      tty() << name() << ": " << positionalArgument << ": error reading name attribute" << Terminal::EOL;
      res = E_ERROR;
      break;
    }

    if ((res = fsNodeRead(child, FS_NODE_ACCESS, 0, &nodeAccess, sizeof(nodeAccess), nullptr)) != E_OK)
    {
      tty() << name() << ": " << nodeName << ": error reading access attribute" << Terminal::EOL;
      res = E_ERROR;
      break;
    }

    FsLength nodeIdLength = 0;
    uint64_t nodeId = 0;

    if (fsNodeLength(child, FS_NODE_DATA, &nodeIdLength) == E_OK)
    {
      if (nodeIdLength <= sizeof(nodeId))
        fsNodeRead(child, FS_NODE_ID, 0, &nodeId, nodeIdLength, nullptr);
    }

    FsLength nodeDataLength = 0;
    fsNodeLength(child, FS_NODE_DATA, &nodeDataLength);

    time64_t nodeTime = 0;
    fsNodeRead(child, FS_NODE_TIME, 0, &nodeTime, sizeof(nodeTime), nullptr);

    // Get information about child nodes
    FsNode * const firstDescendant = static_cast<FsNode *>(fsNodeHead(child));
    bool hasDescendants = false;

    if (firstDescendant)
    {
      hasDescendants = true;
      fsNodeFree(firstDescendant);
    }

    if (m_arguments.longListing)
    {
      const auto format = tty().format();
      const auto width = tty().width();

      // Print inode value
      if (m_arguments.showInodes)
        tty() << Terminal::Width{16} << Terminal::Format::HEX << nodeId << Terminal::Width{1} << Terminal::Format::DEC;

      // Print access flags
      tty() << " " << HumanReadableAccess{nodeAccess, hasDescendants};

      // Print node size
      tty() << " ";
      if (m_arguments.humanReadable)
        tty() << Terminal::Width{8} << HumanReadableLength{nodeDataLength} << Terminal::Width{1};
      else
        tty() << Terminal::Width{10} << nodeDataLength << Terminal::Width{1};

      // Date and time
      tty() << " " << HumanReadableTime{nodeTime};

      tty() << " ";
      if (hasDescendants)
        tty() << Terminal::BOLD << Terminal::Color::BLUE << nodeName << Terminal::Color::WHITE << Terminal::REGULAR;
      else
        tty() << nodeName;

      tty() << Terminal::EOL;
      tty() << format << width;
    }
    else
    {
      tty() << nodeName << Terminal::EOL;
    }
  }
  while ((res = fsNodeNext(child)) == E_OK);

  fsNodeFree(child);

  if (res != E_ENTRY)
    m_result = res;
}
