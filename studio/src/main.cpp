/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the MIT license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
//#include "Cosem.h"
#include "LuaWrapper.h"

#include <QApplication>
#include <QCommandLineParser>
#include <iostream>

#include "Settings.h"


int main(int argc, char *argv[])
{
    bool printHelp = false;
    bool enableGui = true;

    // Analyze command line arguments
    if (argc > 1)
    {
        enableGui = false;
    }

    LuaWrapper lua;

    int retCode = 0;
    if (enableGui)
    {
        QApplication app(argc, argv);
        MainWindow window(lua);

        // Load default settings for the GUI
        Settings settings;
        Settings::Context ctx;
        QList<Settings::Context> list;
        settings.Load(ctx);
        list.append(ctx);
        lua.Initialize(list);

        // Redirect lua print function so that it is printed on the console window
        lua.RedirectOutput(&window);

        // Show and execute
        window.resize(640, 512);
        window.show();
        retCode = app.exec();

        // Invalid the output redirection as the object is destroyed
        lua.DisableOutputRedirection();
    }
    else
    {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("Shaddam");
        QCoreApplication::setApplicationVersion("0.1.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("Shaddam, the scriptable Cosem test tool");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("inifile", QCoreApplication::translate("main", "Configuration input file."));

        const QCommandLineOption luaScriptOption("s", "script-file.lua", "script-file");
        parser.addOption(luaScriptOption);

        QStringList arguments;
        for (int i = 0; i < argc; i++)
        {
            arguments << QString(argv[i]);
        }

        parser.process(arguments);

        // FIXME: load specified INI file
        // Load default settings for the GUI
        Settings settings;
        Settings::Context ctx;
        QList<Settings::Context> list;
        settings.Load(ctx);
        list.append(ctx);

        lua.Initialize(list);


        if (parser.isSet(luaScriptOption))
        {
            const QString fileName = parser.value(luaScriptOption);

            if (QFile::exists(fileName))
            {
                // Immediately run the code from the external file
                LuaWrapper::RunScript(fileName.toStdString(), true);
            }
            else
            {
                printHelp = true;
            }
        }
        else
        {
            printHelp = true;
        }

        if (printHelp)
        {
            std::cout << parser.helpText().toStdString() << std::endl;
        }
    }

    // Clean the memory
    lua.Close();

    return retCode;
}
