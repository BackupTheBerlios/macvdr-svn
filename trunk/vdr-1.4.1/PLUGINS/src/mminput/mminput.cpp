/*
 * mminput.cpp: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: mminput.cpp,v 0.0.1 2007/02/08 Stefan Rieke 
 */
#include <sstream>

#include "mminput.hpp"
#include "device.hpp"
#include "setup.hpp"

#include "i18n.h"
#define MAXDVBDEVICES 2
#define MAXMMINPUTDEVICEINDEX 5

const char *cPluginMMInput::DESCRIPTION = "MMInput Driver";

cPluginMMInput::cPluginMMInput(void) {
        _TunerData = NULL;
        _TunerData = new std::vector< TunerRec* >;
}

cPluginMMInput::~cPluginMMInput() {
        if(_TunerData != 0x0){
                _TunerData->clear();
        }
        delete _TunerData;
        delete _TimerMMInput;
}

const char *cPluginMMInput::Description(void) {
        return tr(DESCRIPTION);
}

bool cPluginMMInput::Start(void) {
        //	i18n_name = Name();
        RegisterI18n(Phrases);
                
        _TimerMMInput = new cTimerMMInput(1, _TunerData);
        _TimerMMInput->Start();
        
        return true;
}

bool cPluginMMInput::Initialize(void){
        for (int i = 0; i < MAXDVBDEVICES; i++) {
                TunerRec* myTuner = new TunerRec;
                myTuner->pDevice = cMMInputDevice::Init();
                myTuner->pMM = 0x0;
                myTuner->activated = false;
                myTuner->activationErr = 0;
                myTuner->mmindex = -1;
                myTuner->deviceName = "dummyDevice";
                if(myTuner->pDevice != NULL){
                        _TunerData->push_back(myTuner);
                }
                else{
                        delete myTuner;
                }
        }
        return true;
}

void cPluginMMInput::Housekeeping(void) {
}

const char *cPluginMMInput::MainMenuEntry(void) {
        return NULL;
}

cOsdObject *cPluginMMInput::MainMenuAction(void) {
        return NULL;
}

cMenuSetupPage *cPluginMMInput::SetupMenu(void) {
        return new cMMInputMenuSetupPage;
}

bool cPluginMMInput::SetupParse(const char *Name, const char *Value) {
        return MMInputSetup.SetupParse(Name, Value);
}

cTimerMMInput::cTimerMMInput(int sleepTime, std::vector< TunerRec* >* TunerData):cThread("TimerMMInput"){
        _sleepTime = sleepTime;
        _TunerData = TunerData;
        if(_sleepTime < 1) _sleepTime = 1; // make sure, that the sleep has an minimum of 1 second.
        
}

cTimerMMInput::~cTimerMMInput(){
}

/*if at least on device is found, the trhead will be destroyed */
void cTimerMMInput::Action(void){
        
        bool found = false;
        MMInputDevice * pMM;
        
        while(Running()){
                
                if ( found == true ) Cancel(1);
                
                for(int i = 0; i < MAXMMINPUTDEVICEINDEX; i++){
                        bool full = true; // see is there enough space to handle more devices
                        pMM = 0x0;
                        pMM = MMInputDevice::withIndex( i );
                        
                        printf("cTimerMMInput test index %d\n",i);
                        if(pMM == 0x0) break; // stop if no devices is present
                        
                        for(int j = 0; j < (int)_TunerData->size(); j++){

                                printf("cTimerMMInput check index %d\n",i);
                                TunerRec* tuner = (* _TunerData)[j];
                                if( tuner->mmindex == i || tuner->mmindex != -1) continue;
                                full = false;
                                
                                printf("cTimerMMInput try to fetch device with index %d\n",i);
                        
                                printf("set device at position %d\n",j);
                                
                                tuner->mmindex = i;
                                tuner->pMM = pMM;
                                
                                stringstream mynum;
                                mynum.str("");
                                mynum << i;
                                
                                tuner->deviceName = "MMInputDevice " + mynum.str();
                                printf("cTimerMMInput new device name: %s\n",tuner->deviceName.c_str());
                                
                                tuner->pDevice->SetMMInputDevice(pMM, i);
                                tuner->activated = true;
                                found = true;
                                break;
                        }                        
                        if(full == true) break; // because we could not handle addition devices
                }
                
                printf("TimerMMInput running with sleep time %d seconds. number of device(s) %d\n",
                       _sleepTime,(int)_TunerData->size());
                cCondWait::SleepMs(_sleepTime*1000);
        }
}

VDRPLUGINCREATOR(cPluginMMInput); // Don't touch this!
