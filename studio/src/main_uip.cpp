
// IUP libraries
#include <iup.h>
#include <iupcontrols.h>

// C/C++ libraries
#include <stdlib.h>
#include <sstream>
#include <iostream>

// Project includes
#include "Cosem.h"
#include "Serial.h"
#include "LuaWrapper.h"
#include "Efs.h"

extern "C" void led_load(void);

static Cosem cosem;
static CosemManager gCosemManager;
static Serial serial;
static LuaWrapper lua;

// Global access
ICosem &gCosem = gCosemManager;

int button_exit_clicked(Ihandle *self)
{
//	IupMessage("Hello World Message", "Hello world from IUP.");
	/* Exits the main loop */
	return IUP_CLOSE;
}

int button_start_clicked(Ihandle *self)
{
	// Execute Lua script
	return IUP_CONTINUE;
}


void LoadLuaFiles(const std::list<std::string> &flist)
{
	Efs file;

	for (std::list<std::string>::const_iterator iter = flist.begin(); iter != flist.end(); ++iter)
	{
		if (GetFile(*iter, file))
		{
			std::string script;
			script.assign((const char*)file.ptr, file.size);
			lua.Execute(script);
		}
		else
		{
			std::cout << "Cannot load embedded file: " << *iter << std::endl;
		}
	}
}


int main(int argc, char *argv[])
{
	bool printHelp = false;
	bool enableGui = true;

	// Analyze command line arguments
	if (argc > 1)
	{
		enableGui = false;
	}

	// initialize iup
	IupOpen(&argc, &argv);
	IupControlsOpen();

	// Create widgets
	led_load();
	
	// Initialize utilities
	serial.ListComPorts();

	Settings settings;
	Settings::Context ctx;
	settings.Load(ctx);

	// Create one Cosem and Lua instance
	int id = gCosemManager.AddChannel(ctx.port, ctx.baudrate);
	lua.Initialize(id);

	Efs file;

	// Start main script file
	if (GetFile("image_transfer.lua", file))
	{
		std::string script;
		script.assign((const char*)file.ptr, file.size);
		lua.Execute(script);
	}
	
	/* Registers callbacks */
	IupSetCallback(IupGetHandle("button_exit"), "ACTION", (Icallback)button_exit_clicked);
	IupSetCallback(IupGetHandle("button_start"), "ACTION", (Icallback)button_start_clicked);

	// Show main dialog
	Ihandle *dlg = IupGetHandle("dlg");
	IupShow(dlg);

	/* Initialize widgets */
	std::list<std::uint8_t> ports = serial.GetList();
	Ihandle *commPort = IupGetHandle("comm_port");
	for (std::list<std::uint8_t>::iterator iter = ports.begin(); iter != ports.end(); ++iter)
	{
		std::stringstream ss;
		ss << "COM" << (int)*iter << "\n";
		IupSetAttribute(commPort, "APPENDITEM", ss.str().c_str());
	}

	// Wait for user interaction
	IupMainLoop();

	// Clean up
	IupDestroy(dlg);
	IupClose();

	lua.Stop();

	return EXIT_SUCCESS;
}
