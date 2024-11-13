

#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <sstream>

#include "Util.h"
#include "LuaWrapper.h"
#include "definitions.h"
#include "Efs.h"

IScript::IListener *LuaWrapper::mListener = NULL;

extern ICosem &gCosem;

LuaWrapper::LuaWrapper()
	: mId(-1)
	, mStop(false)
	, mState(NULL)
	, mStarted(false)
{
	
}

LuaWrapper::~LuaWrapper()
{
	Stop();
}

int LuaWrapper::DelayMs(lua_State *L)
{
	long msecs = static_cast<long>(lua_tointeger(L, -1));
    Sleep(msecs);
    return 0;                  /* No items returned */
}

int LuaWrapper::Connect(lua_State *L)
{
    int nArgs = lua_gettop(L);
    unsigned int id = 0;

    if (nArgs == 1)
    {
        id = static_cast<unsigned int>(lua_tointeger(L, -1));
    }

	gCosem.Connect(id);
    return 0;                  /* No items returned */
}


int LuaWrapper::Disconnect(lua_State *L)
{
	int nArgs = lua_gettop(L);
	unsigned int id = 0;

	if (nArgs == 1)
	{
		id = static_cast<unsigned int>(lua_tointeger(L, -1));
	}

	gCosem.Disconnect(id);
	return 0;                  /* No items returned */
}

int LuaWrapper::GetCosem(lua_State *L)
{
	unsigned int id = 0;

    COS_DATA *getData = NULL;
	COS_DATA_ACCESS_ERROR dataAccessError = COS_OTHER_REASON;
    DATAREQID dataReq;

    int nArgs = lua_gettop(L);
	
    if (nArgs == 2)
    {
        size_t size;

		id = static_cast<unsigned int>(lua_tointeger(L, 1));
		std::string cosemIdentity = lua_tolstring(L, 2, &size);

        std::vector<std::string> list = Util::Split(cosemIdentity, "-");

		if (list.size() == 3)
		{
			std::string classID = list[0];
			std::string obis = list[1];
			std::string attribute = list[2];

			dataReq.COSEM_Interface_Class_Id = static_cast<unsigned short>(Util::ToLong(classID));
			dataReq.Attribute_Id = static_cast<unsigned char>(Util::ToLong(attribute));
			list.clear();
			list = Util::Split(obis, " ");
			dataReq.COSEM_Interface_Object_Instance_Id.nb_oct = list.size();

			for (std::uint32_t i = 0U; i < list.size(); i++)
			{
				dataReq.COSEM_Interface_Object_Instance_Id.str_oct[i] = static_cast<unsigned char>(Util::ToLong(list[i]));
			}
		}
    }

    gCosem.GetValue(id, dataReq, getData, dataAccessError);
	
	return SerializeCosemDataAndResult(L, getData, dataAccessError);
}

int LuaWrapper::SetCosem(lua_State *L)
{
	unsigned int id = 0;

	COS_DATA *getData = NULL;
	COS_DATA_ACCESS_ERROR dataAccessError = COS_OTHER_REASON;
	DATAREQID dataReq;

	int nArgs = lua_gettop(L);

	if (nArgs == 2)
	{
		size_t size;

		id = static_cast<unsigned int>(lua_tointeger(L, 1));
		std::string cosemIdentity = lua_tolstring(L, 2, &size);

		std::vector<std::string> list = Util::Split(cosemIdentity, "-");

		if (list.size() == 3)
		{
			std::string classID = list[0];
			std::string obis = list[1];
			std::string attribute = list[2];

			dataReq.COSEM_Interface_Class_Id = static_cast<unsigned short>(Util::ToLong(classID));
			dataReq.Attribute_Id = static_cast<unsigned char>(Util::ToLong(attribute));
			list.clear();
			list = Util::Split(obis, " ");
			dataReq.COSEM_Interface_Object_Instance_Id.nb_oct = list.size();

			for (std::uint32_t i = 0U; i < list.size(); i++)
			{
				dataReq.COSEM_Interface_Object_Instance_Id.str_oct[i] = static_cast<unsigned char>(Util::ToLong(list[i]));
			}
		}
	}

	gCosem.GetValue(id, dataReq, getData, dataAccessError);

	return SerializeCosemDataAndResult(L, getData, dataAccessError);
}

int LuaWrapper::SerializeCosemDataAndResult(lua_State *L, COS_DATA* cosemData, COS_DATA_ACCESS_ERROR DataAccessError)
{
    std::string result;

	// FIXME: iterate through the pointer to serialize all the data
    result = SerializeCosemData(cosemData);

    lua_pushnumber(L, DataAccessError);
    lua_pushlstring(L, result.c_str(), result.size());

    return 2; // result + 1 string in the stack
}


std::string LuaWrapper::SerializeCosemData(COS_DATA* cosemData)
{
	std::stringstream type;
	std::stringstream value;

	if (cosemData != NULL)
	{
		switch (cosemData->type)
		{
		case DATA_UNSIGNED:
			type << "UNSIGNED8";
			value << (int)cosemData->data._unsigned8;
			break;

		case DATA_BOOLEAN:
		{
			type << "BOOLEAN";
			int bVal = 0;
			if (cosemData->data._booleen)
			{
				bVal = 1;
			}
			value << bVal;
			break;
		}

		case DATA_OCTETSTRING:
		{
			type << "OCTETSTRING";
			value << "\"";
			for (int i = 0; i < cosemData->data._octetstring.nb_oct; i++)
			{
				value << (int)cosemData->data._octetstring.str_oct[i] << ";";
			}
			value << "\"";
		}
		break;

		default:
			std::cout << "Cosem type: " << (int)cosemData->type << " not supported" << std::endl;
			break;
		}
	}

	return "{ cosem_type=\"" + type.str() + "\", data=" + value.str() + "}";
}

int LuaWrapper::LuaPrint(lua_State *L)
{
    int nArgs = lua_gettop(L);
    int i;
    lua_getglobal(L, "tostring");
    std::string ret;//this is where we will dump the output
    //make sure you start at 1 *NOT* 0
    for(i = 1; i <= nArgs; i++)
    {
        const char *s;
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        s = lua_tostring(L, -1);
        if (s == NULL)
        {
            return luaL_error(L, LUA_QL("tostring") " must return a string to ", LUA_QL("print"));
        }

        ret.append(s);
        lua_pop(L, 1);
    }
    //Send it wherever
    if (mListener != NULL)
    {
        mListener->Print(ret);
    }

    return 0;
}

void LuaWrapper::EntryPoint(LuaWrapper *pthis)
{
    for(;;)
    {
        std::string script;
		pthis->WaitForItem(script);
		if (pthis->StopRequested())
        {
            break;
        }
        else
        {
            if (script.size() > 0)
            {
                RunScript(pthis->GetLuaState(), script, false);
            }
        }
    }
}

/**
 * Load a module from internal memory (called by Lua 'require'
 */
int LuaWrapper::ModuleLoader(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);  // Module name
	Efs file;
	int ret = 0;

	if (GetFile(std::string(name) + ".lua", file))
	{
		luaL_loadbuffer(L, (const char*)file.ptr, file.size, name);
		ret = 1;
	}

	return ret;
}


int LuaWrapper::Initialize(int id)
{
	mId = id;
	mStarted = true;
	mStop = false;
    
    // on crée un contexte d'exécution de Lua
    mState = luaL_newstate();
	if (mState == NULL)
    {
        std::cout <<  "cannot create state: not enough memory" << std::endl;
        return EXIT_FAILURE;
    }

    // Load lua standard library
	luaL_openlibs(mState);

    // Constants
	lua_pushstring(mState, SHADDAM_VERSION);
	lua_setglobal(mState, "_SHADDAM_VERSION");

	// Custom module loader
	lua_register(mState, "shaddam_loader", ModuleLoader);
	luaL_dostring(mState, "table.insert(package.searchers, shaddam_loader) \n");

    // Additional functions exported to Lua world
	lua_register(mState, "delay_ms", DelayMs);
	lua_register(mState, "connect", Connect);
	lua_register(mState, "getCosem", GetCosem);
	lua_register(mState, "disconnect", Disconnect);

	// Finally create and start the thread
	mThread = std::thread(LuaWrapper::EntryPoint, this);

    return 0;
}

void LuaWrapper::Stop()
{
	if (mStarted)
	{
		// Send a dummy message to exit the thread
		mStop = true;
		mQueue.Push("");
		mThread.join();

		// Shutdown lua
		lua_close(mState);
		mStarted = false;
	}
}

void LuaWrapper::RedirectOutput(IScript::IListener *printer)
{
    mListener = printer;
    // Print overload
	lua_register(mState, "print", LuaWrapper::LuaPrint);
}

void LuaWrapper::DisableOutputRedirection()
{
    mListener = NULL;
}

void LuaWrapper::RunScript(lua_State *L, const std::string &script, bool isFile)
{
    int retCode = -1;

    if (isFile)
    {
		retCode = luaL_dofile(L, script.c_str());
    }
    else
    {
		retCode = luaL_dostring(L, script.c_str());
    }


    if (retCode)
    {
		std::string message = "Lua Error: ";
		message += std::string(lua_tostring(L, -1));

        //Send it wherever
        if (mListener != NULL)
        {
            mListener->Print(message);
            mListener->Result(retCode);
        }
		else
		{
			std::cout << message << std::endl;
		}
    }
    else
    {
        std::cout << "Script execution OK." << std::endl;
    }
}

void LuaWrapper::Execute(const std::string &script)
{
    mQueue.Push(std::string(script));
}



// END OF FILE
