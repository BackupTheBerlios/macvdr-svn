#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

//#define FILDEB(out...) printf(out)

#ifndef FILDEB
#define FILDEB(out...)
#endif
#define MaxEPGSectionLength 4096

class cSectionFilter {
public:
        uint8_t buffer[MaxEPGSectionLength];
        int pos;
        int last_cc;
        
        cSectionFilter();
        ~cSectionFilter();
        
        bool Process(int fd, uint8_t *tspkt);
        
        inline bool  PUSI( uint8_t *tspkt) 
         { return tspkt[1] & 0x40; }; 
        
        inline int PayloadLength( uint8_t *tspkt) 
         { 
                if ( !(tspkt[3] & 0x10) ) // check if we have an adaption filed (optional)
                        return 0;
                if ( tspkt[3] & 0x20 ) { 
                        if (tspkt[4] > 183) 
                                return 0;
                        else 
                                return 184 - 1 - tspkt[4];
                };
                return 184;
         }
private:
        bool WaitForNewSection;
        struct TSdump{
                int length;
                int pos;
                int numSection;
                int pid;
        } TSinfo;
};

cSectionFilter::cSectionFilter() 
        : pos(0), last_cc(0xFF), WaitForNewSection(false) {
};

cSectionFilter::~cSectionFilter() {
};

static inline void Dump(uint8_t *data) {
        int i=0;
        while (i<20)
                printf("%02x ",data[i++]);
        printf("\n");
};

bool cSectionFilter::Process(int fd, uint8_t *tspkt) {

        int len=PayloadLength(tspkt);
        int cc = tspkt[3] & 0x0f;
        bool cc_ok=((last_cc + 1) & 0x0f) == cc;
        last_cc=cc;
        
        if (!len)
                return true;
        
        if( PUSI(tspkt) == 0 && WaitForNewSection == true){
                //printf("cSectionFilter::Process: wait for new section\n");
                return true;
        }
        
        
        if ( ((tspkt[3] & 0x20) && (tspkt[4]>0) && (tspkt[5] & 0x80)) 
             // adaption field and discontinuity indicator present
             || !cc_ok) { // or discontinuity detected
                if(WaitForNewSection == false){
                        FILDEB("discontinuity..\n");
                }
                WaitForNewSection = true;
                pos = 0;
                return true;
        }
        
        int p_start = 188 - len;
        if (PUSI(tspkt) ) {
                FILDEB("cSectionFilter::Process: start new section..\n");

                TSinfo.length = (((tspkt[2] & 0xf) << 8) + tspkt[3])&0x3ff; // section length has 12 bit
                TSinfo.pos = 0;
                TSinfo.numSection =0;
                TSinfo.pid = (((tspkt[1] & 0x1f) << 8) + tspkt[2])&0x1ff; // pid has 13 bit
                
                WaitForNewSection = false;
                uint8_t *remainder_start = &tspkt[p_start+1];
                int remainder_len = tspkt[p_start];

                if (p_start+1+remainder_len>188) {
                        printf("cSectionFilter::Process:remainder_len too large (%d)!! \n",remainder_len);
                        WaitForNewSection = true;
                        pos = 0;
                        TSdump();
                        return true;
                }
                
                if (pos) {
                        // finish the old sectioin
                        if( (pos + remainder_len >= MaxEPGSectionLength)){
                                // section is longer than MaxSectionLength, check for padding bytes
                                // and leave them out...
                                int tmpLen = MaxEPGSectionLength - pos - 1;
                                if(tspkt[p_start + tmpLen] != 0xFF){
                                        printf("cSectionFilter::Process:estimated section too long (pos = %d)!! %d\n",pos, pos + len);
                                        WaitForNewSection = true;
                                        pos = 0;
                                        TSdump();
                                        return true;
                                }
                                len = tmpLen;
                        }
                        memcpy(&buffer[pos],remainder_start,remainder_len);
                        pos+=remainder_len;
                        TSinfo.pos = pos;
                        TSinfo.numSection++;

                        int section_len = (((buffer[1] & 0x0F) << 8) | (buffer[2] & 0xFF)) + 3;
                        if (pos == section_len || 
                            (pos > section_len && buffer[section_len] == 0xFF ) ) {
                                // write the complete section to the pipe
                                if (section_len != write(fd,buffer,section_len) ) {
                                        // FIXME check errno
                                        return false;
                                }
                        } else {
                                // the previos section should be finished, but the size is wrong..
                                // Discard the section.
                                printf("cSectionFilter::Process:was mach ich hier? section_len %d, pos %d\n", section_len, pos);
                                Dump(&buffer[section_len -5]);
                                TSdump();
                                pos=0;
                        }        
                };
                
                // start new section
                pos=0;
                int new_len= len-1-remainder_len;
                memcpy(&buffer[pos],&remainder_start[remainder_len],new_len);
                pos+=new_len;
                TSinfo.pos = pos;
                TSinfo.numSection++;

                return true;
        } 
        
        //printf("copy payload...\n");
        if( (pos + len >= MaxEPGSectionLength)){
                // section is longer than MaxSectionLength, check for padding bytes
                // and leave them out...
                int tmpLen = MaxEPGSectionLength - pos - 1;
                if(tspkt[p_start + tmpLen] != 0xFF){
                        printf("cSectionFilter::Process:estimated section too long (pos = %d)!! %d\n",pos, pos + len);
                        WaitForNewSection = true;
                        pos = 0;
                        return true;
                }
                len = tmpLen;
        }
        memcpy(&buffer[pos],&tspkt[p_start],len);
        pos+=len;
        TSinfo.pos = pos;
        TSinfo.numSection++;
        return true;
};


//--------------------------------cFilterHandle---------------------------------------------


cFilterHandle::cFilterHandle(){
	maxFilter = 0;
        
        memset(FH,0xFF,sizeof(FH));
        memset(PidNum,0xFF,sizeof(PidNum));
}

cFilterHandle::~cFilterHandle(){
	for(int i = 0; i < maxFilter; i++){
		delete FH[i].sf ;
	}
}

// open a pipe for each filter and return it to vdr,
// but only the first filter which matches a pid will recieve sections.
// This should not be a problem, since vdr distributes the sections for us :-)
int cFilterHandle::CreateFilter(int Pid, int Tid){
        int pos=0;
        FILDEB("CreateFilter pid 0x%02x tid 0x%02x \n",Pid,Tid);
        
        // check for closed pipes
        for (pos=0; pos < MAXDEVICEFILTER ; pos++)
                if ( PidNum[pos] != -1) {
                        if ( 0 != write(FH[pos].Whandle,NULL,0) ) {
                                // close pipe
                                FILDEB("Pipe not open. Close Filter pid 0x%02x, tid 0x%02x \n",
                                       PidNum[pos],FH[pos].Tid);
                                PidNum[pos]=-1;
                                delete FH[pos].sf;
                                FH[pos].sf=NULL;
                                close(FH[pos].Whandle);
                        }
                }
                        
                        pos=0;
        while (pos < MAXDEVICEFILTER && PidNum[pos]!=-1 )
                pos++;
        
        if (pos >= MAXDEVICEFILTER) {
                fprintf(stderr," Too many open filters! \n");
                return -1; 	
        };
	FILDEB("cFilterHandle::CreateFilter: Create Pipe for Filter.\n");
        
	int fd[2];
	if(pipe(fd) == 0 ){
		PidNum[pos] = Pid;
		FH[pos].Whandle = fd[1]; // store handle for writing
		FH[pos].Rhandle = fd[0]; // store handle for reading
		FH[pos].Tid = Tid;
		FH[pos].length = 0;
		FH[pos].sf = new cSectionFilter();
		
		return FH[pos].Rhandle; // give back handle for reading
	}
	return -1;
}

int cFilterHandle::Process(uchar* data){
        int pid =  ((data[1] & 0x1f) << 8) + data[2];
        //FILDEB("Process %p pid 0x%02x\n",data,pid);	
        
        int i=0;
        while (i<MAXDEVICEFILTER && PidNum[i]!=pid)
                i++;
        
        if (i>=MAXDEVICEFILTER)
                return 0;
        
        if ( !FH[i].sf->Process(FH[i].Whandle,data) ) {
                // write to pipe failed, close it
                FILDEB("Close Filter pid 0x%02x, tid 0x%02x \n",
                       PidNum[i],FH[i].Tid);
                PidNum[i]=-1;
                delete FH[i].sf;
                FH[i].sf=NULL;
                close(FH[i].Whandle);
        };
        return 1;
}
