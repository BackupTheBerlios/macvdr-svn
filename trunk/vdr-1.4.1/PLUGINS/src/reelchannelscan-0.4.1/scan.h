/********************************************************************************
 * transponders.h channelscan part to parse transponder lists *.tpls
 * 
 *
 * See REDME for copyright information and
 * how to reach the authors.
 *
 ********************************************************************************/


#ifndef _SCAN_H
#define _SCAN_H


//#include <linux/dvb/frontend.h>
#include <string>
#include <vector>
#include <vdr/sources.h>
#include <vdr/sources.h>
#include <vdr/device.h>
#include <vdr/channels.h>
#include <vdr/eit.h>
#include "filter.h"
#include "channelscan.h"
#include "transponders.h"

using std::string;
using std::vector;
using std::cerr;
using std::endl;

class SdtFilter;
class PatFilter;


// -- cScan --------------------------------------------------------

class cScan : public cThread {
friend class SdtFilter;
private:

   cScan(const cScan&);
   cScan&operator=(const cScan&);
   void ScanServices();
   void ScanDVB_S(cTransponder *tp,cChannel *c);
   void ScanDVB_T(cTransponder *tp,cChannel *c);
   void ScanDVB_C(cTransponder *tp,cChannel *c);

   int fd_frontend;
   cDevice *device;
   cChannel *channel;
   bool displayVideo; // show Video during scanning
   int origUpdateChannels;
   int sourceCode;    // internal VDR Code for dvb Sources  see vdr/sources.c
   int cardnr;
   int transponderNr;
   int channelNumber;
   int frequency;
   int totalTp;
   int newChannels;
   int detailedSearch;
   int scanMode;
   int srstat;

   int lastLocked;
   int foundNum, totalNum;

   SdtFilter *SFilter;
   PatFilter *PFilter;
   cEitFilter *EFilter;

   uint16_t getSignal(void);
   uint16_t getSNR(void);
   uint16_t getStatus(void);
protected:
    virtual void Action(void);
public:
    cScan();
    ~cScan();
    bool StartScanning(scanParameters *scp);
    bool Signal(int frequency, char pol, int symbolrate);
    int GetCurrentTransponderNr() { return transponderNr; }
    ///< GetCurrentTransponderNr returns the sequential transponder number 
    int GetCurrentChannelNumber() { return channelNumber; }
    ///< GetCurrentTransponderNr returns the channel number according to frequencies 
    int GetCurrentFrequency() { return frequency; }
    void GetFoundNum(int &current, int &total) {current=foundNum; total=abs(totalNum<200)?totalNum:0;};
};

#endif //_SCAN__H
