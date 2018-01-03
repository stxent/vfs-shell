/*
 * ListNodesScript.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/realtime.h>
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"

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

Result ListNodesScript::run()
{
  if (m_arguments.help)
  {
    ArgParser::help(tty(), name(), descriptors().cbegin(), descriptors().cend());
    return E_OK;
  }
  else
  {
    if (m_arguments.nodeCount)
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, descriptors().cbegin(), descriptors().cend(),
          std::bind(&ListNodesScript::printDirectoryContent, this, std::placeholders::_1));
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

  ShellHelpers::joinPaths(path, env()["PWD"], positionalArgument);
  FsNode * const root = ShellHelpers::openNode(fs(), path);
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
    char nodeName[128]; // XXX
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
    FsNode * const descendant = static_cast<FsNode *>(fsNodeHead(child));
    bool hasDescendants = false;

    if (descendant)
    {
      hasDescendants = true;
      fsNodeFree(descendant);
    }

    if (m_arguments.longListing)
    {
      // Access
      char printableNodeAccess[4];

      printableNodeAccess[0] = hasDescendants ? 'd' : '-';
      printableNodeAccess[1] = nodeAccess & FS_ACCESS_READ ? 'r' : '-';
      printableNodeAccess[2] = nodeAccess & FS_ACCESS_WRITE ? 'w' : '-';
      printableNodeAccess[3] = '\0';

      // Date and time
      const time_t unixTime = static_cast<time_t>(nodeTime / 1000000);
      const unsigned int nodeTimeFrac = static_cast<unsigned int>((nodeTime % 1000000) / 1000);
      RtDateTime convertedTime;
      char printableNodeTime[32];

      rtMakeTime(&convertedTime, unixTime);
      sprintf(printableNodeTime, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
          convertedTime.year, convertedTime.month, convertedTime.day,
          convertedTime.hour, convertedTime.minute, convertedTime.second,
          nodeTimeFrac);

      // Print all values
      if (m_arguments.showInodes)
        tty() << Terminal::Width{16} << Terminal::Format::HEX << nodeId << Terminal::Width{1} << Terminal::Format::DEC;

      tty() << " " << printableNodeAccess;

      if (m_arguments.humanReadable)
        tty() << " " << Terminal::Width{8} << HumanReadableLength{nodeDataLength} << Terminal::Width{1};
      else
        tty() << " " << Terminal::Width{10} << nodeDataLength << Terminal::Width{1};

      tty() << " " << printableNodeTime;
      tty() << " " << nodeName;
      tty() << Terminal::EOL;
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
