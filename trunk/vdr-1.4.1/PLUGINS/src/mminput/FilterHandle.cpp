#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>

cFilterHandle::cFilterHandle(){
	for(int index = 0; index < MAXDEVICEFilter; index++){
		FH[index].PidNum = -1;
		FH[index].Whandle = -1;
		FH[index].Rhandle = -1;
		FH[index].Tid = -1;
		FH[index].length = 0;	
	}
}

cFilterHandle::~cFilterHandle(){
}

int cFilterHandle::CreatePipe(int Pid, int Tid){
	int index = 0;
	for(;index < MAXDEVICEFilter; index++){
		if(FH[index].PidNum == -1){
			break;		
		}		
	}
	if(index >= MAXDEVICEFilter){
		printf("Error!! try to create mor then %d filter pipes\n",MAXDEVICEFilter);
		return -1;
	}
	int fd[2];
	if(pipe(fd) == 0 ){
		FH[index].PidNum = Pid;
		FH[index].Whandle = fd[1]; // store handle for writing
		FH[index].Rhandle = fd[0]; // store handle for reading
		FH[index].Tid = Tid;
		FH[index].length = 0;
	printf("Create Pipe for pid %x, Tid %x \n",FH[index].PidNum, FH[index].Tid);		
		return FH[index].Rhandle; // give back handle for reading
	}
	return -1;
}

int cFilterHandle::Process(int Pid, char* buf){
	int length = 0;
	bool accept = false;
	if(Pid == 0x12 && buf[0] == 0x4e){
	printf("find pid and tid pid %x and tid %x\n",0x12,0x4e);
	}
	for(int index = 0;index < MAXDEVICEFilter; index++){
	if(FH[index].PidNum == -1) break;
		if(Pid == FH[index].PidNum && buf[0] == FH[index].Tid){
			accept = true;
//			printf("find pid %x and tid %x\n",FH[index].PidNum,FH[index].Tid);
			if((FH[index].length == 0 && buf[1] != 0x40) || FH[index].length != 0 ){
				printf("section starts here for pid %x and tid %x Handle %d\n",FH[index].PidNum,FH[index].Tid, index);
				int len = (((buf[1] & 0x0F) << 8) | (buf[2] & 0xFF)) + 3;
				if((len - length) > (188-4)){
					length = 188 - 4;
					memcpy(FH[index].buf + FH[index].length, buf + 3, length);
				}
				else{
					length = len - length;
					memcpy(FH[index].buf + FH[index].length, buf + 3, length);
					if (write(FH[index].Whandle,FH[index].buf, FH[index].length) <= FH[index].length){
						if(errno == EPIPE){
							printf("FilterHandle: write to pipe failed.\n");
							close(FH[index].Whandle);
							FH[index].PidNum = -1;
							FH[index].Tid = -1;
							FH[index].Whandle = -1;
							FH[index].Rhandle = -1;
							FH[index].length = 0;
						}
						else{
							FH[index].length = 0;						
						}
					}
				}
			}
		}
	}
	return accept;
}