/*
 *  $Id: device.c,v 1.6 2005/04/24 16:21:59 lordjaxom Exp $
 */
 
#include "device.hpp"
#include "setup.hpp"
#include "menu.hpp"

#include "DPConnect/TSHandler.hpp"
#include "DPConnect/Streaming.hpp"

#include <vdr/channels.h>
#include <vdr/ringbuffer.h>
#include <vdr/eit.h>
#include <vdr/timers.h>

#include <time.h>
#include <iostream>

using namespace std;
#define DEBUG

#define VIDEOBUFSIZE MEGABYTE(3)

static NSAutoreleasePool * pool = 0;

cMMInputDevice *cMMInputDevice::m_Device = NULL;

cMMInputDevice::cMMInputDevice(void) {
	m_Channel    = NULL;

	s_mmindex = 0;	
	TSPacketCounter = 0;
	TSPackets = 0;
	pool = [[NSAutoreleasePool alloc] init];
	pMM = grabTuner( 0 ); // only for the first time. If a device is present we take the first one.
	pDict = [NSMutableDictionary dictionaryWithCapacity:100];
	if(pMM != 0){
//	        ciHandler = cCiHandler::CreateCiHandler("MMInupt_Device");
	}
	m_Device = this;
	StartSectionHandler();
//	FH = new cFilterHandle;
	
	maxChPids = defChPids;
	chPids = 0;
	
	// only for pid test
	AllTSPkg = 0;
	SelTSPkg = 0;
}

cMMInputDevice::~cMMInputDevice() {
	Dprintf("Device gets destructed\n");
	m_Device = NULL;

	// and we're done
	giveTuner( pMM );

	[pool release];
	if(chPids != 0) delete chPids;
//	delete FH;
}

bool cMMInputDevice::Ready(void)
{
  return true;
}

bool cMMInputDevice::ProvidesSource(int Source) const {
	Dprintf("ProvidesSource, Source=%d\n", Source);
	return true;
}

bool cMMInputDevice::ProvidesTransponder(const cChannel *Channel) const
{
	return true;
}

bool cMMInputDevice::ProvidesChannel(const cChannel *Channel, int Priority, 
		bool *NeedsDetachReceivers) const {
	bool res = false;
	bool ndr = false;
	printf("ProvidesChannel, Channel=%s, Prio=%d\n", Channel->Name(), Priority);
	ndr = true;
	res = true;
//	if (NeedsDetachReceivers) *NeedsDetachReceivers = ndr;
	*NeedsDetachReceivers = true;
	
	return res;
}

bool cMMInputDevice::SetChannelDevice(const cChannel *Channel, 
									  bool LiveView) {
	m_Channel = Channel;
	// check if array to store the pids exists
	if(chPids != 0x0){
		delete chPids;
		chPids = 0x0;
		maxChPids = defChPids;
	}
	printf("SetChannelDevice Channel: %s, LiveView: %s\n", m_Channel->Name(),
		   LiveView ? "true" : "false");
	return myTune( pMM );
	// tune device
	
}


bool cMMInputDevice::SetPid(cPidHandle *Handle, int Type, bool On) {
	
	printf("MMInput device: SetPid, Pid=%d", Handle->pid);
	// check if array, to store the pids for one channel exist allready
	// if not create one witth default size
	
	if(chPids == 0){
		chPids = new int[maxChPids];
		for(int i = 0; i < maxChPids; i++){
			chPids[i] = -1;	
		}
	}
	
	if(chPids == 0) return false;
	
	bool store = false;
	for(int i = 0; i < maxChPids; i++){
		if(chPids[i] == Handle->pid){
			store = true;
			break;
		}
		if(chPids[i] == -1){
			chPids[i] = Handle->pid;
			store = true;
			break;
		}
	}
	
	// check if the array size is to small
	if(store == false){
		int *chPidsTmp = 0x0;
		chPidsTmp = new int[++maxChPids];
		if(chPidsTmp == 0){
			printf("Error. Couldn't resize array to store pids\n");
			return false;
		}
		else{
			for(int i = 0; i < maxChPids; i++){
				chPidsTmp[i] = chPids[i];
			}
			// delete old array
			delete chPids;
			// store new pointer 
			chPids = chPidsTmp;
		}
		chPids[maxChPids-1] = Handle->pid;
	}
	/*	
	printf("MMInput device: SetPid, Pid=%d", Handle->pid);
		switch(Type){
			case(ePidType::ptAudio):{
				printf(", Type= Audio");
				break;}
			case(ePidType::ptVideo):{
				printf(", Type= video");
				break;}
			case(ePidType::ptPcr):{
				printf(", Type= Pcr");
				break;}
			case(ePidType::ptTeletext):{
				printf(", Type= Teletext");
				break;}
			case(ePidType::ptDolby):{
				printf(", Type= Dolby");
				break;}
			default:{
				printf(", Type= Other");
			}
				printf("On=%d, used=%d\n",On, Handle->used);
				return true;
				*/

	printf("device: SetPid: pids: ");
			for(int i = 0; i < maxChPids; i++){
				printf(" %d",chPids[i]);
			}
	printf("\n");
		return true;
}

bool cMMInputDevice::OpenDvr(void) {
	printf("OpenDvr\n");
	CloseDvr();
	if (!pMM->fill( true )){
		printf("Error: Could not start streaming...\n");
		pMM->deactivate();
		pMM->release();
		return false;
	}

	return true;
}

void cMMInputDevice::CloseDvr(void) {
	printf("CloseDvr\n");
	pMM->fill( false );
}

bool cMMInputDevice::GetTSPacket(uchar *&Data) {
	//	printf("cMMInputDevice::GetTSPacket: length=%d, count=%d\n",blobSize,TSPackets);
	if(TSPacketCounter == TSPackets){	
		//		TSPackets=0;
		TSPackets = pMM->retrieve( m_blobDate, blobSize );
//				printf("cMMInputDevice::GetTSPacket: length=%d, count=%d\n",blobSize,TSPackets);
//				TSPackets=0;
		TSPacketCounter = 0;
		if(TSPackets == 0) usleep(83000);
	}
	if(TSPackets == 0){
		Data = NULL;
//		printf("pid filter rejection: %f\n", (100.0*(AllTSPkg-SelTSPkg)/AllTSPkg));
		return true;
	};

	Data = &((uchar*)m_blobDate)[(TSPacketCounter)*188];

//	TSPacketCounter++;
//	return true;

	TSHeader & tsh = *(TSHeader*)Data;
	if (tsh.syncByte != 0x47){
		Data = NULL;
		return true;
	 }
	int pid = tsh.pid;

//	TSPacketCounter++;
//	return true;

	Data = NULL;
//	AllTSPkg++;
	for(int i = 0; i < maxChPids; i++ ){
		if(chPids[i] == pid){
			Data = &((uchar*)m_blobDate)[(TSPacketCounter)*188];
//			SelTSPkg++;
			break;
		}
	}
	TSPacketCounter++;

//	return FH->Process(Data);
	return true;
}

#if VDRVERSNUM >= 10300
int cMMInputDevice::OpenFilter(u_short Pid, u_char Tid, u_char Mask) {
	printf("OpenFilter pid %x, Tid %x Mask %x\n",Pid, Tid, Mask);
//	FH->CreatePipe(Pid, Tid);
	return -1;
}
#endif

bool cMMInputDevice::Init(void) {
	if (m_Device == NULL)
		new cMMInputDevice;
	return true;
}

bool cMMInputDevice::ReInit(void) {
	return Init();
}

MMInputDevice * cMMInputDevice::grabTuner( int mmindex )
{
	// access it
	MMInputDevice * pMM = MMInputDevice::withIndex( mmindex );

	if (pMM == NULL)
	{
		fprintf( stderr, "Error: No device index %d.\n", mmindex );
		return NULL;
	}
	fprintf( stderr, "Accessing device index %d...\n", mmindex );

	// get its details
	int tspace = pMM->tuningSpace() & 63;
	MMInput::DataKind datakind = pMM->dataKind();
	fprintf( stderr, "It is a %s %s device.\n",
		tspace < (int)(sizeof(spectra)/sizeof(*spectra)) ? spectra[tspace] : "Unknown",
		datakind < (int)(sizeof(dkinds)) ? dkinds[datakind] : "Unsupported" );

	// activate it
	fprintf( stderr, "Activating device...\n" );
	if (pMM->activate() != 0)
	{
		fprintf( stderr, "Error: Could not activate device index %d.\n", mmindex );
		pMM->release();
		return NULL;
	}
	fprintf( stderr, "Activated device index %d.\n", mmindex );

	s_mmindex = mmindex;
	return pMM;
}

void cMMInputDevice::giveTuner( MMInputDevice * pMM )
{
	pMM->deactivate();
	pMM->release();
}

bool cMMInputDevice::myTune( MMInputDevice * pMM )
{

	[pDict setObject:[NSNumber numberWithBool:m_Channel->Inversion()] forKey:@"Inversion"];
	
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Bandwidth()*1000000] forKey:@"BandwidthHz"];
	
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->CoderateH()] forKey:@"CodeRateHP"];
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->CoderateL()] forKey:@"CodeRateLP"];
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Modulation()] forKey:@"Constellation"];
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Hierarchy()] forKey:@"Hierarchy"];
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Transmission()*1000] forKey:@"TransmissionMode"];
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Guard()] forKey:@"GuardInterval"];	
	[pDict setObject:[NSNumber numberWithUnsignedLong:m_Channel->Frequency()*1000] forKey:@"FrequencyHz"];

//	NSDictionary * pConstDict = pDict;
	
	//printf( "tuning dictionary: %s\n", [[pConstDict description] lossyCString] );
	
		// tune to those parameters
//	fprintf( stderr, "Tuning to: %s\n", [[pDict description] cString] );
	if (!pMM->tune( pDict ))
	{
		fprintf( stderr, "Error: Could not tune to those parameters\n" );
		giveTuner( pMM );
		return false;
	}
	return true;
}


