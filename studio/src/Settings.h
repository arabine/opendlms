#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{
public:
    struct Context
    {
        int id;
        std::string port; ///< Serial port
        int baudrate; ///< com speed

        Context()
        {
            id = 0;
            port = "COM4";
            baudrate = 115200;
        }
    };


    Settings();

    // Load default configuration file (GUI mode)
    void Load(Context &o_context);
private:
	std::string mAppDataPath;

};

#endif // SETTINGS_H
