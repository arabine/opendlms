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
LIB_METER				:= cosemlib/share/crypto cosemlib/share/util cosemlib/hdlc cosemlib/share/serial
LIB_COSEM				:= cosemlib/src
LIB_CAPTEL				:= src
LIB_CLIENT				:= cosemclient/lib cosemclient/src
LIB_ICL					:= icl

# *******************************************************************************
# CLIENT CONFIGURATION
# *******************************************************************************
APP_MODULES 	:= $(LIB_METER) $(LIB_COSEM) $(LIB_CAPTEL) $(LIB_CLIENT) $(LIB_ICL)
APP_LIBPATH 	:= 
APP_LIBS 		:= 

DEFINES += -DDEBUG=0
DEFINES += -DCOSEM_CLIENT_VER=\"1.2.0\"


# *******************************************************************************
# BUILD ENGINE
# *******************************************************************************
include build/Main.mk


all: $(OBJECTS)
	$(call linker, $(OBJECTS), $(APP_LIBS), captelcosem)
	
clean:
	@echo "Cleaning generated files..."
	$(VERBOSE) $(RM) -rf $(OUTDIR)/*.o $(OUTDIR)/*.d $(OUTDIR)/*.gcov $(OUTDIR)/*.gcov.htm

wipe:
	@echo "Wiping output directory..."
	$(VERBOSE) $(RM) -rf $(OUTDIR)


# *******************************************************************************
# END OF MAKEFILE
# *******************************************************************************
