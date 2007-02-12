
#include "csmenu.h"
#include "filter.h"


// --- cCaDescriptor ---------------------------------------------------------

class cCaDescriptor : public cListObject {
private:
   int caSystem;
   bool stream;
   int length;
   uchar *data;
public:
   cCaDescriptor(int CaSystem, int CaPid, bool Stream, int Length, const uchar *Data);
   virtual ~cCaDescriptor();
   bool operator== (const cCaDescriptor &arg) const;
   int CaSystem(void) { return caSystem; }
   int Stream(void) { return stream; }
   int Length(void) const { return length; }
   const uchar *Data(void) const { return data; }
 };

cCaDescriptor::cCaDescriptor(int CaSystem, int CaPid, bool Stream, int Length, const uchar *Data)
{
   caSystem = CaSystem;
   stream = Stream;
   length = Length + 6;
   data = MALLOC(uchar, length);
   data[0] = SI::CaDescriptorTag;
   data[1] = length - 2;
   data[2] = (caSystem >> 8) & 0xFF;
   data[3] =  caSystem       & 0xFF;
   data[4] = ((CaPid   >> 8) & 0x1F) | 0xE0;
   data[5] =   CaPid         & 0xFF;
   if (Length)
     memcpy(&data[6], Data, Length);
}

cCaDescriptor::~cCaDescriptor()
{
  free(data);
}

bool cCaDescriptor::operator== (const cCaDescriptor &arg) const
{
  return length == arg.length && memcmp(data, arg.data, length) == 0;
}

// --- cCaDescriptors --------------------------------------------------------

class cCaDescriptors : public cListObject {
private:
  int source;
  int transponder;
  int serviceId;
  int numCaIds;
  int caIds[MAXCAIDS + 1];
  cList<cCaDescriptor> caDescriptors;
  void AddCaId(int CaId);
public:
  cCaDescriptors(int Source, int Transponder, int ServiceId);
  bool operator== (const cCaDescriptors &arg) const;
  bool Is(int Source, int Transponder, int ServiceId);
  bool Is(cCaDescriptors * CaDescriptors);
  bool Empty(void) { return caDescriptors.Count() == 0; }
  void AddCaDescriptor(SI::CaDescriptor *d, bool Stream);
  int GetCaDescriptors(const unsigned short *CaSystemIds, int BufSize, uchar *Data, bool &StreamFlag);
  const int *CaIds(void) { return caIds; }
  };

cCaDescriptors::cCaDescriptors(int Source, int Transponder, int ServiceId)
{
  source = Source;
  transponder = Transponder;
  serviceId = ServiceId;
  numCaIds = 0;
  caIds[0] = 0;
}

bool cCaDescriptors::operator== (const cCaDescriptors &arg) const
{
  cCaDescriptor *ca1 = caDescriptors.First();
  cCaDescriptor *ca2 = arg.caDescriptors.First();
  while (ca1 && ca2) {
     if (!(*ca1 == *ca2))
           return false;
      ca1 = caDescriptors.Next(ca1);
      ca2 = arg.caDescriptors.Next(ca2);
  }
  return !ca1 && !ca2;
}

bool cCaDescriptors::Is(int Source, int Transponder, int ServiceId)
{
  return source == Source && transponder == Transponder && serviceId == ServiceId;
}

bool cCaDescriptors::Is(cCaDescriptors *CaDescriptors)
{
  return Is(CaDescriptors->source, CaDescriptors->transponder, CaDescriptors->serviceId);
}

void cCaDescriptors::AddCaId(int CaId)
{
  if (numCaIds < MAXCAIDS) {
     for (int i = 0; i < numCaIds; i++) {
         if (caIds[i] == CaId)
            return;
     }
     caIds[numCaIds++] = CaId;
     caIds[numCaIds] = 0;
     }
}

void cCaDescriptors::AddCaDescriptor(SI::CaDescriptor *d, bool Stream)
{
  cCaDescriptor *nca = new cCaDescriptor(d->getCaType(), d->getCaPid(), Stream, d->privateData.getLength(), d->privateData.getData());
  for (cCaDescriptor *ca = caDescriptors.First(); ca; ca = caDescriptors.Next(ca)) {
      if (*ca == *nca) {
         delete nca;
         return;
       }
  }
  AddCaId(nca->CaSystem());
  caDescriptors.Add(nca);
}

int cCaDescriptors::GetCaDescriptors(const unsigned short *CaSystemIds, int BufSize, uchar *Data, bool &StreamFlag)
{
  if (!CaSystemIds || !*CaSystemIds)
     return 0;
  if (BufSize > 0 && Data) {
     int length = 0;
     int IsStream = -1;
     for (cCaDescriptor *d = caDescriptors.First(); d; d = caDescriptors.Next(d)) {
         const unsigned short *caids = CaSystemIds;
         do {
            if (*CaSystemIds == 0xFFFF || d->CaSystem() == *caids) {
               if (length + d->Length() <= BufSize) {
                  if (IsStream >= 0 && IsStream != d->Stream())
                     DLOG("CAM: different stream flag in CA descriptors");
                  IsStream = d->Stream();
                  memcpy(Data + length, d->Data(), d->Length());
                  length += d->Length();
                  }
               else
                  return -1;

             }
         } while (*++caids);
     }
     StreamFlag = IsStream == 1;
     return length;
     }
  return -1;
}

// --- cCaDescriptorHandler --------------------------------------------------

class cCaDescriptorHandler : public cList<cCaDescriptors> {
private:
  cMutex mutex;
public:
  int AddCaDescriptors(cCaDescriptors *CaDescriptors);
      // Returns 0 if this is an already known descriptor,
      // 1 if it is an all new descriptor with actual contents,
      // and 2 if an existing descriptor was changed.
  int GetCaDescriptors(int Source, int Transponder, int ServiceId, const unsigned short *CaSystemIds, int BufSize, uchar *Data, bool &StreamFlag);
  };

int cCaDescriptorHandler::AddCaDescriptors(cCaDescriptors *CaDescriptors)
{
  cMutexLock MutexLock(&mutex);
  for (cCaDescriptors *ca = First(); ca; ca = Next(ca)) {
      if (ca->Is(CaDescriptors)) {
         if (*ca == *CaDescriptors) {
            delete CaDescriptors;
            return 0;
            }
         Del(ca);
         Add(CaDescriptors);
         return 2;
         }
      }
  Add(CaDescriptors);
  return CaDescriptors->Empty() ? 0 : 1;
}

int cCaDescriptorHandler::GetCaDescriptors(int Source, int Transponder, int ServiceId, const unsigned short *CaSystemIds, int BufSize, uchar *Data, bool &StreamFlag)
{
  cMutexLock MutexLock(&mutex);
  StreamFlag = false;
  for (cCaDescriptors *ca = First(); ca; ca = Next(ca)) {
      if (ca->Is(Source, Transponder, ServiceId))
         return ca->GetCaDescriptors(CaSystemIds, BufSize, Data, StreamFlag);
      }
  return 0;
}

cCaDescriptorHandler CaDescriptorHandler;

int GetCaDescriptors(int Source, int Transponder, int ServiceId, const unsigned short *CaSystemIds, int BufSize, uchar *Data, bool &StreamFlag)
{
  return CaDescriptorHandler.GetCaDescriptors(Source, Transponder, ServiceId, CaSystemIds, BufSize, Data, StreamFlag);
}

// --- PatFilter ------------------------------------------------------------

PatFilter::PatFilter()
{
  sdtFilter=NULL;
  pmtIndex = 0;
  for (int i=0; i<MAXFILTERS; i++)
    pmtPid[i] = 0;
  numPmtEntries = 0;
  Set(0x00, 0x00);  // PAT
#ifdef REELVDR
  Set(0x1fff, 0x42);  // decrease latency
#endif
  endofScan=false;
  SetStatus(false);
  pit=pnum=0;
  sdtfinished=false;
  for(int n=0;n<128;n++)
    pSid[n]=-1;
  pSidCnt=0;
  lastFound=0;
  waitingForGodot=0;
}

void PatFilter::SetSdtFilter(SdtFilter *SdtFilter)
{
  sdtFilter=SdtFilter;
  SetStatus(true);
}

void PatFilter::SetStatus(bool On)
{
  cFilter::SetStatus(On);
  pmtIndex = 0;
  for (int i=0; i<MAXFILTERS; i++)
    pmtPid[i] = 0;
  numPmtEntries = 0;
  num=0;
  numRunning=0;
  pit=pnum=0;
}

void PatFilter::Trigger(void)
{
  numPmtEntries = 0;
}

bool PatFilter::PmtVersionChanged(int PmtPid, int Sid, int Version)
{
  uint64_t v = Version;
  v <<= 32;
  uint64_t id = (PmtPid | (Sid << 16)) & 0x00000000FFFFFFFFLL;
  for (int i = 0; i < numPmtEntries; i++) {
      if ((pmtVersion[i] & 0x00000000FFFFFFFFLL) == id) {
         bool Changed = (pmtVersion[i] & 0x000000FF00000000LL) != v;
         if (Changed)
            pmtVersion[i] = id | v;
         return Changed;
         }
      }
  if (numPmtEntries < CMAXPMTENTRIES)
     pmtVersion[numPmtEntries++] = id | v;
  return true;
}

bool PatFilter::SidinSdt(int Sid)
{
  for (int i=0; i<sdtFilter->numSid; i++)
    if (sdtFilter->sid[i]==Sid && sdtFilter->usefulSid[i])
    {
      for (int j=0; j<MAXFILTERS; j++)
        if (Sids[j]==Sid)
          return false;
      return true;
    }
  return false;
}

void PatFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length)
{
//  printf("PID %i TID %i\n",Pid,Tid);

  if (lastFound<time(NULL) && waitingForGodot<10) {  // There's something on this channel, waiting a bit longer...
    waitingForGodot++;
    lastFound++;
  }

  if (Pid == 0x00) {
    if (Tid == 0x00) {
      for (int i=0; i<MAXFILTERS; i++) {
        if (pmtPid[i] && (time(NULL) - lastPmtScan[i]) > FILTERTIMEOUT) {
//          printf("Del %i as %i\n", pmtPid[i], i);
          Del(pmtPid[i], 0x02);
          pmtPid[i] =0; // Note for recycling, but do not remove from feed
          pSid[pSidCnt++]=Sids[i];
          num++;
          for (int k=0; i<sdtFilter->numSid; k++)
            if (sdtFilter->sid[k]==Sids[i])
            {
              sdtFilter->sid[k]=0;
              break;
            }
          Sids[i]=0;
          pmtIndex++;
        }
      }
      SI::PAT pat(Data, false);
      if (!pat.CheckCRCAndParse())
        return;
      SI::PAT::Association assoc;

      int tnum=0,tSid[100];

      for (SI::Loop::Iterator it; pat.associationLoop.getNext(assoc, it); ) {
        int xSid=assoc.getServiceId();

        tSid[tnum++]=xSid;
        int sidfound=0;

        for(int n=0;n<pSidCnt;n++) {
          if (pSid[n]==xSid) {
            sidfound=1;
            break;
          }
        }
        
        if (!sidfound && !assoc.isNITPid() &&  SidinSdt(assoc.getServiceId())) {

          int Index = 0;
          int foundIndex=0;

          // Find free filter PID
          for(Index=0;Index<MAXFILTERS;Index++) {
            if (pmtPid[Index]==0) {
              foundIndex=1;
              break;
            }
          }
           
          if (foundIndex) {
            pmtPid[Index] = assoc.getPid();
            Sids[Index] = xSid;
            lastPmtScan[Index] = time(NULL);
//            printf("ADD %i as %i, Sid %i\n",pmtPid[Index],Index,xSid);
            Add(pmtPid[Index], 0x02);
            pSid[pSidCnt++]=xSid;
          }
        }
      }
    }
     }
  else if (Tid == SI::TableIdPMT && Source() && Transponder()) {
     int Index=-1;
     for (int i=0; i<MAXFILTERS; i++)
       if (Pid==pmtPid[i])
          Index=i;
     SI::PMT pmt(Data, false);
     if (!pmt.CheckCRCAndParse())
        return;
     if (!Channels.Lock(true, 10)) {
        numPmtEntries = 0; // to make sure we try again
        return;
        }
     cChannel *Channel = Channels.GetByServiceID(Source(), Transponder(), pmt.getServiceId());
     if (Channel && Index!=-1) {
        SI::CaDescriptor *d;
        cCaDescriptors *CaDescriptors = new cCaDescriptors(Channel->Source(), Channel->Transponder(), Channel->Sid());
        // Scan the common loop:
        for (SI::Loop::Iterator it; (d = (SI::CaDescriptor*)pmt.commonDescriptors.getNext(it, SI::CaDescriptorTag)); ) {
            CaDescriptors->AddCaDescriptor(d, false);
            delete d;
            }
        // Scan the stream-specific loop:
        SI::PMT::Stream stream;
        int Vpid = 0;
        int Ppid = pmt.getPCRPid();
        int Apids[MAXAPIDS + 1] = { 0 };
        int Dpids[MAXDPIDS + 1] = { 0 };
#if VDRVERSNUM >= 10332
        char ALangs[MAXAPIDS + 1][MAXLANGCODE2] = { "" };
        char DLangs[MAXDPIDS + 1][MAXLANGCODE2] = { "" };
#else
        char ALangs[MAXAPIDS + 1][4] = { "" };
        char DLangs[MAXDPIDS + 1][4] = { "" };
#endif
        int Tpid = 0;
        int NumApids = 0;
        int NumDpids = 0;
        for (SI::Loop::Iterator it; pmt.streamLoop.getNext(stream, it); ) {
//     printf("PID: %5d %5d %2d %3d %3d\n", pmt.getServiceId(), stream.getPid(), stream.getStreamType(), pmt.getVersionNumber(), Channel->Number());//XXX
            switch (stream.getStreamType()) {
              case 1: // STREAMTYPE_11172_VIDEO
              case 2: // STREAMTYPE_13818_VIDEO
                      Vpid = stream.getPid();
                      break;
              case 3: // STREAMTYPE_11172_AUDIO
              case 4: // STREAMTYPE_13818_AUDIO
                      {
                      if (NumApids < MAXAPIDS) {
                         Apids[NumApids] = stream.getPid();
                         SI::Descriptor *d;
                         for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); ) {
                             switch (d->getDescriptorTag()) {
                               case SI::ISO639LanguageDescriptorTag: {
                                    SI::ISO639LanguageDescriptor *ld = (SI::ISO639LanguageDescriptor *)d;
                                    if (*ld->languageCode != '-') { // some use "---" to indicate "none"
                                       strn0cpy(ALangs[NumApids], I18nNormalizeLanguageCode(ld->languageCode), 4);
                                       ALangs[NumApids][4] = 0;
                                       }
                                    }
                                    break;
                               default: ;
                               }
                             delete d;
                             }
                         NumApids++;
                         }
                      }
                      break;
              case 5: // STREAMTYPE_13818_PRIVATE
              case 6: // STREAMTYPE_13818_PES_PRIVATE
              //XXX case 8: // STREAMTYPE_13818_DSMCC
                      {
                      int dpid = 0;
                      char lang[4] = { 0 };
                      SI::Descriptor *d;
                      for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); ) {
                          switch (d->getDescriptorTag()) {
                            case SI::AC3DescriptorTag:
                                 dpid = stream.getPid();
                                 break;
                            case SI::TeletextDescriptorTag:
                                 Tpid = stream.getPid();
                                 break;
                            case SI::ISO639LanguageDescriptorTag: {
                                 SI::ISO639LanguageDescriptor *ld = (SI::ISO639LanguageDescriptor *)d;
                                 strn0cpy(lang, I18nNormalizeLanguageCode(ld->languageCode), 4);
                                 }
                                 break;
                            default: ;
                            }
                          delete d;
                          }
                      if (dpid) {
                         if (NumDpids < MAXDPIDS) {
                            Dpids[NumDpids] = dpid;
                            strn0cpy(DLangs[NumDpids], lang, 4);
                            NumDpids++;
                            }
                         }
                      }
                      break;
              }
            for (SI::Loop::Iterator it; (d = (SI::CaDescriptor*)stream.streamDescriptors.getNext(it, SI::CaDescriptorTag)); ) {
                CaDescriptors->AddCaDescriptor(d, true);
                delete d;
                }
            }
        Channel->SetPids(Vpid, Vpid ? Ppid : 0, Apids, ALangs, Dpids, DLangs, Tpid);
  printf("#### %i %s %i %i SID  %i\n",num,Channel->Name(),Vpid, Apids[0], Channel->Sid());
        Channel->SetCaIds(CaDescriptors->CaIds());
        Channel->SetCaDescriptors(CaDescriptorHandler.AddCaDescriptors(CaDescriptors));

//  printf("Del %i as %i\n", pmtPid[Index], Index);
  Del(pmtPid[Index], 0x02);
  pmtPid[Index]=0;

  num++;
  numRunning++;
  lastFound=time(NULL);
        }
#if 0
     if (Index!=-1)
       lastPmtScan[Index] = 0; // this triggers the next scan
#endif
     Channels.Unlock();
     }
  if (sdtfinished && num>=sdtFilter->numUsefulSid) {
//    printf("#### %i %i\n", num,sdtFilter->numSid);
    endofScan=true;
  }
}
void PatFilter::GetFoundNum(int &current, int &total)
{
  current=numRunning;
  total=(sdtFilter?sdtFilter->numSid:0);
  if (total>1000 || total<0)
    total=0;
}

// --- cSdtFilter ------------------------------------------------------------

SdtFilter::SdtFilter(PatFilter *PatFilter)
{      
  patFilter = PatFilter;
  numSid=0;
  numUsefulSid=0;
  Set(0x11, 0x42);  // SDT
#ifdef REELVDR
  Set(0x1fff, 0x42);  // decrease latency
#endif
  AddServiceType = cMenuChannelscanSetup::SetupServiceType;
}

void SdtFilter::SetStatus(bool On)
{
  cFilter::SetStatus(On);
  sectionSyncer.Reset();
}

void SdtFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length)
{
  if (!(Source() && Transponder()))
     return;
  SI::SDT sdt(Data, false);
  if (!sdt.CheckCRCAndParse())
     return;
  if (!sectionSyncer.Sync(sdt.getVersionNumber(), sdt.getSectionNumber(), sdt.getLastSectionNumber()))
     return;
  if (!Channels.Lock(true, 10))
     return;
  SI::SDT::Service SiSdtService;
  for (SI::Loop::Iterator it; sdt.serviceLoop.getNext(SiSdtService, it); ) {
     cChannel *channel = Channels.GetByChannelID(tChannelID(Source(), sdt.getOriginalNetworkId(), sdt.getTransportStreamId(), SiSdtService.getServiceId()));
      if (!channel)
         channel = Channels.GetByChannelID(tChannelID(Source(), 0, Transponder(), SiSdtService.getServiceId()));
      cLinkChannels *LinkChannels = NULL;
      SI::Descriptor *d;
      for (SI::Loop::Iterator it2; (d = SiSdtService.serviceDescriptors.getNext(it2)); ) {
          switch (d->getDescriptorTag()) {
            case SI::ServiceDescriptorTag: {
                 SI::ServiceDescriptor *sd = (SI::ServiceDescriptor *)d;
                    char NameBufDeb[1024];
                    char ShortNameBufDeb[1024];
                 sd->serviceName.getText(NameBufDeb, ShortNameBufDeb, sizeof(NameBufDeb), sizeof(ShortNameBufDeb));
                 // printf(" %s --  ServiceType: %X: AddServiceType %d, Sid %i, running %i\n",
                 // NameBufDeb, sd->getServiceType(), AddServiceType, SiSdtService.getServiceId(), SiSdtService.getRunningStatus());

                 switch (sd->getServiceType()) {
                   case 0x01: // digital television service
                   case 0x02: // digital radio sound service
                   case 0x03: // DVB Subtitles
                   case 0x04: // NVOD reference service
                   case 0x05: // NVOD time-shifted service
                   case 0x11: // MPEG-2 HD digital television service) break;
                   case 0xC3: // some french channels like kiosk
                        {
                        // Add only radio 
                          if ((sd->getServiceType()==1 || sd->getServiceType()==0x11 || sd->getServiceType()==0xC3) && AddServiceType == 1) 
                          { 
                             printf(" Add nur Radio  aber nur TV Sender gefunden  SID skip %d \n",sd->getServiceType());  
                             break;
                          }
                          // Add only tv
                          if (sd->getServiceType()==2 && (AddServiceType==0))
                          { 
                             printf(" Add nur TV  aber nur RadioSender gefunden  SID skip %d \n",sd->getServiceType());  
                             break;
                          }
                          char NameBuf[1024];
                          char ShortNameBuf[1024];
                          char ProviderNameBuf[1024];
                          sd->serviceName.getText(NameBuf, ShortNameBuf, sizeof(NameBuf), sizeof(ShortNameBuf));
                          char *pn = compactspace(NameBuf);
                          char *ps = compactspace(ShortNameBuf);
                          sd->providerName.getText(ProviderNameBuf, sizeof(ProviderNameBuf));
                          char *pp = compactspace(ProviderNameBuf);

                          mutexNames.Lock();
                          switch(sd->getServiceType())
                          {
                             case 1: tvChannelNames.push_back(NameBuf); // if service wanted 
                                     break;
                             case 2: radioChannelNames.push_back(NameBuf); // if serive wanted 
                                     break;
                             default : dataChannelNames.push_back(NameBuf);
                                     break;
                          }
                          mutexNames.Unlock();
                    
                          if ( SiSdtService.getRunningStatus()>SI::RunningStatusNotRunning) {
                            usefulSid[numSid]=1;
                            numUsefulSid++;
                          }
                          else 
                            usefulSid[numSid]=0;

                          sid[numSid++]=SiSdtService.getServiceId();
                    
                          if (channel) {
                            printf(" ---------------------- Channelscan Add Chanel pn %s ps %s pp %s -------------- \n",  pn, ps, pp); 
                            channel->SetId(sdt.getOriginalNetworkId(), sdt.getTransportStreamId(), SiSdtService.getServiceId(),channel->Rid());
                            //if (Setup.UpdateChannels >= 1)
                            channel->SetName(pn, ps, pp);
                               // Using SiSdtService.getFreeCaMode() is no good, because some
                               // tv stations set this flag even for non-encrypted channels :-(
                               // The special value 0xFFFF was supposed to mean "unknown encryption"
                               // and would have been overwritten with real CA values later:
                               // channel->SetCa(SiSdtService.getFreeCaMode() ? 0xFFFF : 0);
                               }
                             else if (*pn) {
                                channel = Channels.NewChannel(Channel(), pn, ps, pp, sdt.getOriginalNetworkId(), sdt.getTransportStreamId(), SiSdtService.getServiceId());
                               patFilter->Trigger();
                               if (SiSdtService.getServiceId() == 0x12) {
                                  printf(DBG  "-------- found ServiceID for PremiereDirekt!  %s - %s - %s --------- \n",  pn, ps, pp);
                               //eitFilter->Trigger();
                               }


                               }
                          } // end case Digital TV services 
                      }
                   }
         break;
         case SI::NVODReferenceDescriptorTag: {
                 SI::NVODReferenceDescriptor *nrd = (SI::NVODReferenceDescriptor *)d;
                 SI::NVODReferenceDescriptor::Service Service;
                 for (SI::Loop::Iterator it; nrd->serviceLoop.getNext(Service, it); ) {
                     cChannel *link = Channels.GetByChannelID(tChannelID(Source(), Service.getOriginalNetworkId(), Service.getTransportStream(), Service.getServiceId()));
                     if (!link) {
#if 0
           if (SiSdtService.getRunningStatus()>SI::RunningStatusNotRunning) {
               usefulSid[numSid]=1;
          numUsefulSid++;
           }
           else
#endif
               usefulSid[numSid]=0;

           sid[numSid++]=SiSdtService.getServiceId();
           link = Channels.NewChannel(Channel(), "NVOD", "", "", Service.getOriginalNetworkId(), Service.getTransportStream(), Service.getServiceId());
           patFilter->Trigger();
         }
                     if (link) {
                        if (!LinkChannels)
                           LinkChannels = new cLinkChannels;
                        LinkChannels->Add(new cLinkChannel(link));
                        }
                     }
                 }
                 break;
            default: ;
            }
          delete d;
          }
      if (LinkChannels) {
         if (channel)
            channel->SetLinkChannels(LinkChannels);
         else
            delete LinkChannels;
         }
     }
  Channels.Unlock();
  if (sdt.getSectionNumber() == sdt.getLastSectionNumber()) {
     patFilter->SdtFinished();
     SetStatus(false);
     }
}
