/*
 * csmenu.c channelscan menu
 * written by reel-multimedia
 *
 * See README for copyright information and
 * how to reach the authors.
 *
 */

#include <iostream>
#include <vector>
#include <string>
//#include <linux/dvb/frontend.h> 
#include <sys/ioctl.h> 
#include <vdr/channels.h>
#include <vdr/device.h>
#include <vdr/config.h>
#include <vdr/diseqc.h>
#include <vdr/menu.h>
#include <vdr/menuitems.h>
#include <vdr/tools.h>
#include <vdr/osdbase.h>
#include <vdr/status.h>
#include <vdr/menu.h>
#include <vdr/remote.h>

#include "channelscan.h"
#include "csmenu.h"
#include "filter.h"


#define CHNUMWIDTH 16
#define DVBFRONTEND "/dev/dvb/adapter%d/frontend0"
#define POLLDELAY 1
#define MENUDELAY 3

using std::endl;
using std::time;
using std::string;
using std::cout;
using std::cerr;
using std::vector;
using std::ofstream;

// XXX check all static vars!
typedef vector<int>::const_iterator  iConstIter;

vector<string> tvChannelNames;
vector<string> radioChannelNames;
vector<string> dataChannelNames;

cMutex mutexNames;
cMutex mutexScan;

int cMenuChannelscan::autoSearchStat = 1;
int cMenuChannelscan::sourceStat = 0;

bool cMenuChannelscan::expertSettings = 0;


volatile bool cMenuChannelscan::scanning = false;
bool cMenuChannelscan::scanActiveMenu = false;

// OK
cTransponders *cTransponders::instance_ = NULL;

#define SAT   0
#define CABLE 1
#define TERR  2
 
int pollCount;
#ifndef REELVDR
bool loopMode = false;
#endif

// --- Class cMenuChannelscan ------------------------------------------------

cMenuChannelscan::cMenuChannelscan(int CurrentChannelNr)
:cOsdMenu(tr("Channelscan"), CHNUMWIDTH)
{

  data = Setup;
  cTransponders::Create();
  Channels.Save();
  
  //frequency = 11837;
  frequency = 12031;
  symbolrate = 27500;
  polarization = 'H';
  detailedScan = 0;

  autoSearch = autoSearchStat;
  searchMode=0;
  expertSettings = 0;
#ifndef REELVDR
  loopMode = false;
#endif
  srcTuners=0;
  // Tuner detection
	int maxDevLoop =  (cDevice::NumDevices() < 4) ? cDevice::NumDevices() : 4;
  for(int i=0;i<maxDevLoop;i++) {
   cDevice *device = cDevice::GetDevice(i);
   if( device == 0x0 ) break;
   int stp=0;
   srcTexts[i]=NULL; // char[] snprintf avoid free in ~..
   if (device) {
      char *txt=NULL;
      if (device->ProvidesSource(cSource::stTerr)) {
         asprintf(&txt,"%s (%s %i)",tr("DVB-T - Terrestrial"),tr("Tuner"),i+1);
         stp=TERR;
      }
      else if (device->ProvidesSource(cSource::stCable)) {
         asprintf(&txt,"%s (%s %i)",tr("DVB-C - Cable"),tr("Tuner"),i+1);
         stp=CABLE;
      }
      else if (device->ProvidesSource(cSource::stSat)) {
         asprintf(&txt,"%s (%s %i)",tr("DVB-S - Satellite"),tr("Tuner"),i+1);
         stp=SAT;
      }
      if (txt) {
         srcTypes[srcTuners] = stp;
         srcTexts[srcTuners] = txt;
         srcDevIndex[srcTuners++] = device->DeviceNumber();
      }
   }
  }            

  fecTexts[0] = tr("None");
  fecTexts[1] = "1/2";
  fecTexts[2] = "2/3";
  fecTexts[3] = "3/4";
  fecTexts[4] = "4/5";
  fecTexts[5] = "5/6";
  fecTexts[6] = "6/7";
  fecTexts[7] = "7/8";
  fecTexts[8] = "8/9";
  fecTexts[9] = tr("Auto");

  fecStat = 3;
  //modTexts[0] = "QAM-16";
  //modTexts[1] = "QAM-32";
  
  modTexts[0] = "Auto 64QAM/256QAM";
  modTexts[1] = "256QAM";
  modTexts[2] = "4QAM/QPSK";
  modTexts[3] = "16QAM";
  modTexts[4] = "32QAM";
  modTexts[5] = "128QAM";

  modStat = 0;
   
  /*
  updateChannelsTexts[0] = tr("no");
  updateChannelsTexts[1] = tr("names only");
  updateChannelsTexts[2] = tr("PIDs only");
  updateChannelsTexts[3] = tr("names and PIDs");
  updateChannelsTexts[4] = tr("add new channels");
  updateChannelsTexts[5] = tr("add new transponders");
  */

  bandwidth = 0; // 0: 7MHz 1: 8MHz

  sRate6875 = 0;
  sRateItem[0] = "6900";
  sRateItem[1] = "6875";
  sRateItem[2] = "6111";

  sBwItem[0] = "Auto";
  sBwItem[1] = "7 MHz";
  sBwItem[2] = "8 MHz";

  sSRScanMode[0]= tr("Intelligent 6900/6875/6111");
  sSRScanMode[1]= tr("Try all 6900/6875/6111");
  sSRScanMode[2]= tr("Manual");

  srScanMode=0;
  
  // CurrentChannel has to be greater than 0!
  CurrentChannelNr = CurrentChannelNr == 0 ? 1 : CurrentChannelNr;
  cChannel *channel = Channels.GetByNumber(CurrentChannelNr);

  if (channel) 
  {
      source = channel->Source();
  }
  else 
  {
     //XXX hotfix  if no channel given test first device for dvb-type!
     source = cSource::FromString("S19.2E");
     
  }

  currentChannel = CurrentChannelNr;
  scanActiveMenu = false;

  pollCount = POLLDELAY;
  loopIndex = 0;
  
  InitLnbs();

  Set();
}

void cMenuChannelscan::InitLnbs()
{
   lnbs = 0;
   for(cDiseqc *diseqc = Diseqcs.First(); diseqc; diseqc=Diseqcs.Next(diseqc))
   {
      if (diseqc != Diseqcs.First() && diseqc->Source() == Diseqcs.Prev(diseqc)->Source() )
          continue;
      loopSources.push_back(diseqc->Source());
      lnbs++;
   }
}



void cMenuChannelscan::Set()
{    
    int current = Current();

    Clear();  

    sourceType=srcTypes[sourceStat];
    // avoid C/T-positions for SAT

    if (srcTypes[sourceStat]==SAT  && (source==cSource::FromString("C") || source==cSource::FromString("T")))
       source = cSource::FromString("S19.2E");

    int blankLines = 4;
   // for DVB-T or DVB-C   
   // we loop through all satalites
   if(loopMode && ::Setup.DiSEqC){
       Add(new cMenuInfoItem("Scanning configured satellites"));
       AddBlankLineItem(4);

   }
   else{

     if (cPluginChannelscan::AutoScanStat != AssNone){
         switch (cPluginChannelscan::AutoScanStat){
            case AssDvbS:
                 sourceType = SAT;
                 break;
            case AssDvbC: 
                 sourceType = CABLE;  
                 break;
            case AssDvbT: 
                 sourceType = TERR;
                 break;
            default: 
                 cPluginChannelscan::AutoScanStat = AssNone;
                 esyslog ("Channelscan service handling error");
                 break;
         }
         cRemote::Put(kRed);
     
     }
#ifdef REELVDR
      Add(new cMenuEditBoolItem(tr("Setup.DVB$Update channels"),           &data.UpdateChannels, tr("no"), tr("yes") ));
      //Add(new cMenuEditStraItem(tr("Channellist"),     &data.DiSEqC, 4,   ));
      //Add(new cMenuEditStraItem(tr("Setup.DVB$Update channels"),       &data.UpdateChannels, 6, updateChannelsTexts));
#endif
      Add(new cMenuEditStraItem(tr("Source"), &sourceStat, srcTuners, srcTexts));
      
      switch(sourceType) {
      case SAT:
         Add(new cMenuEditBoolItem(tr("Search Mode"), &autoSearch, tr("Manual"),  tr("SearchMode$Auto") ));      
         Add(new cMenuEditSrcItem(tr("Position"), &source)); //XXX all sources in Diseqc? 
         break;
      case TERR:
         Add(new cMenuEditBoolItem(tr("Detailed search"), &searchMode, tr("no"),tr("yes")));
         Add(new cMenuEditBoolItem(tr("Search Mode"), &autoSearch, tr("Manual"),  tr("SearchMode$Auto") ));      
         break;
      case CABLE:
         Add(new cMenuEditBoolItem(tr("Search Mode"), &autoSearch, tr("Manual"),  tr("SearchMode$Auto") ));
         break;
      }
      
      AddBlankLineItem(1);

      if (sourceType == CABLE && (expertSettings || !autoSearch))
      {
         if (!autoSearch) {
            frequency = 113000; // reset right default values 
            Add(new cMenuEditIntItem(tr("Frequency (kHz)"), &frequency));  
            Add(new cMenuEditBoolItem(tr("Bandwidth"), &bandwidth, "7 MHz", "8 MHz"));
         }
            
         Add(new cMenuEditStraItem(tr("Symbol Rate Scan"), &srScanMode,  3, sSRScanMode ));
         if (srScanMode==2)
            Add(new cMenuEditIntItem(tr("Symbolrate"), &symbolrate));
         Add(new cMenuEditStraItem(tr("Modulation"), &modStat, 6, modTexts));

           blankLines -= 2;
      } 
      else {
         modStat=0;
         srScanMode=0;
      }

      if (!autoSearch)
      {
          switch (sourceType)
          { 
              case SAT:
                 // frequency = 11837;
                  frequency = 12031;
                  symbolrate = 27500;  // reset right default values
                  Add(new cMenuEditIntItem(tr("Frequency (MHz)"), &frequency));  
                  Add(new cMenuEditChrItem(tr("Polarization"),&polarization, "HVLR"));
                  Add(new cMenuEditIntItem(tr("Symbolrate"), &symbolrate));
                  Add(new cMenuEditStraItem("FEC", &fecStat, 10, fecTexts ));
                  break;          
          
              case TERR:
                  frequency = 212500; // reset right default values
                  Add(new cMenuEditIntItem(tr("Frequency (kHz)"), &frequency));  
                  Add(new cMenuEditStraItem(tr("Bandwidth"), &bandwidth, 3, sBwItem));

                  AddBlankLineItem(2);
                  break;
              default: 
                  break;
          }
      }
      else 
         AddBlankLineItem(4);
   }

   // AllignButtom()
   if (::Setup.UseSmallFont || strcmp(::Setup.OSDSkin,"Reel") == 0)
   {
      blankLines -= 1;
   }

   AddBlankLineItem(blankLines);

   if (sourceType==SAT) {
      Add(new cMenuInfoItem(tr("DiSEqC"), static_cast<bool>(::Setup.DiSEqC))); 

      if (::Setup.DiSEqC)
         DiseqShow();  // 2
   }

   if (!loopMode && sourceType == CABLE && autoSearch )
   {
       SetHelp(tr("Button$Start") /*tr("DiSEqC"),*/, expertSettings?tr("Standard"):tr("Extended"));
   }
   else 
   {
       SetHelp(tr("Button$Start") /*tr("DiSEqC") */);
   }

   SetCurrent(Get(current));
   Display();
}

void  cMenuChannelscan::Store()
{
    if (data.UpdateChannels>0)
        data.UpdateChannels = 5;
     Setup = data;
     Setup.Save();
} 

eOSState cMenuChannelscan::ProcessKey(eKeys Key)
{

  bool HadSub = HasSubMenu();

  int oldDetailedScan = detailedScan;
  int oldAutoSearch = autoSearch;
  int oldSourceStat = sourceStat;
  int oldSRScanMode = srScanMode;
  bool oldExpertSettings = expertSettings;

  eOSState state = cOsdMenu::ProcessKey(Key);
   
  sourceType=srcTypes[sourceStat];

  scp.card = srcDevIndex[sourceStat];
  scp.type=sourceType;
  scp.bandwidth = bandwidth;
  scp.polarization = polarization;
  scp.symbolrate = symbolrate;
  scp.fec = fecStat;
  scp.detail = autoSearch | (searchMode<<1); // searchMode=1 -> DVB-T offset search
  scp.modulation = modStat;
  scp.symbolrate_mode = srScanMode;

  // items return osUnkown if 
  if (state == osUnknown)
  {
     switch(Key)
     {
        case kOk:
             Store();            
             return osContinue;

        case kRed:
      source = sourceType==1?0x4000:sourceType==2?0xC000:source;
      if (autoSearch)
         scp.frequency=0;     // Full scan
      else
         scp.frequency=frequency;

      cTransponders::GetInstance().Load(source, &scp); 
      return AddSubMenu(new cMenuScanActive(&scp));     
      break;

       /*
       case  kGreen:
             AddSubMenu(new cMenuSetupLNB);
             //Setup();
             break; */

       case  kGreen: //kYellow:
             if (sourceType == 1)
             {
                expertSettings  = expertSettings?false:true;
                Set();
             }
             break;

       /*
       case  kYellow: //XXX only for Testing 
             loopMode = loopMode?false:true;
             break;
       */
             
       default: state = osContinue;
     }
  } 

  // forces setup if menu layout should be changed
  if ( Key != kNone && !HadSub)
  {
     
     if (autoSearch != oldAutoSearch ||
         oldDetailedScan != detailedScan ||
         oldExpertSettings != expertSettings || 
         oldSourceStat != sourceStat ||
    oldSRScanMode != srScanMode)
     {
        if (sourceType == CABLE && symbolrate>8000)  // avoid DVB-S symbolrates in DVB-C
      symbolrate=6900;

         oldSourceStat = sourceStat;

         Set();
     }
  }
  return state;
}

/*
void cMenuChannelscan::StartScan()
{
  //XXX 
     
}
*/
void cMenuChannelscan::DiseqShow()
{
  for (iConstIter iter = loopSources.begin(); iter != loopSources.end(); ++iter)
  {
     char *buffer = NULL;

     asprintf(&buffer,"LNB %c: %s", 'A'+(iter-loopSources.begin()), *cSource::ToString(*iter));
     DLOG ("Show fetch Source [%d] %d", iter-loopSources.begin() , *iter);
     Add(new cMenuInfoItem(buffer));
     free(buffer);
  }
  Add(new cMenuInfoItem(tr("channels in current list"), Channels.MaxNumber()));
} 

cMenuChannelscan::~cMenuChannelscan()
{
  for(int i=0;i<srcTuners;i++)
     if (srcTexts[i])
        free(srcTexts[i]);

  cTransponders::Destroy();
  tvChannelNames.clear();
  radioChannelNames.clear();
  dataChannelNames.clear();
  loopMode = false;
  cPluginChannelscan::AutoScanStat = AssNone;

  dsyslog ("DEBUG cs  --------------------------------------- END CHANNELSCAN  ------------------------------------------ "); 
  printf ("DEBUG cs  --------------------------------------- END CHANNELSCAN  ------------------------------------------ \n"); 
}

// taken fron vdr/menu.c
void cMenuChannelscan::AddBlankLineItem(int lines)
{
   for(int i = 0;i < lines;i++)
   {
      cOsdItem *item = new cOsdItem;
      item->SetSelectable(false);
      item->SetText(strndup(" ", 1), false);
      Add(item);
   }
}

// --- cMenuScanActiveItem ----------------------------------------------------
cMenuScanActiveItem::cMenuScanActiveItem(const char *TvChannel,const char *RadioChannel)
{
  tvChannel = strdup(TvChannel);
  radioChannel = strdup(RadioChannel);
  char *buffer = NULL;
  asprintf(&buffer, "%s \t%s", tvChannel, radioChannel);
  SetText(buffer, false);
  SetSelectable(false);
}

cMenuScanActiveItem::~cMenuScanActiveItem()
{
  free(tvChannel);
  free(radioChannel);
}

// --- cMenuScanActive -------------------------------------------------------

cMenuScanActive::cMenuScanActive(scanParameters *sp)
:cOsdMenu(tr("TV CHANNELS                     RADIO"), 25)
{
  retStat_ = false;
  scp=sp;

  DLOG(DBG " cMenuScanActive Freq %d  --- ",  scp->frequency);

#ifdef REELVDR
  oldUpdateChannels = ::Setup.UpdateChannels;
  ::Setup.UpdateChannels = 0; // prevent  VDRs own update Channel  
#endif

  cMenuChannelscan::scanActiveMenu = true;
  oldChannelNumbers = Channels.MaxNumber();

  // Make class 
  tvChannelNames.clear();
  radioChannelNames.clear();

  cTransponders &transponders = cTransponders::GetInstance();
  transponderNum_ = transponders.v_tp_.size();
  Scan.reset(new cScan());
  
  DLOG (DBG " start Scanning @ Card %d --- ", scp->card);

  if (!Scan->StartScanning(scp))
  {
     esyslog (ERR "  Tuner Error");
     // ShowInfo
     cMenuChannelscan::scanning == false;
     //Add(new cMenuInfoItem(tr("Tuner Error")));
  }
  Setup();
}

void cMenuScanActive::Setup()
{
  int num_tv=0,num_radio=0;
  Clear();
  mutexNames.Lock();

  vector<string>::iterator tv;
  vector<string>::iterator radio;

  tv = tvChannelNames.begin();
  radio = radioChannelNames.begin();

  num_tv=tvChannelNames.size();
  num_radio=radioChannelNames.size();

  if (tvChannelNames.size() > 10)
     tv+=tvChannelNames.size()-10;

  if (radioChannelNames.size() > 10)
     radio+=radioChannelNames.size()-10;

  while(1)
  {
     if (tv == tvChannelNames.end() &&  radio == radioChannelNames.end())
        break;

     cMenuScanActiveItem *Item = new cMenuScanActiveItem(tv == tvChannelNames.end()?"":tv->c_str(),
                               radio == radioChannelNames.end()?"":radio->c_str());
     Add(Item);

     if (tv != tvChannelNames.end()) {
      ++tv;
     }
     if (radio != radioChannelNames.end()) {
      ++radio;
     }

  }
  int nameLines = tvChannelNames.size() > radioChannelNames.size() ? tvChannelNames.size() : radioChannelNames.size();

  if (nameLines>10)
     nameLines=10;

  mutexNames.Unlock();

  AddBlankLineItem(11 - nameLines);

  /*
    Add(new cMenuScanInfoItem(transponders.Position(),transponders[i].Frequency(),
    transponders[i].Polarisation(), transponders[i].Symbolrate(), transponders[i].FEC()));
  */
  char buffer[50];
  snprintf(buffer,50, "TV: %i \tRadio: %i ", num_tv,num_radio);
  Add(new cMenuInfoItem(buffer));
  /*
  int foundNum=0,totalNum=0;
  Scan->GetFoundNum(foundNum,totalNum);
    snprintf(buffer,50, tr("Running services on transponder: %i / %i"), foundNum, totalNum);
  Add(new cMenuInfoItem(buffer));
 */

  if (cMenuChannelscan::scanning)
  {
      //Add(new cMenuStatusBar(transponderNum_, i, chNumber));

//      snprintf(buffer,50, tr("Scanning %s (%.3fMHz) \tPlease Wait"), cTransponders::GetInstance().Position().c_str(), 

      char *txts[3]={"TV only","Radio only","Radio + TV"};

      if (scp->type==SAT)
         snprintf(buffer,50, tr("Scanning %s (%iMHz)\t%s"), cTransponders::GetInstance().Position().c_str(), 
             Scan->GetCurrentFrequency(),tr(txts[cMenuChannelscanSetup::SetupServiceType]));
      else
         snprintf(buffer,50, tr("Scanning %s (%.3fMHz)\t%s"), tr(cTransponders::GetInstance().Position().c_str()), 
             Scan->GetCurrentFrequency()/(1000.0*1000),tr(txts[cMenuChannelscanSetup::SetupServiceType]));
         
      Add(new cMenuInfoItem(buffer));
      Add(new cMenuStatusBar(transponderNum_, Scan->GetCurrentTransponderNr(), Scan->GetCurrentChannelNumber()));
    
  }
  else // if (!loopMode) // ScanStat == success 
  {
    Channels.Save();

    if (Channels.MaxNumber() > oldChannelNumbers ) //&& ScanStat == success)
       Add(new cMenuInfoItem(tr("Added new channels"), (Channels.MaxNumber() - oldChannelNumbers)));
     else if (Channels.MaxNumber() == oldChannelNumbers ) //&& ScanStat == success)
       Add(new cMenuInfoItem(tr("No new channels found")));
     
       Add(new cMenuInfoItem(""));
       Add(new cMenuInfoItem(tr("Press OK to finish or Exit for new scan")));
    
     /*
    if (cPluginChannelscan::AutoScanStat != AssNone)
       cRemote::Put(kBack);       
    */

  }

  Display();
}

void cMenuScanActive::DeleteDummy()
{
   cChannel *channel = Channels.First(); 

   printf ("channel-Name   %s  Short: %s  ",channel->Name()  , channel->ShortName());
   if (strcmp(channel->Name(),"ReelBox") == 0)
       Channels.Del(channel);

   Channels.ReNumber();
}

eOSState cMenuScanActive::ProcessKey(eKeys Key)
{

  if (cPluginChannelscan::AutoScanStat != AssNone && !cMenuChannelscan::scanning)
       return osEnd;

  DLOG (DBG " !!  MenuActiveScan ProcKey loopMode %s :scanning %s ", loopMode?"Yes":"No", cMenuChannelscan::scanning ?"Yes":"No");
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (!cMenuChannelscan::scanning && loopMode) 
  {
     pollCount--;
     if (pollCount == 0)
     { 
       return osBack;
     }
     Setup();
  }

  if (state == osUnknown)
  {
     switch (Key) 
     {
        case kOk: 
             retStat_ = true;
             return osEnd;
        case kBack:
           cMenuChannelscan::scanning = false; //ScanStat = aborted;
           Skins.Message(mtError, tr("Scanning aborted by User"));
           return osBack;
        default: state = osContinue;
     }
  Setup();
  }
  return state;
}
void cMenuScanActive::AddBlankLineItem(int lines)
{
   for(int i = 0;i < lines;i++)
   {
     cOsdItem *item = new cOsdItem;
     item->SetSelectable(false);
     item->SetText(strndup(" ", 1), false);
     Add(item);
   }
}

cMenuScanActive::~cMenuScanActive()
{
  DLOG  (DBG " destruct ~cMenuScanActive ");
  
  tvChannelNames.clear();
  radioChannelNames.clear();
  dataChannelNames.clear();
  // XXX
  cMenuChannelscan::scanning = false;
  cMenuChannelscan::scanActiveMenu = false; //XXX HasSubMenu
#ifdef REELVDR
  ::Setup.UpdateChannels = oldUpdateChannels;
  loopMode = false;
#endif

  // call cMenuChannels if kOk 
  DeleteDummy();
  if (retStat_)
     cRemote::Put(kChannels); 

  if (cPluginChannelscan::AutoScanStat != AssNone)
  {
    DLOG (" Call by Install Return to wizard " );
    cRemote::CallPlugin("install");
  }
      
      
}

// cMenuEditSrcItem taken fron vdr/menu.c
// --- cMenuEditSrcItem ------------------------------------------------------

cMenuEditSrcItem::cMenuEditSrcItem(const char *Name, int *Value)
:cMenuEditIntItem(Name, Value, 0)
{
   source = Sources.Get(*Value);
   Set();
}

void cMenuEditSrcItem::Set(void)
{
    if (source) {
        char *buffer = NULL;
        asprintf(&buffer, "%s - %s", *cSource::ToString(source->Code()), source->Description());
        SetValue(buffer);
        free(buffer);
    }
    else
      cMenuEditIntItem::Set();
}

eOSState cMenuEditSrcItem::ProcessKey(eKeys Key)
{
    eOSState state = cMenuEditItem::ProcessKey(Key);

    if (state == osUnknown) {
        if (NORMALKEY(Key) == kLeft) { // TODO might want to increase the delta if repeated quickly?
            if (source && source->Prev()) {
                source = (cSource *)source->Prev();
                *value = source->Code();
            }
        } else if (NORMALKEY(Key) == kRight) {
            if (source) {
                if (source->Next()) 
                    source = (cSource *)source->Next();
            }
            else
                source = Sources.First();
                if (source)
                    *value = source->Code();
        }
        else
            return state; // we don't call cMenuEditIntItem::ProcessKey(Key) here since we don't accept numerical input

        Set();
        state = osContinue;
    }
    return state;
}

//taken from rotor rotor plugin

cMenuScanInfoItem::cMenuScanInfoItem(string Pos, int f, char pol, int sr, int fecIndex)
{

    const char *pos = Pos.c_str();
    char *buffer = NULL;
    asprintf(&buffer, "%s :\t%s %d %c %d %s",tr("Scanning on transponder"), pos, f, pol, sr, FECToStr(fecIndex));

    SetText(strdup(buffer), false);
    SetSelectable(false);
}
const char *cMenuScanInfoItem::FECToStr(int Index)
{
    switch (Index)
    {
        case 1: return "1/2";
        case 2: return "2/3";
        case 3: return "3/4";
        case 4: return "4/5";
        case 5: return "5/6";
        case 6: return "6/7";
        case 7: return "7/8";
        default : return tr("Auto");
    }
    return tr("Auto");
}

// --- cMenuStatusBar ----------------------------------------------------

cMenuStatusBar::cMenuStatusBar(int Total, int Current , int Channel)
{
   int barWidth = 50;
   int percent;
   Current +=1;

   if (Current > Total)
   {
     Current = Total;
     cerr << "error! Wrong transponder Values\n";
   }
   if (Total < 1)
   {
     Total = 1;
     cerr << "error! Wrong transponder Values\n";
   }

   // GetReal EditableWidth
   percent = static_cast<int>(((Current) * barWidth/(Total)));

   char buffer[barWidth+1];
   int i;

   buffer[0] = '[';
   for (i = 1;i < barWidth; i++)
       i < percent ?  buffer[i] = '|' : buffer[i] = ' ';

   buffer[i++] = ']';
   buffer[i] = '\0';

   char *tmp;
   int l = asprintf(&tmp,"%s\t  %d / %d  (CH: %d)", buffer, Current, Total, Channel);

   SetText(strndup(tmp,l), false);
   SetSelectable(false);
   free(tmp);
}

// --- Class cMenuInfoItem -------------------------------------

cMenuInfoItem::cMenuInfoItem(const char *text, const char *textValue)
{
    char *buffer = NULL;
    asprintf(&buffer,"%s  %s",text, textValue? textValue:"");

    SetText(strdup(buffer), false);
    SetSelectable(false);
   free(buffer);
}

cMenuInfoItem::cMenuInfoItem(const char *text, int intValue)
{
    char *buffer = NULL;
    asprintf(&buffer,"%s: %d",text, intValue);

    SetText(strdup(buffer), false);
    SetSelectable(false);
    free(buffer);
}
cMenuInfoItem::cMenuInfoItem(const char *text, bool boolValue)
{
    char *buffer = NULL;
    asprintf(&buffer,"%s: %s",text, boolValue? tr("enabled"):tr("disabled"));

    SetText(strdup(buffer), false);
    SetSelectable(false);
    free(buffer);
}

