/*
 * channelscan.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __CHANNELSCAN_H
#define __CHANNELSCAN_H

#include <vdr/plugin.h>
#include <vdr/thread.h>
#include <vector>
#include <string>


#define DBG " Channelscan DEBUG: -- "
#define ERR " Channelscan ERROR: -- "

#define DEBUG_CHANNELSCAN 
#ifdef DEBUG_CHANNELSCAN
#  define DLOG(x...) dsyslog(x)
#else
# define DLOG(x...)
#endif


using std::vector;
using std::string;

static const char *VERSION        = "0.4.1";
static const char *DESCRIPTION    = "Search Transponders for DVB Channels";
static const char *MAINMENUENTRY  = "Channelscan";

extern cMutex mutexNames;
extern vector<string> tvChannelNames;
extern vector<string> radioChannelNames;
extern vector<string> dataChannelNames;

/*
enum eScan {
		idle,
		scanning,
		error,
		aborted,
		success
};
*/
enum eAutoScanStat { // SAT / MixMode Loop ??
		AssNone,
		AssDvbS,
		AssDvbC,
		AssDvbT
};


//extern volatile eScan ScanStat;

// --- cPluginChannelscan ---------------------------------------------------------

class cPluginChannelscan : public cPlugin {
private:
	// Add any member variables or functions you may need here.
public:
	cPluginChannelscan();
	virtual ~cPluginChannelscan();
	virtual const char *Version(void) { return VERSION; }
	virtual const char *Description(void) { return tr(DESCRIPTION); }
	virtual const char *CommandLineHelp(void);
	virtual bool ProcessArgs(int argc, char *argv[]);
	virtual bool Initialize(void);
	virtual bool Start(void);
	virtual void Housekeeping(void);
	virtual const char *MainMenuEntry(void) { return tr(MAINMENUENTRY); }
	virtual cOsdObject *MainMenuAction(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *Name, const char *Value);
    virtual bool Service(const char *Id,void *Data = NULL);
    virtual bool HasSetupOptions(void) { return true; }
    static eAutoScanStat AutoScanStat;
};

// --- cSetupMenu ------------------------------------------------------
// Move to menu.c

class cMenuChannelscanSetup : public cMenuSetupPage
{
private:
	int serviceType;
	const char *serviceTypeTexts[3];
	virtual void Setup(void);
protected:
	//virtual eOSState ProcessKey(eKeys Key);
	virtual void Store(void);
public:
	cMenuChannelscanSetup(void);
    static int SetupServiceType;
};

#endif // __CHANNELSCAN_H
