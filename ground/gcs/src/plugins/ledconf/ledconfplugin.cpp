#include "ledconfplugin.h"
#include <QtPlugin>
#include <QStringList>

LedConfPlugin::LedConfPlugin(){
	// Do nothing
}

LedConfPlugin::~LedConfPlugin(){
	// Do nothing  
}

bool LedConfPlugin::initialize(const QStringList& args, QString *errMsg) {
	Q_UNUSED(args);
	Q_UNUSED(errMsg);

	return true;
}
void LedConfPlugin::extensionsInitialized() {
	// Do nothing
}

void LedConfPlugin::shutdown(){
	// Do nothing  
}

Q_EXPORT_PLUGIN(LedConfPlugin)
