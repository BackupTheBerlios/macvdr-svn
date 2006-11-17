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
	
	FH = 0x0;
	FH = new cFilterHandle;
	
	maxChPids = defChPids;
	chPids = 0;
	
	// only for pid test
	AllTSPkg = 0;
	SelTSPkg = 0;
	
	tunerStatus = tsIdle;
	m_Channel = new cChannel;	
	  m_Channel->SetTerrTransponderData(0, 0, 0, 0, 0, 0, 0,0,0);
}

cMMInputDevice::~cMMInputDevice() {
	Dprintf("Device gets destructed\n");
	m_Device = NULL;

	// and we're done
	giveTuner( pMM );

	[pool release];
	if(chPids != 0) delete chPids;
	errCounter = 0;
//	delete FH;
}

bool cMMInputDevice::Ready(void)
{
  return true;
}

bool cMMInputDevice::ProvidesSource(int Source) const
{
//  int type = Source & cSource::st_Mask;
return true;
/*
  return type == cSource::stNone
      || type == cSource::stCable && frontendType == FE_QAM
      || type == cSource::stSat   && frontendType == FE_QPSK
      || type == cSource::stTerr  && frontendType == FE_OFDM;
*/
}

bool cMMInputDevice::ProvidesTransponder(const cChannel *Channel) const
{
	return true;
}

bool cMMInputDevice::ProvidesChannel(const cChannel *Channel, int Priority, bool *NeedsDetachReceivers) const {
	
	printf("cMMInputDevice::ProvidesChannel(start): Device support the modulation 0x%x priority %d\n",pMM->tuningSpace(), Priority);
	bool result = false;
	bool hasPriority = Priority < 0 || Priority > this->Priority();
	bool needsDetachReceivers = false;
	
	if(ProvidesSource(Channel->Source())) {
		result = hasPriority;
//	printf("cMMInputDevice::ProvidesChannel: Device support the modulation 0x%x return value %d priority %d\n",pMM->tuningSpace(), result, Priority);
		if(Priority >= 0 && Receiving(true)) {
			if(IsTunedTo(Channel)) {
				if(Channel->Vpid() && !HasPid(Channel->Vpid()) || Channel->Apid(0) && !HasPid(Channel->Apid(0))) {
/*
					if(Ca() >= CA_ENCRYPTED_MIN || Channel->Ca() >= CA_ENCRYPTED_MIN){
						needsDetachReceivers = Ca() != Channel->Ca();
					}
					else 
*/
					if(!IsPrimaryDevice()){
					 result = true;
					 }
					else result = Priority >= Setup.PrimaryLimit;
				}
				else result = !IsPrimaryDevice() || Priority >= Setup.PrimaryLimit;
			}
			else needsDetachReceivers = true;
		}
	}
	if (NeedsDetachReceivers) *NeedsDetachReceivers = needsDetachReceivers;
	printf("cMMInputDevice::ProvidesChannel(stop): Device support the modulation 0x%x return value %d priority %d\n",pMM->tuningSpace(), result, Priority);
	return result;
}

/*
		The most of code inside this function is a copy from dvbdevice
*/
bool cMMInputDevice::SetChannelDevice(const cChannel *Channel, 
									  bool LiveView) {
	
/*
  bool TurnOffLivePIDs = HasDecoder()
                         && (DoTune
                            || !IsPrimaryDevice()
                            || LiveView // for a new live view the old PIDs need to be turned off
                            || pidHandles[ptVideo].pid == Channel->Vpid() // for recording the PIDs must be shifted from DMX_PES_AUDIO/VIDEO to DMX_PES_OTHER
                            );

  bool StartTransferMode = IsPrimaryDevice() && !DoTune
                           && (LiveView && HasPid(Channel->Vpid() ? Channel->Vpid() : Channel->Apid(0)) && (pidHandles[ptVideo].pid != Channel->Vpid() || (pidHandles[ptAudio].pid != Channel->Apid(0) && (Channel->Dpid(0) ? pidHandles[ptAudio].pid != Channel->Dpid(0) : true)))// the PID is already set as DMX_PES_OTHER
                              || !LiveView && (pidHandles[ptVideo].pid == Channel->Vpid() || pidHandles[ptAudio].pid == Channel->Apid(0)) // a recording is going to shift the PIDs from DMX_PES_AUDIO/VIDEO to DMX_PES_OTHER
                              );

  bool TurnOnLivePIDs = HasDecoder() && !StartTransferMode && LiveView;
*/
  // Set the tuner:
	tunerStatus = tsSet;
	if (myTune( pMM, Channel) == true) tunerStatus = tsTuned;
	else tunerStatus = tsIdle;

	if(tunerStatus == tsTuned){

	// check if array to store the pids exists
	if(chPids != 0x0){
		delete chPids;
		chPids = 0x0;
		maxChPids = defChPids;
	}
	
//	printf("SetChannelDevice Channel: %s, LiveView: %s\n", m_Channel->Name(),
//		   LiveView ? "true" : "false");
	}

	// If this channel switch was requested by the EITScanner we don't wait for
	// a lock and don't set any live PIDs (the EITScanner will wait for the lock
	// by itself before setting any filters):

//	if(EITScanner.UsesDevice(this)) return true; //XXX
//	printf("tune channel %d\n",tunerStatus);
//	printf("cMMInputDevice::SetChannelDevice: Device support the modulation 0x%x\n",pMM->tuningSpace());

	return true;
}

// not cards with an own vido or audi decoder are supported decoder
bool cMMInputDevice::HasDecoder(void) const {
  return false;
//   return fd_video >= 0 && fd_audio >= 0;
}

// This driver don't support any Channels thats provides conditional access 
int cMMInputDevice::ProvidesCa(const cChannel *Channel) const{
	return 0;
}

bool cMMInputDevice::IsTunedTo(const cChannel *Channel) const {
  return tunerStatus != tsIdle && m_Channel->Source() == Channel->Source() && m_Channel->Transponder() == Channel->Transponder();
}

bool cMMInputDevice::SetPid(cPidHandle *Handle, int Type, bool On) {
	
	printf("MMInput device: SetPid, Pid=%d\n", Handle->pid);
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
/*
	printf("device: SetPid: pids: ");
			for(int i = 0; i < maxChPids; i++){
				printf(" %d",chPids[i]);
			}
	printf("\n");
*/
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

	TSPacketCounter++;
	return true;

	TSHeader & tsh = *(TSHeader*)Data;
	if (tsh.syncByte != 0x47){
		Data = NULL;
		return true;
	 }
	int pid = tsh.pid;

//	TSPacketCounter++;
//	return true;

	FH->Process(Data);
	Data = NULL;
//	AllTSPkg++;
	for(int i = 0; i < maxChPids; i++ ){
		if(chPids[i] == pid){
			Data = &((uchar*)m_blobDate)[(TSPacketCounter)*188];
//			SelTSPkg++;
			break;
		}
	}
	/*
	if(Data == NULL){
		if((errCounter %2001) == 0){
			printf("device no valid TS packet\n");
			printf("\tpids: ");
			for(int i = 0; i < maxChPids; i++ ){
				printf(" %d",chPids[i]);
			}	
			printf(" \n");
		}
		errCounter++;
	}
	*/
	TSPacketCounter++;

	return true;
}

#if VDRVERSNUM >= 10300
int cMMInputDevice::OpenFilter(u_short Pid, u_char Tid, u_char Mask) {
//	printf("OpenFilter pid %x, Tid %x Mask %x\n",Pid, Tid, Mask);
	if(FH == 0x0){
	printf("Error! No Filter Handler exist!\n");
	return -1;
	}
	return FH->CreateFilter(Pid, Tid);
//	return -1;
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

bool cMMInputDevice::myTune( MMInputDevice * pMM, const cChannel *Channel){
	
	if(tunerStatus == tsSet){
		tunerStatus = tsTuned;
		if(m_Channel->Bandwidth() != Channel->Bandwidth())			tunerStatus = tsSet;
		if(m_Channel->Inversion() != Channel->Inversion())			tunerStatus = tsSet;
		if(m_Channel->CoderateH() != Channel->CoderateH())			tunerStatus = tsSet;
		if(m_Channel->CoderateL() != Channel->CoderateL())			tunerStatus = tsSet;
		if(m_Channel->Modulation() != Channel->Modulation())		tunerStatus = tsSet;
		if(m_Channel->Hierarchy() != Channel->Hierarchy())			tunerStatus = tsSet;
		if(m_Channel->Transmission() != Channel->Transmission())	tunerStatus = tsSet;
		if(m_Channel->Guard() != Channel->Guard())					tunerStatus = tsSet;
		if(m_Channel->Frequency() != Channel->Frequency())			tunerStatus = tsSet;
		
		if(tunerStatus == tsSet){
			[pDict setObject:[NSNumber numberWithBool:Channel->Inversion()] forKey:@"Inversion"];
			
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Bandwidth()*1000000] forKey:@"BandwidthHz"];
			
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->CoderateH()] forKey:@"CodeRateHP"];
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->CoderateL()] forKey:@"CodeRateLP"];
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Modulation()] forKey:@"Constellation"];
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Hierarchy()] forKey:@"Hierarchy"];
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Transmission()*1000] forKey:@"TransmissionMode"];
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Guard()] forKey:@"GuardInterval"];	
			[pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Frequency()*1000] forKey:@"FrequencyHz"];
			
			NSDictionary * pConstDict = pDict;
			
			printf( "tuning dictionary: %s\n", [[pConstDict description] lossyCString] );
			
			// tune to those parameters
			//	fprintf( stderr, "Tuning to: %s\n", [[pDict description] cString] );
			if (!pMM->tune( pDict ))
			{
				fprintf( stderr, "Error: Could not tune to those parameters\n" );
				giveTuner( pMM );
				return false;
			}
			
			/*
			assume that we have managed on the moment only terrastical device
			*/
			printf("store channel values\n");
			  if( m_Channel->SetTerrTransponderData(
				Channel->Source(), 
				Channel->Frequency(), 
				Channel->Bandwidth(), 
				Channel->Modulation(), 
				Channel->Hierarchy(), 
				Channel->CoderateH(), 
				Channel->CoderateL(), 
				Channel->Guard(), 
				Channel->Transmission()) == false){
			  printf("Error! Couldn't store tuner parameter\n");
			  }

		}
	}
	return true;
}


