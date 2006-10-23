/*
 *  $Id: menu.h,v 1.1.1.1 2004/12/30 22:44:02 lordjaxom Exp $
 */
 
#ifndef VDR_MMINPUT_MENU_H
#define VDR_MMINPUT_MENU_H

#include <vdr/osd.h>

//#include "client/remote.h"
#include "device.hpp"

class cMMInputMenuRecordingItem;

// --- cMMInputMenu --------------------------------------------------------

class cMMInputMenu: public cOsdMenu {
private:
	enum eSubmenus {
		sub_Start = os_User,
		subSyncEPG
	};

protected:
	void SuspendServer(void);

public:
	cMMInputMenu(void);
	virtual ~cMMInputMenu(void);

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cMMInputMenuSchedule ------------------------------------------------

class cMMInputMenuSchedule: public cOsdMenu {
private:
	bool              m_Now;
	bool              m_Next;
	int               m_OtherChannel;
#if VDRVERSNUM < 10300
	cMutexLock        m_Lock;
#else
	cSchedulesLock    m_Lock;
#endif

protected:
	void PrepareSchedule(cChannel *Channel);

public:
	cMMInputMenuSchedule(void);
	virtual ~cMMInputMenuSchedule(void);

	virtual eOSState ProcessKey(eKeys Key);
};

// --- cMMInputMenuWhatsOn -------------------------------------------------

class cMMInputMenuWhatsOn: public cOsdMenu {
private:
	static int               m_CurrentChannel;
#if VDRVERSNUM < 10300
	static const cEventInfo *m_ScheduleEventInfo;
#else
	static const cEvent     *m_ScheduleEventInfo;
#endif

protected:

public:
	cMMInputMenuWhatsOn(const cSchedules *Schedules, bool Now, 
			int CurrentChannel);

	static int CurrentChannel(void) { return m_CurrentChannel; }
	static void SetCurrentChannel(int Channel) { m_CurrentChannel = Channel; }
#if VDRVERSNUM < 10300
	static const cEventInfo *ScheduleEventInfo(void);
#else
	static const cEvent *ScheduleEventInfo(void);
#endif

	virtual eOSState ProcessKey(eKeys Key);
};

#endif // VDR_MMINPUT_MENU_H

