CC = g++
C11 = -std=c++11
OPT = -O3
OPT_DEBUG = -g
WARN = -Wall
CFLAGS = $(C11) $(OPT) $(WARN) $(INC) $(LIB)


# List all your .cc/.cpp files here (source files, excluding header files)
SIM_SRC = sim.cc

# List corresponding compiled object files here (.o files)
SIM_OBJ = sim.o
 
#################################

# default rule

all: sim
	@echo "my work is done here..."

debug: CFLAGS = $(C11) $(OPT_DEBUG) $(WARN) $(INC) $(LIB)
debug: sim
	@echo "In debug $(CFLAGS)"

# rule for making sim_cache

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH sim_cache-----------"


# generic rule for converting any .cpp file to any .o file
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc

.cpp.o:
	$(CC) $(CFLAGS)  -c $*.cpp


# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o sim


# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o


