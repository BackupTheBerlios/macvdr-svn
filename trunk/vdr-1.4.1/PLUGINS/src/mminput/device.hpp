/*
 *  $Id: device.h,v 1.3 2005/02/08 15:21:19 lordjaxom Exp $
 */
 
#ifndef VDR_MMINPUTDEV_DEVICE_H
#define VDR_MMINPUTDEV_DEVICE_H

#include <vdr/device.h>

#include "Drivers/MMInputFamily/MMInputLib.hpp"

#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSEnumerator.h>

#include "FilterHandle.hpp"

class cTBString;

#define CMD_LOCK_OBJ(x) cMutexLock CmdLock((cMutex*)&(x)->m_Mutex)
#define MAXDEVICEFilter 16

class cMMInputDevice: public cDevice {
	friend class cRemoteRecordings;

private:
	const cChannel      *m_Channel;
	cTSBuffer           *m_TSBuffer;

	static cMMInputDevice *m_Device;
	int s_mmindex;
	MMInputDevice* pMM;
	UInt32 TSPackets;
	UInt32 TSPacketCounter;
	UInt32 blobSize;
	void* m_blobDate;
	NSMutableDictionary * pDict; // tuningparameters
	cFilterHandle* FH;
protected:
	virtual bool SetChannelDevice(const cChannel *Channel, bool LiveView);
	virtual bool HasLock(int TimeoutMs) 
	{
		//printf("HasLock is %d\n", (ClientSocket.DataSocket(siLive) != NULL));
		//return ClientSocket.DataSocket(siLive) != NULL;
		return true;
	}

	virtual bool SetPid(cPidHandle *Handle, int Type, bool On);
	virtual bool OpenDvr(void);
	virtual void CloseDvr(void);
	virtual bool GetTSPacket(uchar *&Data);

#if VDRVERSNUM >= 10300
//	virtual int OpenFilter(u_short Pid, u_char Tid, u_char Mask);
#endif

public:
	cMMInputDevice(void);
	virtual ~cMMInputDevice();

	virtual bool ProvidesSource(int Source) const;
	virtual bool ProvidesTransponder(const cChannel *Channel) const;
	virtual bool ProvidesChannel(const cChannel *Channel, int Priority = -1,
			bool *NeedsDetachReceivers = NULL) const;
	virtual	bool cMMInputDevice::Ready(void);
//	virtual int ProvidesCa(const cChannel *Channel) const;
	virtual int OpenFilter(u_short Pid, u_char Tid, u_char Mask);

	static bool Init(void);
	static bool ReInit(void);
	
	MMInputDevice * grabTuner( int mmindex );
	void giveTuner( MMInputDevice * pMM );
	bool myTune( MMInputDevice * pMM );
	
	static cMMInputDevice *GetDevice(void) { return m_Device; }
};

static char * spectra[] = {
	/*[MMInput::NONE] =*/ "Untunable",
	/*[MMInput::FREQ] =*/ "Generic Frequency",
	/*[MMInput::ODFM] =*/ "Terrestrial (ODFM)",
	/*[MMInput::QPSK] =*/ "Satellite (QPSK)",
	/*[MMInput::ODFM] =*/ "Terrestrial (VSB)"
};

static char * dkinds[] = {
	/*[MMInput::DVB]  =*/ "DVB",
	/*[MMInput::ATSC] =*/ "ATSC"
};		// that's all we support


#endif // VDR_MMINPUTDEV_DEVICE_H