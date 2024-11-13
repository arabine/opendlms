
#include <Windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include "Serial.h"
#include <TCHAR.H> 
#include <stdlib.h>

#include <winspool.h>
#include <setupapi.h>

void Serial::CheckNamePort(LPTSTR lpstrPortName)
{
	int nLen;
	if ((nLen = (int)_tcslen(lpstrPortName)) > 3)
	{
		if (_tcsnicmp(lpstrPortName, _T("COM"), 3) == 0)
		{
			int nPortNr = _ttoi(lpstrPortName + 3);
			if (nPortNr != 0)
			{
				lpstrPortName[nLen - 1] = '\0';
				//
				// This is a real registered serial port number,
				// All we now have to do is check if it really exists with the 
				// slow GetDefaultCommConfig
				COMMCONFIG cc;
				DWORD dwSize = sizeof(COMMCONFIG);
				if (GetDefaultCommConfig(lpstrPortName, &cc, &dwSize))
				{
					mList.push_back(nPortNr);
					std::cout << "Found COM" << nPortNr << std::endl;
				}
			}
		}
	}
}


bool Serial::ListComPorts()
{
	mList.clear();

	//
	//Call the first time to determine the size of the buffer to allocate
	//
	DWORD dwNeeded = 0;
	DWORD dwPorts = 0;
	EnumPorts(NULL, 1, NULL, 0, &dwNeeded, &dwPorts);

	//
	// Allocate the buffer and call the enumports function again
	//
	BYTE* pPorts = (BYTE*) new BYTE[dwNeeded];
	//
	// Use enumports for a fast search of all registered serial ports;
	// if parameter 2 is changed for 1 to 2 a more extensive description 
	// can be obtained by using the EnumPorts function
	//
	if (EnumPorts(NULL, 1, pPorts, dwNeeded, &dwNeeded, &dwPorts))
	{
		PORT_INFO_1 *pPortInfo = (PORT_INFO_1*)pPorts;

		for (DWORD dwCount = 0; dwCount < dwPorts; dwCount++, pPortInfo++)
		{
			//
			// Check if the name holds the COM part and look if 
			// it is really available and store port number in array
			//
			CheckNamePort(pPortInfo->pName);
		}
	}

	// Delete the allocated memory for the pPorts array
	delete[] pPorts;

	//
	// Sort the array
	//
	mList.sort();

	return (mList.size() != 0);
}



