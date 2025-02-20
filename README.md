# DLMS/Cosem stack

This is an implementation of the DLMS/Cosem protocol in the MIT Open Source and permissive license. This protocol is mainly used in gas/water/electricity meters but is enough generic to target any IoT device.

See the official organization group to learn more: http://www.dlms.com.

This repository provides C code, mainly framing encoding/decoding functions. There is no any integration in a complete stand-alone examples. See other repositories for that purpose.

## Development goals

This Cosem stack has the following goals :

  * Pure portable and stand-alone ANSI C99 code
  * Fully unit tested with pre-defined vectors
  * Client/server implementation, LogicalName referencing, LLS, HLS3, 4 and 5, security policy 1
  * Examples using Cosem over TCP/IP
  * Memory efficient / no dynamic allocation (static, configurable at build-time) / no buffer copy
  * Full traces
  * Memory protected against buffer overflow using array utility

## What is working so far

  
  * BER coder/decoder
  * Association coders and decoders AARQ/AARE/RLRQ/RLRE (LLS)
  * Secure HLS5 GMAC Authentication
  * Get Request normal/by block
  * Set request normal
  * Action service
  * Exception response in case of problem
  * HDLC framing utility
  * Serial port HAL (Win32/Linux)
  * Utilities
  * Client reader over HDLC with full logging and XML output
  * Server example as a meter simulator
  * Scripting GUI tool prototype

## Directory organization

- `cosemlib`: kernel files of the DLMS/Cosem protocol
- `client`: specific utilities to write a DLMS/Cosem client
- `server`: specific utilities to write a DLMS/Cosem server
- `examples`: Client/server examples
- `studio`: Graphical utility with Lua scripting (WIP)
- `tests`: Unit tests for the protocol with raw frames and application objects

# How to build

You must have CMake and a C/C++ compiler. Examples generate stand alone executables, other repositories generate static libraries.

The top level cmake file will build everything:

```
mkdir build
cd build
cmake ..
make
```

# How to edit & code

The best way is to use VSCode, there are pre-configured settings in the .vscode directory to build & execute demos.

# Manual and integration hints

FIXME: before writing this section, wait for stabilization of the HAL/Cosem API and utilities


# Development schedule

## Version 1.0 TODO

  * LN with ciphering Security Policy 0 (Authenticated & encrypted)
  * HLS 3, 4, 5, 6

## Version X.0

  * Multiple logical devices support
  * ACCESS service
  * GBT service
  * ECDSA
  * ECDSA data transport cyphering
