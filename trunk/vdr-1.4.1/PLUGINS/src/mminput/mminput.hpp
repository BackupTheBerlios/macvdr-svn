/*
 *  $Id: mminput.hpp,v 0.0.1 2007/02/08 Stefan Rieke $
 */
 
#ifndef VDR_MMINPUT_H
#define VDR_MMINPUT_H

#include <vdr/plugin.h>
#include <vdr/thread.h>
#include <vector>

#include "TunerData.h"

extern const char *VERSION;

class cTimerMMInput : public cThread{
	private:
		int _sleepTime;
		std::vector< TunerRec* >*_TunerData;
	protected:
		virtual void Action(void);
	public:
		cTimerMMInput(int sleep, std::vector< TunerRec* >* TunerData);
		virtual ~cTimerMMInput();
};

class cPluginMMInput : public cPlugin {
private:
	static const char *DESCRIPTION;
// timer thread to scan for new devices
	cTimerMMInput* _TimerMMInput;

	cMutex mutex;
	std::vector< TunerRec* >* _TunerData;

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
