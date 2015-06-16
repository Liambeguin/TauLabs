#ifndef LEDCONF_H
#define LEDCONF_H

#include <extensionsystem/iplugin.h>

class LedConfPlugin : public ExtensionSystem::IPlugin
{
	public:
		LedConfPlugin();
		~LedConfPlugin();

		void extensionsInitialized();
		bool initialize(const QStringList & arguments, QString * errorString);
		void shutdown();
};

#endif // LEDCONF_H
