OBJECT_PATH = objects
INCLUDES_PATH = includes/
BIN_PATH = .

SPOT_INCLUDES_PATH = /Users/sujansaha/usr/include
SPOT_LIB_PATH = /Users/sujansaha/usr/lib

OUTPUTS_PATH = outputs

# Object files
OBJS =          $(OBJECT_PATH)/CAPSL.o \
								$(OBJECT_PATH)/automaton.o \
								$(OBJECT_PATH)/config.o \
								$(OBJECT_PATH)/generateVHDL.o
								#$(OBJECT_PATH)/generateSystemC.o

# Include options
INCLUDEOPTS     = -I$(INCLUDES_PATH) \
									-I/local/gnu/include/g++ \
									-I$(SPOT_INCLUDES_PATH)

# Linker options
LINKEROPTS			= -L$(SPOT_LIB_PATH) -lspot


# ----------------------------------
# Compiler directives
# ----------------------------------
CPP = g++
CFLAGS = -g -Wall -std=c++11


# ----------------------------------
# Rules
# ----------------------------------
.FAILED:
	@echo "********************************************"
	@echo "       Errors encountered (!)(!)            "
	@echo "********************************************"

.DONE:
	@echo ""
	@echo "        Make finished"
	@echo ""

all: $(BIN_PATH)/capsl

$(BIN_PATH)/capsl: Makefile $(OBJS)
	$(CPP) $(CFLAGS) -o $(BIN_PATH)/capsl $(OBJS) $(LINKEROPTS) -lm

$(OBJECT_PATH)/CAPSL.o: sources/CAPSL.c
	$(CPP) $(CFLAGS) -o $(OBJECT_PATH)/CAPSL.o \
	$(INCLUDEOPTS) -c sources/CAPSL.c

$(OBJECT_PATH)/automaton.o: sources/automaton.c
	$(CPP) $(CFLAGS) -o $(OBJECT_PATH)/automaton.o \
	$(INCLUDEOPTS) -c sources/automaton.c

$(OBJECT_PATH)/config.o: sources/config.c
	$(CPP) $(CFLAGS) -o $(OBJECT_PATH)/config.o \
	$(INCLUDEOPTS) -c sources/config.c

$(OBJECT_PATH)/generateVHDL.o: sources/generateVHDL.c
	$(CPP) $(CFLAGS) -o $(OBJECT_PATH)/generateVHDL.o \
	$(INCLUDEOPTS) -c sources/generateVHDL.c

clean:
	-rm $(BIN_PATH)/capsl
	-rm $(OBJECT_PATH)/*
	-rm $(OUTPUTS_PATH)/sere/*
	-rm $(OUTPUTS_PATH)/vhdl/*
