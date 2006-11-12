#include "FilterHandle.hpp"
#include <unistd.h>
#include <string.h>

//#include <vdr/tools.h>
#include "DPConnect/TSHandler.hpp"
#include "DPConnect/Streaming.hpp"

cFilterHandle::cFilterHandle(){
	
	FH = new FilterPids[MAXDEVICEFilter];
	
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
		FH[index].tb = new TableBuilder();
		
	printf("Create Pipe for pid %x, Tid %x \n",FH[index].PidNum, FH[index].Tid);		
		return FH[index].Rhandle; // give back handle for reading
	}
	return -1;
}

int cFilterHandle::Process(uchar* data){
int accept = 0;
		TSHeader & tsh = *(TSHeader*)data;
	if (tsh.syncByte != 0x47) return 0;
	//if (tsh.transportErr) return;

	uint8 * payload = data + 4;
	
	uint32 pid = tsh.pid;
	
	// should do a lot better than this switch!
	// set a function pointer to represent the current state...
	
	switch (pid){
	case (0x12):{
		if (tsh.scrambling != 0) return 0;
//		switch(tid){
//			case (0x4e):{
//			}
//		}
	
	}
	default:{
	return 0;
	}
	}

	return accept;
}