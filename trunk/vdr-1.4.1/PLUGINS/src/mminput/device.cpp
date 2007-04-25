/*
 *  $Id: device.cpp,v 0.0.1 2007/02/08 Stefan Rieke
 */

#include "device.hpp"
#include "setup.hpp"
#include "menu.hpp"

#include <vdr/channels.h>
#include <vdr/ringbuffer.h>
#include <vdr/eit.h>
#include <vdr/timers.h>

#include <vdr/macosfrontend.h>

#include "MMInput.hpp"

#include <time.h>
#include <iostream>

using namespace std;
#define DEBUG

#define VIDEOBUFSIZE MEGABYTE(3)

//static NSAutoreleasePool * pool = 0;

cMMInputDevice *cMMInputDevice::m_Device = NULL;

cMMInputDevice::cMMInputDevice() {
	m_Channel    = NULL;
        //	AllTSPacket = 0;
        
	_mmindex = -1;	
	TSPacketCounter = 0;
	TSPackets = 0;
	pool = [[NSAutoreleasePool alloc] init];
	printf("cMMInputDevice::cMMInputDevice: create with index %d\n",_mmindex);
        //	pMM = grabTuner( _mmindex ); // only for the first time. If a device is present we take the first one.
	pDict = [NSMutableDictionary dictionaryWithCapacity:100];
        
	_activeTuner = false;
        
	m_Device = this;
        
	
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
	CloseDvr();
	StartSectionHandler();
	printf("Device gets constructed\n");
}

cMMInputDevice::~cMMInputDevice() {
	printf("Device gets destructed\n");
	m_Device = NULL;
        
	// and we're done
	giveTuner( pMM );
        
	[pool release];
	if(chPids != 0) delete chPids;
	errCounter = 0;
        //	delete FH;
}

bool cMMInputDevice::Ready(void){
        return true;
}

bool cMMInputDevice::ProvidesSource(int Source) const{
        if(_activeTuner == false) return false;
        //	printf("cMMInputDevice::ProvidesSource\n");
	int type = Source & cSource::st_Mask;
        bool result (type == cSource::stTerr);
	printf("cMMInputDevice::ProvidesSource %d\n",result);
        //  bool result = ( (type == cSource::stNone)
        //		|| (type == cSource::stCable && MMInput::QAM == pMM->tuningSpace())
        //		|| (type == cSource::stSat   && MMInput::QPSK == pMM->tuningSpace())
        //		|| (type == cSource::stTerr  && MMInput::OFDM == pMM->tuningSpace()) );
        return result;
}

bool cMMInputDevice::ProvidesTransponder(const cChannel *Channel) const
{
        //	printf("cMMInputDevice::ProvidesTransponder\n");
        if(_activeTuner == false) return false;
	return true;
}

bool cMMInputDevice::ProvidesChannel(const cChannel *Channel, int Priority, bool *NeedsDetachReceivers) const {
        if(_activeTuner == false){
                //                printf("tuner is not activated\n");
                return false;
	}
        
	bool result = false;
	bool hasPriority = Priority < 0 || Priority > this->Priority();
	bool needsDetachReceivers = false;
	if(ProvidesSource(Channel->Source())){
		result = hasPriority;
                
		if(Priority >= 0 && Receiving(true)) {
			needsDetachReceivers = false;
			if(IsTunedTo(Channel)){
                                result = true;	
                        }
                        else{
                                needsDetachReceivers = true;
                        }
		}
	}
	else{ 
                printf("cMMInputDevice::ProvidesChannel is false\n");
		result = false;
	}
	
	if(NeedsDetachReceivers) *NeedsDetachReceivers = needsDetachReceivers;
        
        printf("cMMInputDevice::ProvidesChannel is %d, device index: %d\n",result,_mmindex);
	return result;
}

/*
 The most of code inside this function is a copy from dvbdevice
 */
bool cMMInputDevice::SetChannelDevice(const cChannel *Channel, 
                                      bool LiveView) {
        if(_activeTuner == false) return false;
        
        // Set the tuner:
	tunerStatus = tsSet;
	printf("cMMInputDevice::SetChannelDevice: livetune: %d\n",LiveView);
	if (myTune( pMM, Channel) == true) tunerStatus = tsTuned;
	else{
		tunerStatus = tsIdle;
                return false;
        }
        
	return true;
}

uint cMMInputDevice::getSNR(void) const{
	UInt32 progress = 0;
	UInt32 strength = 0;
	UInt32 quality = 0;
	pMM->situation( progress, strength, quality );
	return quality;
}

uint cMMInputDevice::getSignal(void) const{
	UInt32 progress = 0;
	UInt32 strength = 0;
	UInt32 quality = 0;
	pMM->situation( progress, strength, quality );
	return strength;
}

uint cMMInputDevice::getStatus(void) const{
	UInt32 progress = 10;
	UInt32 strength = 10;
	UInt32 quality = 10;
	pMM->situation( progress, strength, quality );
        /*
         printf("cMMInputDevice::getStatus: value of progress is: 0x%x\n",progress);
         printf("cMMInputDevice::getStatus: value of strength is: 0x%x\n",strength);
         printf("cMMInputDevice::getStatus: value of quality is: 0x%x\n",quality);
         */
	return progress;
}

bool cMMInputDevice::HasLock(int TimeoutMs){
	
	if(getStatus() < 0xffff){
		usleep(TimeoutMs*1000);
		if(getStatus() < 0xffff) return false;
	}
        return true;
}

bool cMMInputDevice::HasDecoder(void) const
{
        return false;
}

int cMMInputDevice::ProvidesCa(const cChannel *Channel) const{
	return 0;
}

bool cMMInputDevice::IsTunedTo(const cChannel *Channel) const {
        if(_activeTuner == false) return false;
        
        return tunerStatus != tsIdle && m_Channel->Source() == Channel->Source() && m_Channel->Transponder() == Channel->Transponder();
}

bool cMMInputDevice::SetPid(cPidHandle *Handle, int Type, bool On) {
        if(_activeTuner == false) return false;
	
	printf("MMInput device: SetPid, Pid=%d type %d on %d\n", Handle->pid,Type,On);
	// check if array, to store the pids for one channel exist allready
	// if not create one witth default size
	return true;
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
        return true;
}

bool cMMInputDevice::OpenDvr(void) {
        if(_activeTuner != false){
                printf("OpenDvr\n");
                CloseDvr();
                if (!pMM->fill( true )){
                        printf("Error: Could not start streaming...\n");
                        pMM->deactivate();
                        pMM->release();
                        return false;
                }
        }
	return true;
}

void cMMInputDevice::CloseDvr(void) {
	if(_activeTuner != false){
		printf("CloseDvr\n");
		pMM->fill( false );
	}
}

uint16 GetPid(uint8_t *tspkt) {
        return (((tspkt[1] & 0x1f) << 8) + tspkt[2])&0x1ff;
};

static inline void Dump(uint8_t *data) {
        int i=0;
        while (i<20)
                printf("%02x ",data[i++]);
        printf("\n");
};

bool cMMInputDevice::GetTSPacket(uchar *&Data) {
	if(_activeTuner == false){
		Data = NULL;
		return true;
	}
        /*
         if(MMInputDevice::withIndex( _mmindex ) == 0x0){
                 printf("no device present\n");
                 _activeTuner = false;
         }
         _activeTuner = true;
         */
	if(TSPacketCounter == TSPackets){	
		TSPackets = pMM->retrieve( m_blobDate, blobSize );
		TSPacketCounter = 0;
		if(TSPackets == 0) usleep(83000);
	}
	if(TSPackets == 0){
		Data = NULL;
		return true;
	};
        
	Data = &((uchar*)m_blobDate)[(TSPacketCounter)*188];
        // Dump(Data);
        // do we have a valid ts packet?
	if(Data[0] != 0x47){
                //		printf("cMMInputDevice::GetTSPacket: sync byte wrong %x \n",Data[0]);
		Data = NULL;
		TSPacketCounter++;
		return true;
        }
        
	uint16 pid = GetPid(Data);
        
        //	if(pid != 0x0){ // temporary fix, because in device.c  the unused pids has the value 0x0
        if(HasPid(pid) == true){
                TSPacketCounter++;
                return true;
        }
        //	}
	
	if( FH->Process(Data)  == 1){
		Data = NULL;
		TSPacketCounter++;
		return true;		
	}
	FH->Process(Data);
	TSPacketCounter++;
	return true;
        
        
}

#if VDRVERSNUM >= 10300
int cMMInputDevice::OpenFilter(u_short Pid, u_char Tid, u_char Mask) {
        //	printf("OpenFilter pid 0x%x, Tid 0x%x Mask 0x%x\n",Pid, Tid, Mask);
	if(FH == 0x0){
                printf("Error! No Filter Handler exist!\n");
                return -1;
	}
        //	clog << "call filter handle" << endl;
        //return -1;
	return FH->CreateFilter(Pid, Tid);
}
#endif

cMMInputDevice* cMMInputDevice::Init() {
	printf("cMMInputDevice::Init\n");
        
        if (UseDevice(NextCardIndex())) {
                m_Device = new cMMInputDevice();
                return m_Device;
        }
        return NULL;
}

bool cMMInputDevice::ReInit(void) {
	return Init();
}

bool cMMInputDevice::SetMMInputDevice(MMInputDevice * _pMM, const int mmindex){
	if (_pMM == NULL){
		fprintf( stderr, "Error: No device found.\n");
		return false;
	}
        
	this->pMM = _pMM;
        this->_mmindex = mmindex;
	printf("Accessing device ... index %d\n",_mmindex);
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
		fprintf( stderr, "cMMInputDevice::SetMMInputDevice: Error: Could not activate device.\n");
		pMM->release();
		return false;
         }

	fprintf( stderr, "Activated device.\n");
        
	sleep(1);
	_activeTuner = true;
	return true;
}

void cMMInputDevice::giveTuner( MMInputDevice * pMM ){
	if(_activeTuner == false){
		pMM->deactivate();
		pMM->release();
	}	
}

bool cMMInputDevice::myTune( MMInputDevice * pMM, const cChannel *Channel){
        //	printf("cMMInputDevice::myTune\n");
	if(_activeTuner == false) return false;
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        if(IsTunedTo(Channel) == false){
                printf("cMMInputDevice::myTune: set new channel. Device index; %d\n",_mmindex);
                tunerStatus = tsSet;
                
                [pDict setObject:[NSNumber numberWithBool:Channel->Inversion()] forKey:@"Inversion"];
                
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Bandwidth()*1000000] forKey:@"BandwidthHz"];
                
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->CoderateH()] forKey:@"CodeRateHP"];
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->CoderateL()] forKey:@"CodeRateLP"];
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Modulation()] forKey:@"Constellation"];
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Hierarchy()] forKey:@"Hierarchy"];
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Transmission()*1000] forKey:@"TransmissionMode"];
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Guard()] forKey:@"GuardInterval"];	
                [pDict setObject:[NSNumber numberWithUnsignedLong:Channel->Frequency()*1000] forKey:@"FrequencyHz"];
                
                /*			
                        NSDictionary * pConstDict = pDict;
                
		printf( "tuning dictionary: %s\n", [[pConstDict description] lossyCString] );
                */			
                
                // tune to those parameters
                //	fprintf( stderr, "Tuning to: %s\n", [[pDict description] cString] );
                printf("cMMInputDevice::myTune: tune device\n");
                if (!pMM->tune( pDict )){
                        fprintf( stderr, "Error: Could not tune to those parameters\n" );
                        giveTuner( pMM );
                        return false;
                }
                
                /*
                 assume that we have managed on the moment only terrastical device
                 */
                //			printf("store channel values\n");
                
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
                //		m_Channel->SetId(Channel->Nid(),Channel->Tid(),Channel->Sid(),Channel->Rid());
        }
        [pool release];
        return true;
        
}