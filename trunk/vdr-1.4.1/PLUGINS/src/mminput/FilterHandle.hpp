/*
 *  $Id: device.h,v 1.3 2005/02/08 15:21:19 lordjaxom Exp $
 */
 
#ifndef VDR_FILTERHANDLE_H
#define VDR_FILTERHANDLE_H

#define MAXDEVICEFilter 16

#include<vector>

class cFilterHandle {

private:

	struct FilterPids{
	int PidNum;
	int Rhandle;
	int Whandle;
	int length;
	int Tid;
	int buf[4096];
	};
	
	FilterPids FH[MAXDEVICEFilter];
	bool ClosePipe(int Pid, int tid);

public:
	int CreatePipe(int Pid, int tid);
	cFilterHandle();
	~cFilterHandle();
	int Process(int Pid, char* buf);
};

#endif // VDR_FILTERHANDLE_H
