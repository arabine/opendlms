
REM FIXME: ZIP resources before embedding

REM Build files to detect any error before including it in the executable
..\..\tools\luac53.exe -s -o img.luac image_transfer.lua
..\..\tools\luac53.exe -s -o cm.luac ..\..\etc\util.lua
..\..\tools\luac53.exe -s -o cm.luac ..\..\etc\cosem.lua

REM Generate the file system
..\..\tools\lua53.exe ..\..\tools\bin2c.lua image_transfer.lua ..\..\etc\cosem.lua ..\..\etc\util.lua > Efs.cpp
