/*
 * mminput.cpp: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: mminput.cpp,v 0.0.1 2007/02/08 Stefan Rieke 
 */

#include "mminput.hpp"
#include "device.hpp"
#include "setup.hpp"

#include "i18n.h"

const char *cPluginMMInput::DESCRIPTION = "MMInput Driver";

cPluginMMInput::cPluginMMInput(void) {
}

cPluginMMInput::~cPluginMMInput() {
}

const char *cPluginMMInput::Description(void) {
	return tr(DESCRIPTION);
}

bool cPluginMMInput::Start(void) {
//	i18n_name = Name();
  RegisterI18n(Phrases);

	cMMInputDevice::Init();

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

VDRPLUGINCREATOR(cPluginMMInput); // Don't touch this!
