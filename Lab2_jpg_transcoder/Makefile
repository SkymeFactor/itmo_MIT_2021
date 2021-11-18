#------------------------------------------------------------
#	Variables
#------------------------------------------------------------
# Target platform compiler
ifeq ($(target),win32)
CXX=x86_64-w64-mingw32-g++
CXXFLAGS=-Wall -Werror -std=c++17 -static-libstdc++ -static-libgcc
LD_FLAGS=
BINARY_EXT=exe
else
CXX=g++
CXXFLAGS=-g -Wall -std=c++17 -pthread
LD_FLAGS=
BINARY_EXT=elf
endif
# Cleaner
RM=rm -rf

# Directories
SRC_DIR=src/
OBJ_DIR=obj/
BIN_DIR=build/

# Files
SOURCES=$(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJECTS=$(SOURCES:$(SRC_DIR)%.cpp=%.o)
INCLUDE=-Iinclude/

# Executables
BINARIES=encoder decoder

#------------------------------------------------------------
#	Targets
#------------------------------------------------------------

.PHONY: all clean

all:  $(OBJECTS) $(BINARIES)

	
$(OBJECTS): %.o: $(SRC_DIR)%.cpp
	@if [ ! -d "$(OBJ_DIR)$(@D)" ]; then mkdir -p $(OBJ_DIR)$(@D); fi
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $(OBJ_DIR)$@ -MMD
	
$(BINARIES): $(OBJECTS)
# We need to filter out unnecessarry object files
# and add the $(OBJ_DIR) prefix to each filename
	$(CXX) $(CXXFLAGS) $(addprefix $(OBJ_DIR),$(filter $@%,$(OBJECTS))) obj/ppmd.o -o $(BIN_DIR)$@.$(BINARY_EXT) $(LD_FLAGS)

clean:
	$(RM) $(BIN_DIR)*
	$(RM) $(OBJ_DIR)*
