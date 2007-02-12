/********************************************************************************
 * csmenu.h channelscan menu part
 * writen by markus Hahn @ reel-multimedia
 *
 * See REDME for copyright information and
 * how to reach the authors.
 *
 ********************************************************************************/

#ifndef __CSMENU_H
#define __CSMENU_H


#include <string>
#include <vector>
#include <vdr/osdbase.h>
#include <vdr/channels.h>
#include <vdr/menuitems.h>
#include <vdr/thread.h>
#include <vdr/diseqc.h>
#include <vdr/sources.h>
#include <vdr/config.h>
#include "transponders.h"
#include "channelscan.h"
#include "scan.h"

/*class ServiceNames
{

  public:
  vector<string> tvChannelNames_;
  vector<string> radioChannelNames_;
  vector<string> dataChannelNames_;
  Lock();
  UnLock();
};
*/


class cMenuChannelscan : public cOsdMenu  {
private:
   cSetup data;
   int searchMode;
   int autoSearch;
   int source;
   int frequency;
   int symbolrate;
   char *srcTexts[4];
   int srcTypes[4];
   int srcDevIndex[4];
   int srcTuners;
   const char *fecTexts[10];
   const char *modTexts[10];
   const char *sRateItem[3];
   const char *sSRScanMode[3];
   const char *sBwItem[3];

   
   int fecStat;
   int modStat;
   char polarization;
   int bandwidth;
   int detailedScan;
   int srScanMode;

   scanParameters scp;

   int loopIndex;
   std::vector<int> loopSources;
   static int sourceStat;
   int sourceType;
   static int autoSearchStat;
   static int channelNumber;
   static bool expertSettings; // static??

   int sRate6875;
   int sRateManual;
   int qam256;
   
   int lnbs;
   int currentChannel;
   void InitLnbs();
   void Set();
   int getType(int src);
   //const char *updateChannelsTexts[6];
public:
   cMenuChannelscan(int CurrentChannelNr);
   ~cMenuChannelscan();
   virtual eOSState ProcessKey(eKeys Key);
   void DiseqShow();
   void Store();
   void AddBlankLineItem(int line);

   static volatile bool scanning;
   static bool scanActiveMenu;
};


class cMenuScanActive : public cOsdMenu {
private:
   int oldChannelNumbers;
   int oldUpdateChannels;
   //int cardNr_;
   bool retStat_;
   int transponderNum_;
   std::auto_ptr<cScan> Scan;
   scanParameters *scp;
    void Setup();
public:
   cMenuScanActive(scanParameters *sp);
   ~cMenuScanActive();
   eOSState ProcessKey(eKeys Key);
   void AddBlankLineItem(int lines);
   void DeleteDummy();
   virtual bool NoAutoClose(void) { return true; }
};

// --- cMenuScanActiveItem ----------------------------------------------------

class cMenuScanActiveItem : public cOsdItem {
private:
   char *tvChannel;
   char *radioChannel;
public:
   cMenuScanActiveItem();
   cMenuScanActiveItem(const char *TvChannel,const char *RadioChannel);
   ~cMenuScanActiveItem();
   void SetValues();
};

// taken from menu.c
// --- cMenuSetupBase ---------------------------------------------------------
class cMenuSetupBase : public cMenuSetupPage {
protected:
  cSetup data;
  virtual void Store(void);
public:
  cMenuSetupBase(void);
  };


// --- cMenuSetupLNB ---------------------------------------------------------

class cMenuSetupLNB : public cMenuSetupBase {
private:
  void Setup(void);
  void SetHelpKeys(void);
  cSource *source;

  const char *useDiSEqcTexts[4];
  const char *lofTexts[5];
  bool diseqcConfRead;
  int diseqObjNumber;
  int lnbNumber;
  int currentchannel;
  int waitMs;
  int repeat;

public:
  cMenuSetupLNB(void);
  virtual eOSState ProcessKey(eKeys Key);
  };

#ifndef REELVDR
class cMenuEditSrcItem : public cMenuEditIntItem {
private:
   const cSource *source;
protected:
   virtual void Set(void);
public:
   cMenuEditSrcItem(const char *Name, int *Value);
   eOSState ProcessKey(eKeys Key);
};
#endif

class cMenuScanInfoItem : public cOsdItem {
public:
    cMenuScanInfoItem(std::string pos, int f, char c, int sr, int fecIndex);
    const char *FECToStr(int Index);
};

class cMenuStatusBar : public cOsdItem {
   int total;
   int part;
public:
   cMenuStatusBar(int total, int current, int channelNr);
   int LiteralCentStat(char **str);
};

class cMenuInfoItem : public cOsdItem {
public:
   cMenuInfoItem(const char *text, const char *textValue = NULL);
   cMenuInfoItem(const char *text, int intValue);
   cMenuInfoItem(const char *text, bool boolValue);
};

#endif // __CSMENU_H
