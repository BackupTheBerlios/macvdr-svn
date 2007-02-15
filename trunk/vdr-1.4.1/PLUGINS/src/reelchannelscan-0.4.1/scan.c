/********************************************************************************
 * scan.c
 * provides scanning through given tansponder lists
 * writen by reel-multimedia
 *
 * Contact to mhahn@reel-multimedia.com
 ********************************************************************************/

#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <vdr/device.h>
#include <vdr/sources.h>
#include <vdr/player.h>
#include <vdr/receiver.h>
#include "channelscan.h"
#include "csmenu.h"
#include "scan.h"

//#include <linux/dvb/frontend.h>
#include <vdr/macosfrontend.h>

// make scan delay  
#define SCAN_DELAY 5
#define DVBS_LOCK_TIMEOUT 4000

#define DBG " Channelscan DEBUG: -- "
#define DLOG(x...) dsyslog(x)

bool scanning_on_receiving_device = false;

using std::list;
using std::cout;

//-------------------------------------------------------------------------
class cDummyReceiver: public cReceiver {
public:
         cDummyReceiver() : cReceiver(0,99,007) {
         };
         virtual ~cDummyReceiver() {};         
         virtual void Receive(uchar *Data, int Length) {
         };
 };

//-------------------------------------------------------------------------

class cDummyReplay: public cPlayer {
 private:
        bool Active;
 protected:
       virtual void Activate(bool On) {
                printf("dummy device Activate %d \n",On);
                Active=On;
       }
 public:
        cDummyReplay() :cPlayer() {
                printf("Dummy device created\n");
        };
        virtual ~cDummyReplay() {
                printf("Dummy replay ended!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                Detach();
        };
};

class cDummyCtl: public cControl {
 private:
        cDummyReplay *dummyReplay;
 public:
        cDummyCtl() : cControl( dummyReplay=new cDummyReplay() ) {};
        virtual ~cDummyCtl() {
                };
        virtual void Hide() {};
        void Stop() {
                printf("DummyCtl Stop\n");
                delete dummyReplay;
        };
      
        virtual eOSState ProcessKey(eKeys Key) {
                return osContinue;
        };
};

 
//-------------------------------------------------------------------------

cScan::cScan()
{
   origUpdateChannels = Setup.UpdateChannels;
   ::Setup.UpdateChannels = 4;
   cTransponders &transponders = cTransponders::GetInstance();
   sourceCode = transponders.SourceCode();
   newChannels = 0;
   fd_frontend= -1;
   cardnr = -1;
   transponderNr = 0;
   channelNumber = 0;
   frequency = 0;
   foundNum=0;

   PFilter=NULL;
   SFilter=NULL;
   EFilter=NULL;
   dummyCtl=NULL;
}

//--------- Destructor ~cScan -----------------------------------

cScan::~cScan()
{
  ::Setup.UpdateChannels =  origUpdateChannels;
  //worst case
  cMenuChannelscan::scanning = false;
  Cancel(5000);

  if (fd_frontend > 0)
     close(fd_frontend);
  
  scanning_on_receiving_device = false;
  //scanning_by_channelscan = false;

  if (PFilter)
    cDevice::GetDevice(cardnr)->Detach(PFilter);
  if (SFilter)
    cDevice::GetDevice(cardnr)->Detach(SFilter);

  if (EFilter)
    cDevice::GetDevice(cardnr)->Detach(EFilter);

  PFilter=NULL;
  SFilter=NULL;
  EFilter=NULL;
}

//-------------------------------------------------------------------------
uint16_t cScan::getSNR(){
	device = cDevice::GetDevice(cardnr);
	return device->getSNR();

}

uint16_t cScan::getStatus(){
	device = cDevice::GetDevice(cardnr);
	return device->getStatus();

}

uint16_t cScan::getSignal(){
	device = cDevice::GetDevice(cardnr);
	return device->getStatus();

}

//-------------------------------------------------------------------------

void cScan::ScanServices()
{

     // to recieve filters, the mminput device needs recievers attached...
     cDummyReceiver *dummyReceiver = new cDummyReceiver();
     device->AttachReceiver(dummyReceiver);

    EFilter = new cEitFilter();
    PFilter = new PatFilter();
    SFilter = new SdtFilter(PFilter);
    PFilter->SetSdtFilter(SFilter);

    device->AttachFilter(SFilter);
    device->AttachFilter(PFilter);
    device->AttachFilter(EFilter);

    time_t start=time(NULL);

    int foundSids=0;
    foundNum=totalNum=0;
    // Heuristic: Delay scan timeout if Sids or Services withs PIDs are found
//	printf("cScan::ScanServices: start time %d ist time %d SCAN_DELAY %d\n",start, time(NULL), SCAN_DELAY);
		while(!PFilter->EndOfScan() && (
              (time(NULL) - start < SCAN_DELAY && cMenuChannelscan::scanning) ||
              (time(NULL)-PFilter->LastFoundTime() < SCAN_DELAY))) {
        
//		printf("cScan::ScanServices: last found time %d time in loop %d max possible time %d\n",
//			PFilter->LastFoundTime(), time(NULL) - start, SCAN_DELAY);
			PFilter->GetFoundNum(foundNum,totalNum);
        
			if (totalNum && !foundSids) {
				start=time(NULL);
				foundSids=1;
			}
			usleep(200*1000);
		}
		
		usleep(200*1000);		
		PFilter->GetFoundNum(foundNum,totalNum);

        device->Detach(PFilter);
        device->Detach(SFilter);
        device->Detach(EFilter);

        PFilter = NULL;
        SFilter = NULL;
        EFilter = NULL;
    device->Detach(dummyReceiver);
    delete dummyReceiver;
}
//-------------------------------------------------------------------------
void cScan::ScanDVB_S(cTransponder *tp,cChannel *c)
{
    tp->SetTransponderData(c, sourceCode);
    if (!device->SwitchChannel(c,false))
        esyslog(ERR "SwitchChannel(c)  failed");
#if VDRVERSNUM >= 10330
    else 
    {
        struct 
        {
            cDevice* device;
            cChannel* channel;
        } data;
        data.device = device;
        data.channel = c; 
        cPlugin *Plugin = cPluginManager::GetPlugin("rotor");
        if (Plugin)
              Plugin->Service("Rotor-SwitchChannel", &data);
    }
#endif        
    usleep(100*1000);
    if (cDevice::GetDevice(cardnr)->HasLock(DVBS_LOCK_TIMEOUT))
    {
        DLOG(DBG "  ------------- HAS LOCK -------------     ");
        ScanServices();
    }
}
//-------------------------------------------------------------------------

// detail bit 0: wait longer
// detail bit 1: search also +-166kHz offset

void cScan::ScanDVB_T(cTransponder *tp, cChannel *c)
{
    int timeout=1000;
    int retries=0;
    int response,n,m;
    int frequency_orig=tp->Frequency();
    int offsets[3]={0,-166666,166666};
    tp->SetFrequency(frequency_orig);

    for(n=0;n<(detailedSearch&2?3:1);n++) {

        if (!cMenuChannelscan::scanning)
            break;

        tp->SetFrequency(frequency_orig+offsets[n]);
        frequency = tp->Frequency();
        tp->SetTransponderData(c, sourceCode);
        
        if (!device->SwitchChannel(c,true))
            esyslog(ERR "SwitchChannel(c)  failed");
        DLOG("%i Tune %i \n",retries,frequency);
        usleep(1500*1000);
        if (lastLocked)
            sleep(2); // avoid false lock

        for(m=0;m<(detailedSearch&1?8:2);m++) {

            response=getStatus();
			if(getStatus() != 0){
				DLOG("%i RESPONSE %x\n",retries,response);
            }
            if (response&0x10==0x10) {// Lock
                break;
            }

            if (response&15>2) // Almost locked, give it some more time
                sleep(1);
            sleep(1);
        }

        if (!cMenuChannelscan::scanning)
            break;
            
        if ( device->HasLock(timeout))
        {
            DLOG(DBG "  ------ HAS LOCK ------");
            printf("LOCK @ %.1f\n",tp->Frequency()/1.0e6);
            ScanServices();
            lastLocked=1;
            return;
        }
        lastLocked=0;
    }
}
//-------------------------------------------------------------------------
/*
  Scan algorithm for DVB-C: 
  Try 256QAM after 64QAM only if signal strength is high enough
  If initial mod is != 64QAM, then no 64QAM/256QAM auto detection is done

  Try 6900/6875/6111 until lock is achieved, then use found rate for all subsequent scans
  if scanMode is 2, only fixed SR is used.

  Wait additional 3s when last channel had lock to avoid false locking
 */

void cScan::ScanDVB_C(cTransponder *tp,cChannel *c)
{
    int timeout=500;
    int str1,n,m;
    int srtab[3]={6900,6875,6111};
    int fixedModulation=0;
    
    frequency = tp->Frequency();
    DLOG("Scan %f, Modulation %i SR %i\n",frequency/1e6,tp->Modulation(), tp->Symbolrate());

    if (tp->Modulation()!=QAM_64)
        fixedModulation=1;

    // SR try loop
    for(m=0;m<3;m++) {
        if (scanMode!=2) { // not manual
            if (srstat!=-1) {
                printf("Use auto SR %i\n",srtab[srstat]);
                tp->SetSymbolrate(srtab[srstat]);
            }
            else {
                printf("Use SR %i\n",srtab[m]);
                tp->SetSymbolrate(srtab[m]);
            }
        }

        // Reset modulation
        if (!fixedModulation)
            tp->SetModulation(QAM_64);

        for(n=0;n<2;n++) { // try 64QAM/256QAM
            if (!cMenuChannelscan::scanning)
                return;
            
            tp->SetTransponderData(c, sourceCode);
            
            if (!device->SwitchChannel(c,true))
                esyslog(ERR "SwitchChannel(c)  failed");
            
            usleep(500*1000);
            
            str1=getSignal();
            
            if (lastLocked) {
                printf("Wait last lock\n");
                sleep(3);  // avoid false lock
            }
            
            if (device->HasLock(timeout))
            {
                DLOG(DBG "  ------------- HAS LOCK -------------     ");
                printf("LOCK @ %.1f\n",tp->Frequency()/1.0e6);
                ScanServices();
                lastLocked=1;
                if (scanMode==0 && srstat==-1)
                    srstat=m; // remember SR for the remaining scan
                return;
            }
            lastLocked=0;
            
            if (!fixedModulation && n==0 && str1>0x4000) {
                printf("QAM256 %x\n",str1);
                tp->SetModulation(QAM_256);
            }
            else 
                break;
        }
        // leave SR try loop without useable signal strength (even in "try all" mode)
        if (str1<0x4000 || (scanMode==0 && srstat!=-1) || scanMode==2) 
            break;
    }
}

//---------  cMainMenu::Action()  -----------------------------------

void cScan::Action()
{
    DLOG(DBG " Action ");
	displayVideo = true; //remove this
    // setting scanning state only here
    cMenuChannelscan::scanning = true;

    device = cDevice::GetDevice(cardnr);
	device->DetachAllReceivers();

    cChannel *c = new cChannel;
	
    DLOG(DBG " loop through all transponders ");
    cTransponders &transponders = cTransponders::GetInstance();
    // loop through all Transponders
    for (vTpIter tp = transponders.v_tp_.begin(); tp != transponders.v_tp_.end(); ++tp)
    {
        
        if (!cMenuChannelscan::scanning)
            break;

        // get counter
        transponderNr = tp - transponders.v_tp_.begin();
        channelNumber = (*tp)->ChannelNum();
        frequency = (*tp)->Frequency();
	
        if (device->ProvidesSource(cSource::stTerr)){
			        ScanDVB_T(*tp,c);
			}
        else if (device->ProvidesSource(cSource::stSat)){
            ScanDVB_S(*tp,c);}
        else if (device->ProvidesSource(cSource::stCable)){
            ScanDVB_C(*tp,c);}

    } // Scanning loop
    DLOG(DBG " End of transponderlist. End of scan ");

    dummyCtl->Stop();
    delete dummyCtl;

    cMenuChannelscan::scanning = false;
    
    if(channel) //  && scanning_on_receiving_device)
        cDevice::PrimaryDevice()->SwitchChannel(channel,true);

    delete c;

    scanning_on_receiving_device = false;
}


//-------------------------------------------------------------------------

bool cScan::StartScanning(scanParameters *scp)
{
    DLOG(DBG " StartScanning"); 
    cTransponders &transponders = cTransponders::GetInstance();
    if (transponders.v_tp_.size()==0)
    {
        esyslog(ERR " Empty Transponderlist" );
        return false;
    }

    detailedSearch=scp->detail;
    scanMode=scp->symbolrate_mode;

    channel = Channels.GetByNumber(cDevice::CurrentChannel());
    //scanning_by_channelscan = true;
    //bool needsDetachReceivers = true;

    // Reset internal scan states
    lastLocked=1; // for safety
    srstat=-1;
    
    cDevice::PrimaryDevice()->StopReplay();

    cardnr = scp->card;
    DLOG (DBG " Stop Replay Card Number %d", cardnr);

    dummyCtl=new cDummyCtl();
//    cControl::Launch(dummyCtl);

    // setting scanning state only here
    //cMenuChannelscan::scanning = true;
    DLOG(DBG " End Start scanning CallStart");
    Start();

    return true;
}
