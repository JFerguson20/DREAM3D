#pragma once

#include "IO/IOPlugin.h"

class IOGuiPlugin : public IOPlugin
{
  Q_OBJECT
  Q_INTERFACES(ISIMPLibPlugin)
  Q_PLUGIN_METADATA(IID "net.bluequartz.dream3d.IOGuiPlugin")

public:
  IOGuiPlugin();
  ~IOGuiPlugin() override;

public:
  IOGuiPlugin(const IOGuiPlugin&) = delete;            // Copy Constructor Not Implemented
  IOGuiPlugin(IOGuiPlugin&&) = delete;                 // Move Constructor
  IOGuiPlugin& operator=(const IOGuiPlugin&) = delete; // Copy Assignment Not Implemented
  IOGuiPlugin& operator=(IOGuiPlugin&&) = delete;      // Move Assignment Not Implemented
};
