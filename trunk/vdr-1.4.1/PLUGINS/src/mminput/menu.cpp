/*
 *  $Id: menu.c,v 1.4 2005/03/12 12:54:19 lordjaxom Exp $
 */
 
#include <vdr/menuitems.h>
#include <vdr/interface.h>

#include "menu.h"
#include "i18n.h"

#define CHNUMWIDTH  (numdigits(Channels.MaxNumber()) + 1)

// --- cMenuText -------------------------------------------------------------

class cMenuText : public cOsdMenu {
public:
  cMenuText(const char *Title, const char *Text, eDvbFont Font = fontOsd);
  virtual eOSState ProcessKey(eKeys Key);
  };

// --- cMMInputMenu --------------------------------------------------------

cMMInputMenu::cMMInputMenu(void):
		cOsdMenu(tr("Streaming Control")) {
	SetHasHotkeys();
	Add(new cOsdItem(hk(tr("Synchronize EPG")),   (eOSState)subSyncEPG));
}

cMMInputMenu::~cMMInputMenu() {
}

eOSState cMMInputMenu::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	switch (state) {
	case subSyncEPG:		ClientSocket.SynchronizeEPG(); return osEnd;
	default:            return state;
	}
}


#if VDRVERSNUM < 10307
// --- cMenuEditChanItem -----------------------------------------------------

class cMenuEditChanItem : public cMenuEditIntItem {
protected:
  virtual void Set(void);
public:
  cMenuEditChanItem(const char *Name, int *Value);
  virtual eOSState ProcessKey(eKeys Key);
  };

// --- cMenuEditDateItem -----------------------------------------------------

class cMenuEditDateItem : public cMenuEditItem {
protected:
  time_t *value;
  virtual void Set(void);
public:
  cMenuEditDateItem(const char *Name, time_t *Value);
  virtual eOSState ProcessKey(eKeys Key);
  };

// --- cMenuWhatsOnItem ------------------------------------------------------

#if VDRVERSNUM < 10300
class cMenuWhatsOnItem : public cOsdItem {
public:
  const cEventInfo *eventInfo;
#	ifdef HAVE_BEAUTYPATCH
  cMenuWhatsOnItem(const cEventInfo *EventInfo, bool ShowProgressBar);
  ~cMenuWhatsOnItem();
  virtual void Display(int Offset= -1, eDvbColor FgColor = clrWhite, eDvbColor BgColor = clrBackground);
protected:
  cBitmap *progressBar;
  bool showProgressBar;
  float percent;
private:
  void DrawProgressBar(eDvbColor FgColor, eDvbColor BgColor);
#	else
  cMenuWhatsOnItem(const cEventInfo *EventInfo);
#	endif
};
#else
class cMenuWhatsOnItem : public cOsdItem {
public:
  const cEvent *event;
  const cChannel *channel;
  cMenuWhatsOnItem(const cEvent *Event, cChannel *Channel); //, bool Now = false);
};
#endif

// --- cMenuEvent ------------------------------------------------------------

#if VDRVERSNUM < 10300
class cMenuEvent : public cOsdMenu {
private:
  const cEventInfo *eventInfo;
public:
  cMenuEvent(const cEventInfo *EventInfo, bool CanSwitch = false);
  cMenuEvent(bool Now);
  virtual eOSState ProcessKey(eKeys Key);
};
#elif VDRVERSNUM < 10307
class cMenuEvent : public cOsdMenu {
private:
  const cEvent *event;
public:
  cMenuEvent(const cEvent *Event, bool CanSwitch = false);
  cMenuEvent(bool Now);
  virtual eOSState ProcessKey(eKeys Key);
};
#else
class cMenuEvent : public cOsdMenu {
private:
  const cEvent *event;
public:
  cMenuEvent(const cEvent *Event, bool CanSwitch = false);
  virtual void Display(void);
  virtual eOSState ProcessKey(eKeys Key);
};
#endif

// --- cMMInputMenuWhatsOn -------------------------------------------------

int cMMInputMenuWhatsOn::m_CurrentChannel = 0;
#if VDRVERSNUM < 10300
const cEventInfo *cMMInputMenuWhatsOn::m_ScheduleEventInfo = NULL;
#else
const cEvent *cMMInputMenuWhatsOn::m_ScheduleEventInfo = NULL;
#endif

#if VDRVERSNUM < 10300
static int CompareEventChannel(const void *p1, const void *p2) {
  return (int)((*(const cEventInfo**)p1)->GetChannelNumber() 
			- (*(const cEventInfo**)p2)->GetChannelNumber());
}
#endif

cMMInputMenuWhatsOn::cMMInputMenuWhatsOn(const cSchedules *Schedules,
		bool Now, int CurrentChannel):
		cOsdMenu(Now ? tr("What's on now?") : tr("What's on next?"), CHNUMWIDTH, 
				7, 6) {
#if VDRVERSNUM < 10300
	const cSchedule *Schedule = Schedules->First();
	const cEventInfo **pArray = NULL;
	int num = 0;

	while (Schedule) {
		pArray=(const cEventInfo**)realloc(pArray, (num + 1) * sizeof(cEventInfo*));
		pArray[num] = Now ? Schedule->GetPresentEvent() 
				: Schedule->GetFollowingEvent();
		if (pArray[num]) {
			cChannel *channel
					= Channels.GetByChannelID(pArray[num]->GetChannelID(), true);
			if (channel)
				pArray[num++]->SetChannelNumber(channel->Number());
		}
		Schedule = Schedules->Next(Schedule);
	}

	qsort(pArray, num, sizeof(cEventInfo*), CompareEventChannel);
	for (int a = 0; a < num; ++a) {
		int channelnr = pArray[a]->GetChannelNumber();
#	ifdef HAVE_BEAUTYPATCH
		Add(new cMenuWhatsOnItem(pArray[a],Now), channelnr == CurrentChannel);
#	else
		Add(new cMenuWhatsOnItem(pArray[a]), channelnr == CurrentChannel);
#	endif
	}

	free(pArray);
#else
  for (cChannel *Channel = Channels.First(); Channel; 
			Channel = Channels.Next(Channel)) {
  	if (!Channel->GroupSep()) {
      const cSchedule *Schedule 
					= Schedules->GetSchedule(Channel->GetChannelID());
      if (Schedule) {
        const cEvent *Event = Now ? Schedule->GetPresentEvent() 
						: Schedule->GetFollowingEvent();
        if (Event)
          Add(new cMenuWhatsOnItem(Event, Channel), 
							Channel->Number() == CurrentChannel);
      }
    }
  }
#endif
	m_CurrentChannel = CurrentChannel;
	SetHelp(Count() ? tr("Record") : NULL, Now ? tr("Next") : tr("Now"), 
			tr("Schedule"), tr("Switch"));
}

#if VDRVERSNUM < 10300
const cEventInfo *cMMInputMenuWhatsOn::ScheduleEventInfo(void) {
	const cEventInfo *ei = m_ScheduleEventInfo;
	m_ScheduleEventInfo = NULL;
	return ei;
}
#else
const cEvent *cMMInputMenuWhatsOn::ScheduleEventInfo(void) {
	const cEvent *ei = m_ScheduleEventInfo;
	m_ScheduleEventInfo = NULL;
	return ei;
}
#endif

eOSState cMMInputMenuWhatsOn::Switch(void) {
	cMenuWhatsOnItem *item = (cMenuWhatsOnItem*)Get(Current());
	if (item) {
		cChannel *channel 
#if VDRVERSNUM < 10300
				= Channels.GetByChannelID(item->eventInfo->GetChannelID(), true);
#else
				= Channels.GetByChannelID(item->event->ChannelID(), true);
#endif
		if (channel && cDevice::PrimaryDevice()->SwitchChannel(channel, true))
			return osEnd;
	}
	ERROR(tr("Can't switch channel!"));
	return osContinue;
}

eOSState cMMInputMenuWhatsOn::Record(void) {
	cMenuWhatsOnItem *item = (cMenuWhatsOnItem*)Get(Current());
	if (item) {
		cRemoteTimer *timer 
#if VDRVERSNUM < 10300
				= new cRemoteTimer(item->eventInfo);
#else
				= new cRemoteTimer(item->event);
#endif
		return AddSubMenu(new cMMInputMenuEditTimer(timer));
		// Load remote timers and see if timer exists before editing
	}
	return osContinue;
}

eOSState cMMInputMenuWhatsOn::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
		case kRecord:
		case kRed:
			return Record();

		case kYellow:
			state = osBack;
		case kGreen: 
			{
				cMenuWhatsOnItem *mi = (cMenuWhatsOnItem*)Get(Current());
				if (mi) {
#if VDRVERSNUM < 10300
					m_ScheduleEventInfo = mi->eventInfo;
					m_CurrentChannel = mi->eventInfo->GetChannelNumber();
#else
					m_ScheduleEventInfo = mi->event;
					m_CurrentChannel = mi->channel->Number();
#endif
				}
			}
			break;

		case kBlue:
			return Switch();

		case kOk:
			if (Count())
#if VDRVERSNUM < 10300
				return AddSubMenu(new cMenuEvent(
						((cMenuWhatsOnItem*)Get(Current()))->eventInfo, true));
#else
				return AddSubMenu(new cMenuEvent(
						((cMenuWhatsOnItem*)Get(Current()))->event, true));
#endif
			break;

		default:
			break;
		}
	}
	return state;
}

// --- cMenuScheduleItem -----------------------------------------------------

#if VDRVERSNUM < 10300
class cMenuScheduleItem : public cOsdItem {
public:
  const cEventInfo *eventInfo;
  cMenuScheduleItem(const cEventInfo *EventInfo);
};
#else
class cMenuScheduleItem : public cOsdItem {
public:
  const cEvent *event;
  cMenuScheduleItem(const cEvent *Event);
};
#endif

// --- cMMInputMenuSchedule ------------------------------------------------

cMMInputMenuSchedule::cMMInputMenuSchedule(void):
#if VDRVERSNUM < 10300
		cOsdMenu("", 6, 6)
#else
		cOsdMenu("", 7, 6, 4)
#endif
{
	m_Now          = false;
	m_Next         = false;
	m_OtherChannel = -1;
	m_Schedules    = NULL;

	cChannel *channel = Channels.GetByNumber(cDevice::CurrentChannel());
	if (channel) {
#if VDRVERSNUM < 10300
		m_Schedules = cSIProcessor::Schedules(m_Lock);
#else
		m_Schedules = cSchedules::Schedules(m_Lock);
#endif
		PrepareSchedule(channel);
		SetHelp(Count() ? tr("Record") : NULL, tr("Now"), tr("Next"));
	}
}

cMMInputMenuSchedule::~cMMInputMenuSchedule() {
}

#if VDRVERSNUM < 10307
static int CompareEventTime(const void *p1, const void *p2) {
#if VDRVERSNUM < 10300
  return (int)((*(cEventInfo **)p1)->GetTime() 
			- (*(cEventInfo **)p2)->GetTime());
#else
  return (int)((*(cEvent**)p1)->StartTime() 
			- (*(cEvent**)p2)->StartTime());
#endif
}
#endif

void cMMInputMenuSchedule::PrepareSchedule(cChannel *Channel) {
#if VDRVERSNUM < 10300
	cTBString buffer;
	Clear();
	buffer.Format(tr("Schedule - %s"), Channel->Name());
	SetTitle(buffer);
	if (m_Schedules) {
		const cSchedule *Schedule=m_Schedules->GetSchedule(Channel->GetChannelID());
		if (Schedule) {
			int num = Schedule->NumEvents();
			const cEventInfo **pArray = MALLOC(const cEventInfo*, num);
			if (pArray) {
				time_t now = time(NULL);
				int numreal = 0;
				for (int a = 0; a < num; ++a) {
					const cEventInfo *EventInfo = Schedule->GetEventNumber(a);
					if (EventInfo->GetTime() + EventInfo->GetDuration() > now)
						pArray[numreal++] = EventInfo;
				}

				qsort(pArray, numreal, sizeof(cEventInfo*), CompareEventTime);
				for (int a = 0; a < numreal; ++a)
					Add(new cMenuScheduleItem(pArray[a]));
				free(pArray);
			}
		}
	}
#else
	Clear();
  char *buffer = NULL;
  asprintf(&buffer, tr("Schedule - %s"), Channel->Name());
  SetTitle(buffer);
  free(buffer);
  if (m_Schedules) {
     const cSchedule *Schedule = m_Schedules->GetSchedule(Channel->GetChannelID());
     if (Schedule) {
        const cEvent *PresentEvent = Schedule->GetPresentEvent(Channel->Number() == cDevice::CurrentChannel());
        time_t now = time(NULL) - Setup.EPGLinger * 60;
        for (const cEvent *Event = Schedule->Events()->First(); Event; Event = Schedule->Events()->Next(Event)) {
            if (Event->EndTime() > now || Event == PresentEvent)
               Add(new cMenuScheduleItem(Event), Event == PresentEvent);
            }
        }
     }
#endif
}

eOSState cMMInputMenuSchedule::Switch(void) {
	if (m_OtherChannel) {
		if (Channels.SwitchTo(m_OtherChannel))
			return osEnd;
	}
	ERROR(tr("Can't switch channel!"));
	return osContinue;
}

eOSState cMMInputMenuSchedule::Record(void) {
	cMenuScheduleItem *item = (cMenuScheduleItem*)Get(Current());
	if (item) {
		cRemoteTimer *timer 
#if VDRVERSNUM < 10300
				= new cRemoteTimer(item->eventInfo);
#else
				= new cRemoteTimer(item->event);
#endif
		return AddSubMenu(new cMMInputMenuEditTimer(timer));
		// Load remote timers and see if timer exists before editing
	}
	return osContinue;
}

eOSState cMMInputMenuSchedule::ProcessKey(eKeys Key) {
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
		case kRecord:
		case kRed:    
			return Record();

		case kGreen:
			if (m_Schedules) {
				if (!m_Now && !m_Next) {
					int channelnr = 0;
					if (Count()) {
						cChannel *channel 
#if VDRVERSNUM < 10300
								= Channels.GetByChannelID(
								((cMenuScheduleItem*)Get(Current()))->eventInfo->GetChannelID(),
								true);
#else
								= Channels.GetByChannelID(
								((cMenuScheduleItem*)Get(Current()))->event->ChannelID(), true);
#endif
						if (channel)
							channelnr = channel->Number();
					}
					m_Now = true;
					return AddSubMenu(new cMMInputMenuWhatsOn(m_Schedules, true, 
							channelnr));
				}
				m_Now = !m_Now;
				m_Next = !m_Next;
				return AddSubMenu(new cMMInputMenuWhatsOn(m_Schedules, m_Now, 
						cMMInputMenuWhatsOn::CurrentChannel()));
			}

		case kYellow:
			if (m_Schedules)
				return AddSubMenu(new cMMInputMenuWhatsOn(m_Schedules, false, 
						cMMInputMenuWhatsOn::CurrentChannel()));
			break;

		case kBlue:
			if (Count())
				return Switch();
			break;

		case kOk:
			if (Count())
#if VDRVERSNUM < 10300
				return AddSubMenu(new cMenuEvent(
						((cMenuScheduleItem*)Get(Current()))->eventInfo, m_OtherChannel));
#else
				return AddSubMenu(new cMenuEvent(
						((cMenuScheduleItem*)Get(Current()))->event, m_OtherChannel));
#endif
			break;

		default:
			break;
		}
	} else if (!HasSubMenu()) {
		m_Now = false;
		m_Next = false;
#if VDRVERSNUM < 10300
		const cEventInfo *ei 
#else
		const cEvent *ei
#endif
				= cMMInputMenuWhatsOn::ScheduleEventInfo();
		if (ei) {
			cChannel *channel 
#if VDRVERSNUM < 10300
					= Channels.GetByChannelID(ei->GetChannelID(), true);
#else
					= Channels.GetByChannelID(ei->ChannelID(), true);
#endif
			if (channel) {
				PrepareSchedule(channel);
				if (channel->Number() != cDevice::CurrentChannel()) {
					m_OtherChannel = channel->Number();
					SetHelp(Count() ? tr("Record") : NULL, tr("Now"), tr("Next"), 
							tr("Switch"));
				}
				Display();
			}
		}
	}
	return state;
}

// --- cMMInputMenuRecordingItem -------------------------------------------

class cMMInputMenuRecordingItem: public cOsdItem {
private:
	int   m_Total;
	int   m_New;
	char *m_FileName;
	char *m_Name;

public:
	cMMInputMenuRecordingItem(cRemoteRecording *Recording, int Level);
	virtual ~cMMInputMenuRecordingItem();

	void IncrementCounter(bool New);
	const char *Name(void) const { return m_Name; }
	const char *FileName(void) const { return m_FileName; }
	bool IsDirectory(void) const { return m_Name != NULL; }
};

cMMInputMenuRecordingItem::cMMInputMenuRecordingItem(
		cRemoteRecording *Recording, int Level) {
	m_FileName = strdup(Recording->Name());
	m_Name = NULL;
	m_Total = m_New = 0;
	SetText(Recording->Title('\t', true, Level));
	if (*Text() == '\t')
		m_Name = strdup(Text() + 2);
}

cMMInputMenuRecordingItem::~cMMInputMenuRecordingItem() {
}

void cMMInputMenuRecordingItem::IncrementCounter(bool New) {
	++m_Total;
	if (New) ++m_New;
	char *buffer = NULL;
	asprintf(&buffer, "%d\t%d\t%s", m_Total, m_New, m_Name);
	SetText(buffer, false);
}

// --- cMMInputMenuRecordings ----------------------------------------------

cRemoteRecordings cMMInputMenuRecordings::Recordings;
int cMMInputMenuRecordings::HelpKeys = -1;

cMMInputMenuRecordings::cMMInputMenuRecordings(const char *Base, int Level,
		bool OpenSubMenus):
		cOsdMenu(Base ? Base : tr("Remote Recordings"), 6, 6) {
	m_Base = Base ? strdup(Base) : NULL;
	m_Level = Setup.RecordingDirs ? Level : -1;

  Display(); // this keeps the higher level menus from showing up briefly when 
	           // pressing 'Back' during replay

	if (!Base) {
		STATUS(tr("Fetching recordings..."));
		FLUSH();
	}

	if (Base || Recordings.Load()) {
		cMMInputMenuRecordingItem *LastItem = NULL;
		char *LastItemText = NULL;
		for (cRemoteRecording *r = Recordings.First(); r; r = Recordings.Next(r)) {
			if (!Base || (strstr(r->Name(), Base) == r->Name()
						&& r->Name()[strlen(Base)] == '~')) {
				cMMInputMenuRecordingItem *Item = new cMMInputMenuRecordingItem(r, 
						m_Level);
				if (*Item->Text() && (!LastItem || strcmp(Item->Text(), LastItemText) 
						!= 0)) {
					Add(Item);
					LastItem = Item;
					free(LastItemText);
					LastItemText = strdup(LastItem->Text());
				} else
					delete Item;

				if (LastItem) {
					if (LastItem->IsDirectory())
						LastItem->IncrementCounter(r->IsNew());
				}
			}
		}
		free(LastItemText);
		if (Current() < 0)
			SetCurrent(First());
		else if (OpenSubMenus && Open(true))
			return;
	}

#if VDRVERSNUM >= 10307
	STATUS(NULL);
	FLUSH();
#endif

	SetHelpKeys();
}

cMMInputMenuRecordings::~cMMInputMenuRecordings() {
	if (m_Base != NULL) free(m_Base);
	HelpKeys = -1;
}

void cMMInputMenuRecordings::SetHelpKeys(void) {
  cMMInputMenuRecordingItem *ri =(cMMInputMenuRecordingItem*)Get(Current());
  int NewHelpKeys = HelpKeys;
  if (ri) {
     if (ri->IsDirectory())
        NewHelpKeys = 1;
     else {
        NewHelpKeys = 2;
        cRemoteRecording *recording = GetRecording(ri);
        if (recording && recording->Summary())
           NewHelpKeys = 3;
        }
     }
  if (NewHelpKeys != HelpKeys) {
     switch (NewHelpKeys) {
       case 0: SetHelp(NULL); break;
       case 1: SetHelp(tr("Open")); break;
       case 2:
       case 3: SetHelp(NULL, NULL, tr("Delete"), NewHelpKeys == 3 ? tr("Summary") : NULL);
							 //SetHelp(tr("Play"), tr("Rewind"), tr("Delete"), NewHelpKeys == 3 ? tr("Summary") : NULL); XXX
       }
     HelpKeys = NewHelpKeys;
     }
}

cRemoteRecording *cMMInputMenuRecordings::GetRecording(
		cMMInputMenuRecordingItem *Item) {
	Dprintf("looking for %s\n", Item->FileName());
  cRemoteRecording *recording = Recordings.GetByName(Item->FileName());
  if (!recording)
     ERROR(tr("Error while accessing recording!"));
  return recording;
}

bool cMMInputMenuRecordings::Open(bool OpenSubMenus) {
	cMMInputMenuRecordingItem *ri 
			= (cMMInputMenuRecordingItem*)Get(Current());

	if (ri && ri->IsDirectory()) {
		const char *t = ri->Name();
		char *buffer = NULL;
		if (m_Base) {
			asprintf(&buffer, "%s~%s", m_Base, t);
			t = buffer;
		}
		AddSubMenu(new cMMInputMenuRecordings(t, m_Level + 1, OpenSubMenus));
		if (buffer != NULL) free(buffer);
		return true;
	}
	return false;
}

eOSState cMMInputMenuRecordings::Select(void) {	
	cMMInputMenuRecordingItem *ri 
			= (cMMInputMenuRecordingItem*)Get(Current());

	if (ri) {
		if (ri->IsDirectory())
			Open();
		/*else {
			cControl::Launch(new cMMInputPlayerControl(ri->FileName()));
			return osEnd;
		} XXX */
	}
	return osContinue;
}

eOSState cMMInputMenuRecordings::Delete(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  cMMInputMenuRecordingItem *ri 
			= (cMMInputMenuRecordingItem*)Get(Current());
  if (ri && !ri->IsDirectory()) {
    if (Interface->Confirm(tr("Delete recording?"))) {
      cRemoteRecording *recording = GetRecording(ri);
      if (recording) {
        if (ClientSocket.DeleteRecording(recording)) {
          cOsdMenu::Del(Current());
          Recordings.Del(recording);
          Display();
          if (!Count())
            return osBack;
        }
      }
    }
  }
  return osContinue;
}

eOSState cMMInputMenuRecordings::Summary(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  cMMInputMenuRecordingItem *ri=(cMMInputMenuRecordingItem *)Get(Current());
  if (ri && !ri->IsDirectory()) {
    cRemoteRecording *recording = GetRecording(ri);
    if (recording && recording->Summary() && *recording->Summary())
       return AddSubMenu(new cMenuText(tr("Summary"), recording->Summary()));
  }
  return osContinue;
}

eOSState cMMInputMenuRecordings::ProcessKey(eKeys Key) {
	bool HadSubMenu = HasSubMenu();
	eOSState state = cOsdMenu::ProcessKey(Key);

	if (state == osUnknown) {
		switch (Key) {
		case kOk:     
		case kRed:    return Select();
		case kYellow: return Delete();
		case kBlue:   return Summary();
		default:      break;
		}
	}

	if (Key == kYellow && HadSubMenu && !HasSubMenu()) {
		cOsdMenu::Del(Current());
		if (!Count())
			return osBack;
		Display();
	}

	if (!HasSubMenu() && Key != kNone)
		SetHelpKeys();
	return state;
}

// --- cMMInputMenuTimerItem -----------------------------------------------

class cMMInputMenuTimerItem: public cOsdItem {
private:
	cRemoteTimer *m_Timer;

public:
	cMMInputMenuTimerItem(cRemoteTimer *Timer);
	virtual ~cMMInputMenuTimerItem();

	virtual void Set(void);

	cRemoteTimer *Timer(void) const { return m_Timer; }
};

cMMInputMenuTimerItem::cMMInputMenuTimerItem(cRemoteTimer *Timer) {
	m_Timer = Timer;
	Set();
}

cMMInputMenuTimerItem::~cMMInputMenuTimerItem() {
}

void cMMInputMenuTimerItem::Set(void) {
  char *buffer = NULL;
  asprintf(&buffer, "%c\t%d\t%s\t%02d:%02d\t%02d:%02d\t%s",
                    !m_Timer->Active() ? ' ' : 
										m_Timer->FirstDay() ? '!' : 
										/*m_Timer->Recording() ? '#' :*/ '>',
                    m_Timer->Channel()->Number(),
                    m_Timer->PrintDay(m_Timer->Day()),
                    m_Timer->Start() / 100,
                    m_Timer->Start() % 100,
                    m_Timer->Stop() / 100,
                    m_Timer->Stop() % 100,
                    m_Timer->File());
  SetText(buffer, false);
}

// --- cMMInputMenuTimers --------------------------------------------------

cMMInputMenuTimers::cMMInputMenuTimers(void):
		cOsdMenu(tr("Remote Timers"), 2, CHNUMWIDTH, 10, 6, 6) {
	Refresh();
  SetHelp(tr("Edit"), tr("New"), tr("Delete"), tr("On/Off"));
}

cMMInputMenuTimers::~cMMInputMenuTimers() {
}

eOSState cMMInputMenuTimers::ProcessKey(eKeys Key) {
	int timerNum = HasSubMenu() ? Count() : -1;
	eOSState state = cOsdMenu::ProcessKey(Key);

	if (state == osUnknown) {
		switch (Key) {
		case kOk:     return Summary();
		case kRed:    return Edit();
		case kGreen:  return New();
		case kYellow: return Delete();
		case kBlue:   OnOff(); break;
		default:      break;
		}
	}

	if (timerNum >= 0 && !HasSubMenu()) {
		Refresh();
		Display();
	}
	return state;
}

eOSState cMMInputMenuTimers::Edit(void) {
  if (HasSubMenu() || Count() == 0)
    return osContinue;
  isyslog("MMInput: Editing remote timer %d", CurrentTimer()->Index() + 1);
  return AddSubMenu(new cMMInputMenuEditTimer(CurrentTimer()));
}

eOSState cMMInputMenuTimers::New(void) {
  if (HasSubMenu())
    return osContinue;
  return AddSubMenu(new cMMInputMenuEditTimer(new cRemoteTimer, true));
}

eOSState cMMInputMenuTimers::Delete(void) {
	cRemoteTimer *ti = CurrentTimer();
	if (ti) {
		if (Interface->Confirm(tr("Delete timer?"))) {
			int idx = ti->Index();
			if (ClientSocket.DeleteTimer(ti)) {
				RemoteTimers.Del(ti);
				cOsdMenu::Del(Current());
				isyslog("MMInput: Remote timer %d deleted", idx + 1);
			}
			Refresh();
			Display();
		}
	}
	return osContinue;
}

eOSState cMMInputMenuTimers::OnOff(void) {
	cRemoteTimer *timer = CurrentTimer();
	if (timer) {
		cRemoteTimer data = *timer;
		data.OnOff();
		if (data.FirstDay())
			isyslog("MMInput: Remote timer %d first day set to %s", 
					data.Index() + 1, data.PrintFirstDay());
    else
      isyslog("MMInput: Remote timer %d %sactivated", data.Index() + 1, 
					data.Active() ? "" : "de");

		if (ClientSocket.SaveTimer(timer, data)) {
			*timer = data;
			RefreshCurrent();
			DisplayCurrent(true);
		} else {
			Refresh();
			Display();
		}
	}
	return osContinue;
}

eOSState cMMInputMenuTimers::Summary(void) {
	if (HasSubMenu() || Count() == 0)
		return osContinue;

	cRemoteTimer *ti = CurrentTimer();
	if (ti && ti->Summary() != "")
		return AddSubMenu(new cMenuText(tr("Summary"), ti->Summary().c_str()));

	return osContinue;
}

cRemoteTimer *cMMInputMenuTimers::CurrentTimer(void) {
	cMMInputMenuTimerItem *item = (cMMInputMenuTimerItem*)Get(Current());
	return item ? item->Timer() : NULL;
}

void cMMInputMenuTimers::Refresh(void) {
	Clear();
	if (RemoteTimers.Load()) {
		for (cRemoteTimer *t = RemoteTimers.First(); t; t = RemoteTimers.Next(t)) {
			Add(new cMMInputMenuTimerItem(t));
		}
	}
}
