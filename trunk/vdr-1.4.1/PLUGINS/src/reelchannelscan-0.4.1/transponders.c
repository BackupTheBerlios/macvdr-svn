/********************************************************************************
 * transponders.c
 * supports transponder data.
 * writen by reel-multimedia
 *
 * Contact to mhahn@reel-multimedia.com
 ********************************************************************************/

#include "transponders.h"
#include <vdr/plugin.h>
#include <vdr/sources.h>
//#include <linux/dvb/frontend.h>
#include <vdr/macosfrontend.h>

using std::cerr;
using std::endl;
using std::string;
using std::list;
using std::ifstream;
using std::ofstream;
using std::cout;

/* Notation for Europe (region 0)
   channel 1-4: VHF band 1
   5-13: VHF band 3
   21-69: UHF band 4/5
   101-110/111-120: cable frequencies (aka Sonderkanal/Midband/Superband S1-S20)
   121-141: cable frquencies (aka Hyperband, S21-S41)
   173/181: D73 and D81 in C-networks (Germany)
*/
#define DBG " Channelscan DEBUG: -- "
#define DLOG(x...) dsyslog(x)
   
int cTransponders::channel2Frequency(int region, int channel, int& bandwidth )
{
    bandwidth=BANDWIDTH_7_MHZ;

    if (region==0) {
        if (channel>=1 && channel <=4) {
            return 38000000 + (channel-1)*7000000; 
        }
        if (channel>=5 && channel<=13) {
            return 177500000 + 7000000*(channel-5);
        }
        if (channel>=21 && channel<=69) {
            bandwidth=BANDWIDTH_8_MHZ;
            return 474000000 + 8000000*(channel-21);
        }
        if (channel==101) 
            return 107500000;

        if (channel==102 || channel==103) {
            bandwidth=BANDWIDTH_8_MHZ;
            return 113000000+8000000*(channel-102);
        }
        if (channel>=104 && channel<=110) 
            return 128000000+7000000*(channel-104); // Fixme +500khz Offset?
            
        if (channel>=111 && channel<=120) 
            return 233000000+7000000*(channel-111); // Fixme +500khz Offset?

        if (channel>=121 && channel<=141) {
            bandwidth=BANDWIDTH_8_MHZ;
            return 306000000+8000000*(channel-121);
        }    
        if (channel==173) {
            bandwidth=BANDWIDTH_8_MHZ;
            return 73000000;
        }
        if (channel==181) {
            bandwidth=BANDWIDTH_8_MHZ;
            return 81000000;
        }
        
    }
    return 0;
}
//----------- Class Transponder -------------------------------


cTransponder::cTransponder(int Frequency)
:frequency_(Frequency)
{
   channelNum_ = 0;
};

int cTransponder::IntToFec(int val)
{
   int fec;
   switch (val) {
     case 12: return fec = FEC_1_2;
     case 23: return fec = FEC_2_3;
     case 34: return fec = FEC_3_4;
     case 45: return fec = FEC_4_5;
     case 56: return fec = FEC_5_6;
     case 67: return fec = FEC_6_7;
     case 78: return fec = FEC_7_8;
     default : return fec = FEC_AUTO;
   }
   return FEC_NONE;
}

//----------- Class cSatTransponder -------------------------------

cSatTransponder::cSatTransponder()
:cTransponder(0)
{
   symbolrate_ = 0;
   pol_ = ' ';
   fec_ = 0;
}

cSatTransponder::cSatTransponder(int Frequency, char Pol, int SymbolRate, int FEC)
:cTransponder(Frequency)        
{
   channelNum_ = 0;
   symbolrate_ = SymbolRate;
   pol_ = Pol;
   fec_ = FEC;
   DLOG (DBG " new cSatTransponder(f: %d,p: %c,sRate: %d,fec %d", frequency_, pol_, symbolrate_, fec_);
}

bool cSatTransponder::SetTransponderData(cChannel *c, int Code)
{
   DLOG (DBG " SetSatTransponderData(s:%d,f:%d,p:%c,sRate:%d,fec%d", Code, frequency_, pol_, symbolrate_, fec_);
   return c->SetSatTransponderData(Code, frequency_, pol_, symbolrate_, fec_);
}

 
bool cSatTransponder::Parse(string Line)
{
   string tpNumb(Line);
   //    cerr << "Line : " << Line << endl;

   int index = tpNumb.find_first_of('=');
   if (index == -1)
      return false;

    //cerr << "index " << index << endl;
   tpNumb =  tpNumb.erase(index);

    //   chop  string
   string tmp = Line.substr(index+1);

    // get Frequenz
   index = tmp.find_first_of(',');

   if (index == -1)
      return false;

   string freq = tmp;  // tmp.substr(index);
   freq.erase(index);

    // get Polarisatzion
   string polar =  tmp.substr(index+1);
   index = polar.find_first_of(',');
   if (index == -1)
       return false;

    // get SymbolRate
   string symRate =  polar.substr(index+1);
   polar.erase(index);


   index = symRate.find_first_of(',');
   if (index == -1)
       return false;

   string sFec = symRate.substr(index+1);
   symRate.erase(index);


   //transponderNum_ = strToInt(tpNumb.c_str());
   channelNum_ = strToInt(tpNumb.c_str());
   frequency_ = strToInt(freq.c_str());
   if (frequency_ == -1) return false;
   pol_ = polar[0];
   symbolrate_ = strToInt(symRate.c_str());
   if (symbolrate_  == -1) return false;
   fec_ = IntToFec(strToInt(sFec.c_str()));

   return true;
}


//----------- Class cTerrTransponder -------------------------------

cTerrTransponder::cTerrTransponder(int ChannelNr, int Frequency, int Bandwith)
:cTransponder(Frequency)
{
   DLOG (DBG  " cTerr Channel: %d Frequency: %d, Bandwith %d",ChannelNr, Frequency, Bandwith);

   channelNum_ = ChannelNr;
   symbolrate_ = 27500;
   bandwidth_ = Bandwith;
   // fec is called Srate in vdr 
   fec_h_ = FEC_AUTO;
   fec_l_ = FEC_AUTO;
//   fec_h_ = FEC_2_3;
//   fec_l_ = FEC_2_3;
   hierarchy_ = HIERARCHY_NONE;
   modulation_ = FE_OFDM;
//   modulation_ = QAM_16;
   guard_ =  GUARD_INTERVAL_AUTO;
//   guard_ =  GUARD_INTERVAL_1_4;
   transmission_ = TRANSMISSION_MODE_AUTO;
//   transmission_ = TRANSMISSION_MODE_8K;
}

cTerrTransponder::~cTerrTransponder()
{
}

bool cTerrTransponder::SetTransponderData(cChannel *c, int Code)
{
   
   int type = cSource::stTerr;
   if (bandwidth_==BANDWIDTH_8_MHZ) {
       if (frequency_>=306*1000*1000)
           bandwidth_=BANDWIDTH_8_MHZ;
       else
           bandwidth_=BANDWIDTH_7_MHZ;
   }   
   else if (bandwidth_==BANDWIDTH_7_MHZ)
       bandwidth_=BANDWIDTH_7_MHZ;
   else
       bandwidth_=BANDWIDTH_8_MHZ;
       
//   printf (DBG " SetTerrTransponderData(s:%d,f: %d ,BW: %d, mod %d, Hier %d\n,fec_h %d/l %d,G %d,T %d ", type, 
//                 frequency_, bandwidth_, modulation_, hierarchy_, fec_h_, fec_l_, guard_, transmission_);
   return c->SetTerrTransponderData(type, frequency_, bandwidth_, modulation_, hierarchy_, fec_h_,
                                     fec_l_, guard_, transmission_);//
   
}
//----------- Class cCableTransponder -------------------------------

cCableTransponder::cCableTransponder(int ChannelNr, int Frequency, int Bandwith, int sRate, int Mod)
:cTransponder(Frequency)
{
   int modTab[6]={QAM_64,QAM_256,QPSK,QAM_16,QAM_32,QAM_128};
   // QAM_64 is Auto QAM64/QAM256

   DLOG (DBG  " new cCableTransponder Channel: %d f: %d, sRate :%d  BW %d ",ChannelNr, Frequency, sRate, Bandwith); 
    
   channelNum_ = ChannelNr;
   symbolrate_ = sRate;
   bandwidth_ = Bandwith;

   fec_h_ = FEC_AUTO;
   hierarchy_ = HIERARCHY_NONE;

   modulation_ = modTab[Mod];
}

cCableTransponder::~cCableTransponder()
{
}

bool cCableTransponder::SetTransponderData(cChannel *c, int Code)
{
   int type = cSource::stCable;

   DLOG(DBG " SetCableTransponderData(f:%d, m :%d ,sRate: %d, fec %d", frequency_,modulation_, symbolrate_,fec_h_);
   return c->SetCableTransponderData(type, frequency_, modulation_, symbolrate_, fec_h_);
}

//----------- Class Transponders -------------------------------

cTransponders::cTransponders()
:sourceCode_(0)
{
} 

void cTransponders::Load(int Source, scanParameters *scp)
{
  DLOG (DBG " LoadTransponders --  Autoscan   source: %d Symbolrate %d mod %d", Source, scp->symbolrate, scp->modulation);
  // type S/T/C
  Clear();
  sourceCode_ = Source;
  if (cSource::IsSat(sourceCode_))
  {
    //position_ = tr("Sat");
      if (!scp->frequency) {
          fileName_ = TplFileName(Source);
          position_ = SetPosition(fileName_);
          LoadTpl(fileName_);
      }
      else {
          lockMs_ = 500;
          cSatTransponder *t = new cSatTransponder(scp->frequency, scp->polarization, scp->symbolrate, scp->fec);
          v_tp_.push_back(t);
      }
  }
  else if (cSource::IsTerr(sourceCode_))
  {
      position_ = "Terrestrial";
      if (!scp->frequency) 
          CalcTerrTpl();
      else {
          int channel=0;
          cTerrTransponder *t = new cTerrTransponder(channel, scp->frequency*1000,  scp->bandwidth);
          v_tp_.push_back(t);
      }
  }
  else if (cSource::IsCable(sourceCode_))
  {
      position_ = "Cable";
      if (!scp->frequency)
          CalcCableTpl(0, scp);
      else {
          int channel=0;
          int sRate;
          if (scp->symbolrate_mode==2)
              sRate=scp->symbolrate;
          else
              sRate=6900;

          cCableTransponder *t = new cCableTransponder(channel, scp->frequency*1000, 
                                   scp->bandwidth?BANDWIDTH_7_MHZ:BANDWIDTH_8_MHZ, 
                                   sRate, scp->modulation);
          v_tp_.push_back(t);  
      }
  }
  else 
    esyslog(DBG "   Wrong  sourceCode %d",sourceCode_);

}

bool cTransponders::LoadTpl(string tplFileName)
{

  lockMs_ = 500;
  DLOG (DBG " in LoadSatTpls %s",tplFileName.c_str());
  cerr << "Loadini(): " << tplFileName << endl;

  sourceCode_ = cSource::FromString(position_.c_str());

  ifstream tpFH(tplFileName.c_str());

  if (!tpFH)
  {
    cerr << "can`t open File: " << tplFileName << endl;
    return false;
  }

      //< flash *this first

   char line[200];
   int lc = 0;
   while (tpFH.getline(line,200))
   {
     //unsigned int pos = 0;
     string::size_type  pos = 0;
     string l = line;
       // lookup for [
     pos =l.find('[');

     if (pos == l.npos)
     {
       cSatTransponder *t = new cSatTransponder();
       if (t->Parse(l))
          v_tp_.push_back(t);
     }
     lc++;
  }
  return true;
}


void cTransponders::CalcCableTpl(bool Complete, scanParameters *scp) 
{
   int bandwidth;
   int f, channel=0;
   int sRate, qam;



   Clear();

   if (scp->symbolrate_mode==2)
       sRate=scp->symbolrate;
   else
       sRate=6900;

   qam=scp->modulation;

   DLOG (DBG " CalcCableTpl : complete Scan %s sRate %d mod %d ", Complete?"YES":"NO", scp->symbolrate  ,scp->modulation);

   // Give the user the most popular cable channels first, speeds up SR auto detect
   for(channel=121;channel<200;channel++) {
       f=channel2Frequency(0,channel,bandwidth);
       if (f) {
           cCableTransponder *t = new cCableTransponder(channel, f, bandwidth, sRate , qam);
           v_tp_.push_back(t);
       }
   }

   for(channel=101;channel<121;channel++) {
       f=channel2Frequency(0,channel,bandwidth);
       if (f) {
           cCableTransponder *t = new cCableTransponder(channel, f, bandwidth, sRate , qam);
           v_tp_.push_back(t);
       }
   }

   for(channel=1;channel<100;channel++) {
       f=channel2Frequency(0,channel,bandwidth);
       if (f) {
           cCableTransponder *t = new cCableTransponder(channel, f, bandwidth, sRate , qam);
           v_tp_.push_back(t);
       }
   }

}

void cTransponders::CalcTerrTpl()
{
   Clear();
   int f = 0; 
   int channel = 0;
   int bandwidth;
   
   position_ = "Terrestrial";

   for (channel=5; channel <= 69; channel++) 
   {
       f = channel2Frequency(0,channel,bandwidth);
       if (f) {
           cTerrTransponder *t = new cTerrTransponder(channel, f, bandwidth);
           v_tp_.push_back(t);
       }
   } 
}

string cTransponders::SetPosition(string tplPath)
{
  if(cSource::IsSat(sourceCode_))
  { 
     string tmp = fileName_.substr(tplPath.find_last_of('/')+1);
     int index = tmp.find_last_of('.');
     tmp.erase(index);
     return tmp;
  }
  if(cSource::IsTerr(sourceCode_))
     return "DVB-T";

  if(cSource::IsCable(sourceCode_))
     return "DVB-C";

  return "";
}

string cTransponders::TplFileName(int satCodec)
{
  string tmp = cPlugin::ConfigDirectory();
     // TODO remove /plugins/ from tmp
  tmp += "/transponders/";
  char *buffer;
  asprintf(&buffer,"%s",*cSource::ToString(satCodec));
  tmp += buffer;
  tmp += ".tpl";
  //cerr << " ransponder list path: " << tmp << endl;
  return tmp;
}

void  cTransponders::Clear()
{
   for (vTpIter iter = v_tp_.begin(); iter != v_tp_.end(); ++iter)
       delete *iter;

    v_tp_.clear();
}
