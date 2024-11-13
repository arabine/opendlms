

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "Cosem.h"
#include <Windows.h>
#include "ApiCosemServices.h"

Cosem::Cosem()
	: mId(-1)
	, mConnected(false)
{

}

Cosem::~Cosem()
{
    
}

void Cosem::Initialize(int id, const std::string &port, const int baudrate)
{
	mId = id;
	mPort = port;
	mBaudrate = baudrate;

    char version[30];
	Api_GetCosemVersion(version);
    std::cout << "Cosem version: " << version << std::endl;

	GetInitApiCosem(&mContext);

    mContext.cosem_init.Control_Param = ADDRESS_32BITS;
	ReturnCode(OPEN_API_COSEM, OpenApiCosem(&mContext.cosem_init));
}

void Cosem::Connect(int id)
{
	if (id != mId)
	{
		std::cout << "Cosem error: bad id!" << std::endl;
		return;
	}

	Disconnect(id);

	mPhyParam.Baudrate = mBaudrate;
	std::memset(mPhyParam.com_port, 0, MAX_LEN_STRING);
//	std::memcpy(mPhyParam.com_port, mPort.c_str(), mPort.size());
	//mPhyParam.physical_connection_type = DIRECT_CONNECTION; //IEC1107_MODE_E;
	mPhyParam.physical_connection_type = TCPIP;

	//   mPhyParam.iec1107_param.forceNegotiationSpeed = NS_REPLACE_300BAUDS;
	//   mPhyParam.iec1107_param.opticalProbeType = OPTICAL_PROBE_GENERIC;

	mComPortParam.data = 8;
	mComPortParam.stop = 1;
	mComPortParam.parity = 0;

	mPhyParam.tcpip_param.nServerPort = 4059;
	std::memset(mPhyParam.tcpip_param.szSourceAddress, 0, MAX_LEN_STRING); // Need to zero array for TCP also
	std::string tcpAddr = "127.0.0.1";
//	std::memcpy(mPhyParam.tcpip_param.szSourceAddress, tcpAddr.c_str(), tcpAddr.size());
	std::memcpy(mPhyParam.com_port, tcpAddr.c_str(), tcpAddr.size());

	//ReturnCode(OPEN_COM_PORT_PARAMETERS, OpenComPortParameters(&mPhyParam, &mComPortParam, &mPortId));

	ReturnCode(OPEN_COM_PORT, OpenComPort(&mPhyParam, &mPortId));

	mCosemContext.Authentification_service = AUTH_NONE;
	mCosemContext.IdClientProfil = 1;
	mCosemContext.SAPLogicalDev = 1;
	mCosemContext.PhysicalAddress = 16;
	mCosemContext.lls_auth.Password.nb_oct = 8;
	mCosemContext.UserId = NULL;
	//mCosemContext.lls_auth.Password.str_oct // use memcpy to set the password
	ReturnCode(CREATE_COSEM_CONTEXT, CreateCosemContext(&mCosemContext, &mCosemContextId));

	ReturnCode(CONNECT_COSEM, ConnectCosem(mCosemContextId, mPortId, &mConnectionId));
}

void Cosem::GetValue(int id, DATAREQID& i_dataReq, COS_DATA*& o_getData, COS_DATA_ACCESS_ERROR& o_dataAccessError)
{
	if (id != mId)
	{
		std::cout << "Cosem error: bad id!" << std::endl;
		return;
	}

	ReturnCode(GET_COSEM, GetCosem(mCosemContextId, NULL, &i_dataReq, &o_getData, &o_dataAccessError));
}

void Cosem::Disconnect(int id)
{
	if (id != mId)
	{
		std::cout << "Cosem error: bad id!" << std::endl;
		return;
	}

	if (mConnected)
	{
		ReturnCode(RELEASE_CONNECT, ReleaseConnect(mCosemContextId));
	}
}

void Cosem::ReturnCode(Command cmd, HResult res)
{
	std::stringstream ss;
    ss << "Operation ";

	switch (cmd)
	{
	case OPEN_API_COSEM:
		ss << "OpenApiCosem()";
		break;
	case OPEN_COM_PORT_PARAMETERS:
		ss << "OpenComPortParameters()";
		break;
	case CREATE_COSEM_CONTEXT:
		ss << "CreateCosemContext()";
		break;
	case CONNECT_COSEM:
		ss << "ConnectCosem()";
		if (res == COS_OK)
		{
			mConnected = true;
		}
		break;
	case OPEN_COM_PORT:
		ss << "OpenComPort()";
		break;
	case GET_COSEM:
		ss << "GetCosem()";
		break;
	case RELEASE_CONNECT:
		ss << "ReleaseConnect()";
		if (res == COS_OK)
		{
			mConnected = false;
		}
		break;
		
	default:
		break;
	}

    if (res == COS_OK)
    {
        ss << " success.";
    }
    else
    {
        ss << " failure! Code: " << (int)res;
    }

	std::cout << ss.str() << std::endl;
}


