/*
 *  $Id: device.h,v 1.3 2005/02/08 15:21:19 lordjaxom Exp $
 */
 
#ifndef VDR_FILTERHANDLE_H
#define VDR_FILTERHANDLE_H

#define MAXDEVICEFILTER 32

#include <vdr/tools.h>

class cSectionFilter;

class cFilterHandle {

private:

	struct FilterPids{
		int Rhandle;
		int Whandle;
		int length;
		int Tid;
		cSectionFilter *sf;
	};
	
        int PidNum[MAXDEVICEFILTER];
	FilterPids FH[MAXDEVICEFILTER];
	int maxFilter;
	bool ClosePipe(int Pid, int tid);
	void sendTable();

public:
	int CreateFilter(int Pid, int tid);
	cFilterHandle();
	~cFilterHandle();
	int Process(uchar* buf);
};

#endif // VDR_FILTERHANDLE_H
