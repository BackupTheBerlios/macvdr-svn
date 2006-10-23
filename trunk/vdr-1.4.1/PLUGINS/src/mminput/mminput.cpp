/*
 * streamdev.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: streamdev-client.c,v 1.2 2005/04/24 16:19:44 lordjaxom Exp $
 */

#include "mminput.hpp"
#include "device.hpp"
#include "setup.hpp"

//#include "client/menu.h"
#include "i18n.h"

const char *cPluginMMInput::DESCRIPTION = "VTP MMInput";

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
//	if (MMInputSetup.StartClient && MMInputSetup.SyncEPG)
//		ClientSocket.SynchronizeEPG();
}

const char *cPluginMMInput::MainMenuEntry(void) {
	return NULL;
	//return MMInputSetup.StartClient ? tr("Streaming Control") : NULL;
}

cOsdObject *cPluginMMInput::MainMenuAction(void) {
	return NULL;
	//return MMInputSetup.StartClient ? new cMMInputMenu : NULL;
}

cMenuSetupPage *cPluginMMInput::SetupMenu(void) {
  return new cMMInputMenuSetupPage;
//	return NULL;
}

bool cPluginMMInput::SetupParse(const char *Name, const char *Value) {
  return MMInputSetup.SetupParse(Name, Value);
}

VDRPLUGINCREATOR(cPluginMMInput); // Don't touch this!
