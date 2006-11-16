#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>

//#include <vdr/tools.h>
#include "DPConnect/TSHandler.hpp"
#include "DPConnect/Streaming.hpp"

cFilterHandle::cFilterHandle(){
	
	FH = 0x0;
	maxFilter = 0;
	
}

cFilterHandle::~cFilterHandle(){
	for(int i = 0; i < maxFilter; i++){
		delete FH[i].tb ;
	}
	delete [] FH;
}

/*
	assum that eaach filter will create o
*/
int cFilterHandle::CreateFilter(int Pid, int Tid){
	
	if(FH == 0){
		maxFilter = 1;
		FH = new FilterPids[maxFilter];
		
		if(FH != 0){
			FH[0].PidNum = -1;
			FH[0].Whandle = -1;
			FH[0].Rhandle = -1;
			FH[0].Tid = -1;
			FH[0].length = 0;
			FH[0].tb = 0x0;
		}
		else{
			printf("cFilterHandle::CreateFilter: Couldn't create Filter\n");
			return -1;
		}
		
//		printf("cFilterHandle::CreateFilter: Create first Filter\n");
	}
	else{
//		printf("cFilterHandle::CreateFilter: Create Filter number %d\n",maxFilter);
		FilterPids *FHTmp = 0x0;
		FHTmp = new FilterPids[++maxFilter];
		if(FHTmp == 0){
			printf("Error. Couldn't resize array to store filters\n");
			return -1;
		}
		else{
			for(int i = 0; i < (maxFilter - 1); i++){
				FHTmp[i].PidNum		= FH[i].PidNum;
				FHTmp[i].Whandle	= FH[i].Whandle;
				FHTmp[i].Rhandle	= FH[i].Rhandle;
				FHTmp[i].Tid		= FH[i].Tid;
				FHTmp[i].length		= FH[i].length;
				FHTmp[i].tb			= FH[i].tb;
			}
			// delete old array
			// beware there is an memory leak !!!!!!!!!!
			delete [] FH;
			// store new pointer 
			FH = FHTmp;
		}
	}
	
//	printf("cFilterHandle::CreateFilter: Create Pipe for Filter.\n");

	int fd[2];
	if(pipe(fd) == 0 ){
		FH[maxFilter -1].PidNum = Pid;
		FH[maxFilter -1].Whandle = fd[1]; // store handle for writing
		FH[maxFilter -1].Rhandle = fd[0]; // store handle for reading
		FH[maxFilter -1].Tid = Tid;
		FH[maxFilter -1].length = 0;
		FH[maxFilter -1].tb = new TableBuilder();
		
//	printf("existing filter:\n");
//	for(int i = 0; i < maxFilter; i++){
//		printf("\tIndex: %d, Pid: 0x%x, Tid 0x%x, \n",i,FH[i].PidNum, FH[i].Tid);
//	}
		return FH[maxFilter -1].Rhandle; // give back handle for reading
	}
	return -1;
}

int cFilterHandle::Process(uchar* data){
int accept = 0;
static int countEIT = 0;
		TSHeader & tsh = *(TSHeader*)data;
		if (tsh.scrambling != 0) return -1;
		if (tsh.transportErr ) return -1;

	uint8_t * payload = data + 4;
	
	uint32_t pid = tsh.pid;
	uint8_t tid = payload[1];	

	TableSection * table;
	for(int i = 0; i < maxFilter; i++){
//		if(pid == 0x12 && i == 0) printf ("start filter search\n");
//			if(pid == 0x12 && tsh.payloadStart ) printf("check filter: pid 0x%x, 0x%x compare with pid 0x%x and tid 0x%x index %d: \n"
//				,pid,tid,FH[i].PidNum, FH[i].Tid,i);
		if(FH[i].PidNum == pid && FH[i].Tid == tid){	
			if(pid == 0x12 && tid == 0x4e){
				if(tsh.payloadStart) countEIT = 0;
				event_information & eit = *(event_information*)payload;
				printf("section length %d\n",eit.section_length);
				printf("last section number %d\n",eit.segment_last_section_number);
				printf("section number %d\n",eit.section_number);
				countEIT+= eit.section_length;
			}
			FH[i].tb->accumulate(tsh, payload);
			
			if((table = FH[i].tb->next()) == NULL){
				uint8_t* buf =  FH[i].tb->getTable();
				uint16_t len = (((buf[1] & 0x0F) << 8) | (buf[2] & 0xFF)) + 3;
				if(tid == 0x4e) printf("Table (pid 0x%x, tid 0x%x, length %d) complete\n",FH[i].PidNum, FH[i].Tid, len);
				if(tid == 0x4e) printf("Table count length %d\n",countEIT);
//				if(write(FH[i].Whandle, FH[i].tb->getTable(), FH[i].tb->length()) == EPIPE){
				if(write(FH[i].Whandle, FH[i].tb->getTable(), len) == EPIPE){
					printf("Error pipe does not exist\n");
				}
				FH[i].tb->clear();
//				printf("Table (pid 0x%x, length %d) cleared\n",FH[i].PidNum, FH[i].tb->length());
			}
//			else{
//				if(pid == 0x12 && tid == 0x4e){
//				event_information & eit = *(event_information*)table;
//				printf("section length %d\n",eit.section_length);
//				}
//			}
			break;
		}
	}
	return accept;
}