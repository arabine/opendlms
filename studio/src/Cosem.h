#ifndef COSEM_H
#define COSEM_H

#include <string>
#include <map>


class ICosem
{
public:
    virtual ~ICosem() {}
	virtual void Connect(int id) = 0;
	virtual void Disconnect(int id) = 0;
    //virtual void GetValue(int id, DATAREQID& i_dataReq, COS_DATA*& o_getData, COS_DATA_ACCESS_ERROR& o_dataAccessError) = 0;
};

class Cosem : public ICosem 
{
public:
	enum Command {
		OPEN_API_COSEM,
		OPEN_COM_PORT_PARAMETERS,
		OPEN_COM_PORT,
		CREATE_COSEM_CONTEXT,
		CONNECT_COSEM,
		GET_COSEM,
		RELEASE_CONNECT
	};


    Cosem();
    ~Cosem();

	// From ICosem
	virtual void Connect(int id);
	virtual void Disconnect(int id);
    // virtual void GetValue(int id, DATAREQID& i_dataReq, COS_DATA*& o_getData, COS_DATA_ACCESS_ERROR& o_dataAccessError);

	void Initialize(int id, const std::string &port, const int baudrate);

private:
    // void ReturnCode(Command cmd, HResult res);

	std::string mPort;
	int mBaudrate;
	int mId;
	bool mConnected;

    // // Cosem library stuff
 //    COSEM_APICONTEXT mContext;
 //    PHYSICAL_PARAM mPhyParam;
 //    PARAM_COMPORT mComPortParam;
 //    COS_CONTEXT mCosemContext;

    unsigned short mPortId;
    unsigned short mCosemContextId;
    unsigned short mConnectionId;
};

/**
 * Proxy class that manage N cosem instances
 */
class CosemManager : public ICosem
{
public:
	CosemManager()
	{
		mIds = 0;
	}

	virtual void Connect(int id)
	{
		if (mCosem.find(id) != mCosem.end())
		{
			mCosem[id]->Connect(id);
		}
	}

	virtual void Disconnect(int id)
	{
		if (mCosem.find(id) != mCosem.end())
		{
			mCosem[id]->Disconnect(id);
		}
	}

    // virtual void GetValue(int id, DATAREQID& i_dataReq, COS_DATA*& o_getData, COS_DATA_ACCESS_ERROR& o_dataAccessError)
    // {
    // 	if (mCosem.find(id) != mCosem.end())
    // 	{
    // 		mCosem[id]->GetValue(id, i_dataReq, o_getData, o_dataAccessError);
    // 	}
    // }

	~CosemManager()
	{
		// Free cosem instances
		//foreach (Cosem *cosem, mCosem)
		for (std::map<int, Cosem *>::const_iterator iter = mCosem.begin(); iter != mCosem.end(); ++iter)
		{
			delete iter->second;
		}
		mCosem.clear();
	}

	int AddChannel(const std::string &port, const int baudrate)
	{
		int id = mIds;
		Cosem *cosem = new Cosem();
		cosem->Initialize(id, port, baudrate);
		mCosem[id] = cosem;
		
		mIds++;
		return id;
	}

private:
	std::map<int, Cosem*> mCosem;
	int mIds;
};


#endif // COSEM_H
