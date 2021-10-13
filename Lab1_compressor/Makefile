#------------------------------------------------------------
#	Variables
#------------------------------------------------------------
# Target platform compiler
ifeq ($(target),win32)
CXX=x86_64-w64-mingw32-g++
CXXFLAGS=-Wall -Werror -std=c++17 -static-libstdc++ -static-libgcc
BINARY_EXT=exe
else
CXX=g++
CXXFLAGS=-Wall -Werror -std=c++17
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
INCLUDE=$(addprefix -I$(SRC_DIR),$(addsuffix /include,$(BINARIES)))

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
	$(CXX) $(CXXFLAGS) $(addprefix $(OBJ_DIR),$(filter $@%,$(OBJECTS))) -o $(BIN_DIR)$@.$(BINARY_EXT)

clean:
	$(RM) $(BIN_DIR)*
	$(RM) $(OBJ_DIR)*
