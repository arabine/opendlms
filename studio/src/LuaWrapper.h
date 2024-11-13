#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include <list>
#include "Settings.h"
#include "i_script.h"
#include "ThreadQueue.h"
#include "Cosem.h"
#include "lua.hpp"

class LuaWrapper : public IScript
{

public:
	LuaWrapper();
	~LuaWrapper();

    // Static methods
    static int DelayMs(lua_State *L);
    static int Connect(lua_State *L);
    static int GetCosem(lua_State *L);
	static int SetCosem(lua_State *L);
    static int Disconnect(lua_State *L);
	static int ModuleLoader(lua_State *L);

    static int SerializeCosemDataAndResult(lua_State *L, COS_DATA* i_cosemData, COS_DATA_ACCESS_ERROR DataAccessError);
	static std::string SerializeCosemData(COS_DATA* i_cosemData);

    static int LuaPrint(lua_State *L);
	static void EntryPoint(LuaWrapper *pthis);
	static void RunScript(lua_State *L, const std::string &script, bool isFile);

	// Instance methods
    int Initialize(int id);
    void Stop();
    void RedirectOutput(IScript::IListener *printer);
	
	void WaitForItem(std::string &script)
	{
		mQueue.WaitAndPop(script);
	}
    void DisableOutputRedirection();

	lua_State *GetLuaState() { return mState;  }
	bool StopRequested() { return mStop; }
	
    // From IScript
    virtual void Execute(const std::string &script);

private:
    static IScript::IListener *mListener;
    
	int mId;
	ThreadQueue<std::string> mQueue;
    std::thread mThread;
    bool mStop;
    lua_State *mState;
	bool mStarted;
};

#endif // LUA_WRAPPER_H

