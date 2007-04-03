#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>

//#define FILDEB(out...) printf(out)

#ifndef FILDEB
#define FILDEB(out...)
#endif
#define MaxEPGSectionLength 4096

class cSectionFilter {
public:
	uint8_t buffer[MaxEPGSectionLength];
	int pos;
	uint8_t last_cc;
	
	cSectionFilter();
	~cSectionFilter();
	
	bool Process(int fd, uint8_t *tspkt);
	
	inline bool  PUSI( uint8_t *tspkt) 
         { return tspkt[1] & 0x40; }; 
        
	inline int PayloadLength( uint8_t *tspkt) { 
		if ( !(tspkt[3] & 0x10) ) // check if we have an adaption filed (optional)
			return 0;
		if ( tspkt[3] & 0x20 ) { 
			if (tspkt[4] > 183) return 0;
			else 
				return (184 - 1 - tspkt[4]);
		};
		return 184;
	}
	void TSdump();
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

inline void cSectionFilter::TSdump() {
        printf("Section info: pid %d, length %d, pointer pos %d, num of save sections %d\n",
               TSinfo.pid, TSinfo.length, TSinfo.pos,TSinfo.numSection);
};

bool cSectionFilter::Process(int fd, uint8_t *tspkt) {
        int len=PayloadLength(tspkt);
        int cc = tspkt[3] & 0x0f;
        bool cc_ok=((last_cc + 1) & 0x0f) == cc;
        last_cc=cc;
        
        if(!len) return true;
        
        int p_start = 188-len;
        
        if ( ((tspkt[3] & 0x20) && (tspkt[4]>0) && (tspkt[5] & 0x80)) 
             // adaption field and discontinuity indicator present
             || !cc_ok) { // or discontinuity detected
                if(WaitForNewSection == false){
                        FILDEB("discontinuity..\n");
                }
                WaitForNewSection = true;
                pos = 0;
        }
        
        if( PUSI(tspkt) == 0 && WaitForNewSection == true){
                return true;
        }
        
        if (PUSI(tspkt) ) {
                TSinfo.length = (((tspkt[2] & 0xf) << 8) + tspkt[3])&0x3ff; // section length has 12 bit
                TSinfo.pos = 0;
                TSinfo.numSection =0;
                TSinfo.pid = (((tspkt[1] & 0x1f) << 8) + tspkt[2])&0x1ff; // pid has 13 bit
                                                                          //			FILDEB("cSectionFilter::Process: start new section..\n");		
                WaitForNewSection = false;
                uint8_t *remainder_start=&tspkt[p_start+1];
                uint8_t remainder_len=tspkt[p_start];
                
                if( (remainder_len +p_start+1) >= 188 ){
                        printf("Error remainder length is to long (%d) Wait for new section\n"
                               ,remainder_len);
                        pos = 0;
                        WaitForNewSection = true;					
                        TSdump();
                        return true;
                }
                
                uint8_t *new_start=&remainder_start[remainder_len];
                int new_len= len-1-remainder_len;
                if(new_len <= 0){
                        printf("Error! New length is lower then 0 (%d) Wait for new section\n",new_len);
                        pos = 0;
                        WaitForNewSection = true;					
                        return true;					
                }
                if(new_len >= 184){
                        printf("Error! New length is too long(%d) Wait for new section\n",new_len);
                        pos = 0;
                        WaitForNewSection = true;					
                        TSdump();
                        return true;					
                }
                
                if (!pos) {
                        //no packet to finish
                        if(pos + new_len >= MaxEPGSectionLength){
                                printf("estimated section too long (pos == %d)!! %d\n",pos, pos + new_len);
                                TSdump();
                                return false;
                        }
                        memcpy(&buffer[pos],new_start,new_len);
                        pos+=new_len;
                        TSinfo.pos = pos;
                        TSinfo.numSection++;
                } else {
                        if( (pos + remainder_len) >= MaxEPGSectionLength){
                                printf("cSectionFilter::Process: Section buffer overflow! (%d)\n",pos + remainder_len);
                                pos = 0;
                                WaitForNewSection = true;
                                TSdump();
                                return true;
                        }
                        memcpy(&buffer[pos],remainder_start,remainder_len);
                        pos+=remainder_len;
                        TSinfo.pos = pos;
                        TSinfo.numSection++;
                        
                        int section_len = (((buffer[1] & 0x0F) << 8) | (buffer[2] & 0xFF)) + 3;
                        //Dump(buffer);
                        if (pos >= section_len) {
                                
                                if (section_len != write(fd,buffer,section_len) ) {
                                        // FIXME check errno
                                        return false;
                                }
                                
                        } else {
                                printf("cSectionFilter::Process:was mach ich hier?\n");
                                
                                int section_start=section_len;
                                section_len = (((buffer[section_start+1] & 0x0F) << 8) | (buffer[section_start+2] & 0xFF)) + 3;
                                //                                printf("2:pos %d section_len %d\n",pos,section_len);
                                Dump(&buffer[section_start-5]);
                                TSdump();
                                pos=0;
                        }                        
                        pos=0;
                        // start new section
                        memcpy(&buffer[pos],new_start,new_len);
                        pos+=new_len;
                        TSinfo.pos = pos;
                        TSinfo.numSection++;
                }
        } else {
                if(WaitForNewSection == false){ 
                        //printf("copy payload...\n");
                        if( (pos + len >= MaxEPGSectionLength)){
                                
                                int tmpLen = MaxEPGSectionLength - pos -1;
                                if(tspkt[p_start + tmpLen] != 0xFF){
                                        printf("cSectionFilter::Process:estimated section too long (pos = %d)!! %d\n",pos, pos + len);
                                        TSdump();
                                        return true;
                                }
                                len = tmpLen;
                        }
                        memcpy(&buffer[pos],&tspkt[p_start],len);
                        pos+=len;
                        TSinfo.pos = pos;
                        TSinfo.numSection++;
                }
        };
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
                for (int i=0; i<MAXDEVICEFILTER; i++)
                        fprintf(stderr,"0x%02x ",PidNum[pos]);
                fprintf(stderr,"\n");
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
        
        for(int i = 0; i < MAXDEVICEFILTER; i++){
                //printf("Pidnum[%d] 0x%02x\n",i,PidNum[pos]);
                if (PidNum[i] == pid){ 	
                        if ( !FH[i].sf->Process(FH[i].Whandle,data) ) {
                                // close pipe
                                FILDEB("Close Filter pid 0x%02x, tid 0x%02x \n",
                                       PidNum[i],FH[i].Tid);
                                PidNum[i]=-1;
                                delete FH[i].sf;
                                FH[i].sf=NULL;
                                close(FH[i].Whandle);
                        };
                        return 1;
                }
        }
        return 0;
}
