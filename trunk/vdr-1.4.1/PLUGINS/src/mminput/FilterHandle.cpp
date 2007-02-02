#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>

//#include <vdr/tools.h>
#include "DPConnect/TSHandler.hpp"
#include "DPConnect/Streaming.hpp"

//#define FILDEB(out...) printf(out)

#ifndef FILDEB
#define FILDEB(out...)
#endif

class cSectionFilter {
        public:
                uint8_t buffer[4096];
                int pos;
                int last_cc;

                cSectionFilter();
                ~cSectionFilter();
                
                bool Process(int fd, uint8_t *tspkt);

                inline bool  PUSI( uint8_t *tspkt) 
                { return tspkt[1] & 0x40; };

                inline int PayloadLength( uint8_t *tspkt) 
                { 
                        if ( !(tspkt[3] & 0x10) )
                                return 0;
                        if ( tspkt[3] & 0x20 ) {
                                if (tspkt[4] > 183) 
                                        return 0;
                                else 
                                        return 184 - 1 - tspkt[4];
                        };
                        return 184;
                }
};

cSectionFilter::cSectionFilter() 
        : pos(0), last_cc(0xFF) {
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
        int cc= tspkt[3] & 0x0f;

        if (!len)
                return true;

        int p_start=188-len;

        bool cc_ok=((last_cc + 1) & 0x0f) == cc;
        //printf("got packet cc: %d cc_ok: %d  len %d \n",cc, cc_ok, len);
        //Dump(tspkt);
        last_cc=cc;

        if ( ((tspkt[3] & 0x20) && (tspkt[4]>0) && (tspkt[5] & 0x80)) 
             // adaption field and discontinuity indicator present
             || !cc_ok) { // or discontinuity detected
                pos=0;
                FILDEB("discontinuity..\n");
        }

        if (PUSI(tspkt) ) {
                uint8_t *remainder_start=&tspkt[p_start+1];
                int remainder_len=tspkt[p_start];

                uint8_t *new_start=&remainder_start[remainder_len];
                int new_len= len-1-remainder_len;
                //printf("found start of packet remainder_len %d new_len %d \n", 
                //        remainder_len, new_len);
                if (!pos) {
                        //no packet to finish
                        memcpy(&buffer[pos],new_start,new_len);
                        pos+=new_len;
                } else {
                        memcpy(&buffer[pos],remainder_start,remainder_len);
                        pos+=remainder_len;

                        int section_len = (((buffer[1] & 0x0F) << 8) | (buffer[2] & 0xFF)) + 3;
                        //printf("pos %d section_len %d\n",pos,section_len);
                        //Dump(buffer);
                        if (pos >= section_len) {
                                if (section_len != write(fd,buffer,section_len) ) {
                                        // FIXME check errno
                                        return false;
                                }
                                //fsync(fd);
                                //usleep(5000);
                        } else {
                                printf("pos %d section_len %d\n",pos,section_len);
                                Dump(buffer);
                                printf("was mach ich hier?\n");

                                int section_start=section_len;
                                section_len = (((buffer[section_start+1] & 0x0F) << 8) | (buffer[section_start+2] & 0xFF)) + 3;
                                printf("2:pos %d section_len %d\n",pos,section_len);
                                Dump(&buffer[section_start-5]);
                                pos=0;
                        }                        
                        pos=0;
                        // start new section
                        memcpy(&buffer[pos],new_start,new_len);
                        pos+=new_len;
                }
        } else {
                //printf("copy payload...\n");
                memcpy(&buffer[pos],&tspkt[p_start],len);
                pos+=len;
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

/*
	assum that eaach filter will create o
*/
int cFilterHandle::CreateFilter(int Pid, int Tid){
        int pos=0;
        FILDEB("CreateFilter pid 0x%02x tid 0x%02x \n",Pid,Tid);

/*        if (Pid != 0x12) {
                printf("stop 0x%x\n",Pid);
                return -1;
        };
*/
        
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
