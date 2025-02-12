#include "Settings.h"
#include "Util.h"
#include "IniFile.h"

#include <iostream>

Settings::Settings()
{
#ifdef USE_WINDOWS_OS
	mAppDataPath = Util::AppDataPath() + "\\shaddam";
#else
	mAppDataPath = Util::HomePath() + "/.shaddam";
#endif
}

void Settings::Load(Settings::Context &o_context)
{
	std::string confFileName = mAppDataPath + Util::DIR_SEPARATOR + "shaddam.ini";
	IniFile iniFile(confFileName);

	// Always ensure path exists
	Util::Mkdir(mAppDataPath);

    o_context.id = 0;

	if (Util::FileExists(confFileName))
    {
		iniFile.Load();
		/*
			[default]
			port=COM17
			baudrate=9600
		*/

		o_context.port = iniFile.Get("default", "port", "COM5");
		o_context.baudrate = iniFile.GetInteger("default", "baudrate", 57600);

		std::cout << "Read configuration file: port=" << o_context.port << ", baudrate=" << o_context.baudrate << std::endl;
    }
}

