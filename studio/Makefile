# *******************************************************************************
# Main makefile project
# This makefile calls all the modules defined in the config.mk file
# *******************************************************************************

# *******************************************************************************
# DEFAULT DEFINITIONS
# These definitions can be overloaded from the command line
# *******************************************************************************
PROJECT		?= example
TARGET 		?= release
BOARD 		?= host
OPTIM 		?= high
ENABLE_DEP 	?= true
ARCH		?= host-freertos-gcc



TOPDIR		:= ./
OUTDIR		:= $(TOPDIR)build/output/


# Export them to be sure that they are available in sub-makefiles
export PROJECT
export TARGET
export BOARD
export DEFINES
export OPTIM
export ENABLE_DEP
export ARCH
export TOPDIR
export OUTDIR

# *******************************************************************************
# APPLICATION DEFINITIONS
# List of modules and extra libraries needed to generate project targets
# *******************************************************************************
LIB_METER				:= cosemserver/system cosemserver/database cosemserver/application cosemserver/arch/host
LIB_HDLC				:= cosemlib/hdlc
LIB_SHARE				:= cosemlib/share/crypto cosemlib/share/util cosemlib/share/serial cosemlib/share/ip
LIB_COSEM				:= cosemlib/src
LIB_CLIENT				:= cosemclient/lib
LIB_ICL					:= icl
LIB_TESTS				:= tests

# *******************************************************************************
# CLIENT CONFIGURATION
# *******************************************************************************
ifeq ($(MAKECMDGOALS), client)

DEFINES += -DDEBUG=0

APP_MODULES 	:= $(LIB_SHARE) $(LIB_HDLC) $(LIB_COSEM) $(LIB_CAPTEL) $(LIB_CLIENT) $(LIB_ICL) cosemclient/src
APP_LIBPATH 	:= 
APP_LIBS 		:= 

endif

# *******************************************************************************
# TESTS CONFIGURATION
# *******************************************************************************
ifeq ($(MAKECMDGOALS), tests)

DEFINES += -DDEBUG=1

APP_MODULES 	:= $(LIB_COSEM) $(LIB_ICL) $(LIB_TESTS) $(LIB_CLIENT) $(LIB_SHARE) $(LIB_HDLC) 
APP_LIBPATH 	:= 
APP_LIBS 		:= 

endif


# *******************************************************************************
# SERVER CONFIGURATION
# *******************************************************************************
ifeq ($(MAKECMDGOALS), server)

APP_MODULES 	:= $(LIB_COSEM) $(LIB_SHARE) $(LIB_HDLC) $(LIB_METER) cosemserver/src
APP_LIBPATH 	:= 
APP_LIBS 		:= 

endif

# *******************************************************************************
# HDLC DECODER CONFIGURATION
# *******************************************************************************
ifeq ($(MAKECMDGOALS), hdlc_decoder)

DEFINES += -DDEBUG=0

APP_MODULES 	:= $(LIB_HDLC)  tools/hdlc_decoder
APP_LIBPATH 	:= 
APP_LIBS 		:= 

endif

# *******************************************************************************
# BUILD ENGINE
# *******************************************************************************
include build/Main.mk


tests: $(OBJECTS)
	$(call linker, $(OBJECTS), $(APP_LIBS), cosem_tests)
	
server: $(OBJECTS)
	$(call linker, $(OBJECTS), $(APP_LIBS), cosem_server)

client: $(OBJECTS)
	$(call linker, $(OBJECTS), $(APP_LIBS), cosem_client)

hdlc_decoder: $(OBJECTS)
	$(call linker, $(OBJECTS), $(APP_LIBS), hdlc_decoder)
	
clean:
	@echo "Cleaning generated files..."
	$(VERBOSE) $(RM) -rf $(OUTDIR)/*.o $(OUTDIR)/*.d $(OUTDIR)/*.gcov $(OUTDIR)/*.gcov.htm

wipe:
	@echo "Wiping output directory..."
	$(VERBOSE) $(RM) -rf $(OUTDIR)


# *******************************************************************************
# END OF MAKEFILE
# *******************************************************************************
