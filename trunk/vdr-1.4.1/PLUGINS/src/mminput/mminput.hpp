/*
 *  $Id: mminput.hpp,v 0.0.1 2007/02/08 Stefan Rieke $
 */
 
#ifndef VDR_MMINPUT_H
#define VDR_MMINPUT_H

#include <vdr/plugin.h>

extern const char *VERSION;

class cPluginMMInput : public cPlugin {
private:
	static const char *DESCRIPTION;

public:
  cPluginMMInput(void);
  virtual ~cPluginMMInput();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void);
  virtual bool Start(void);
  virtual void Housekeeping(void);
  virtual const char *MainMenuEntry(void);
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
};

#endif // VDR_MMINPUT_H
