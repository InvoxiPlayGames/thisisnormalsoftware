# Makefile

# the directories to compile, include and use for objects
SRC_DIR := src
INC_DIRS := include
BUILD_DIR := build
# the libraries to link the application with
LIBS :=
# the output filename
OUT_FILE := normalsoftware

# add the include directories to the C flags 
CFLAGS += $(patsubst %,-I%,$(INC_DIRS))
# add the libraries to the LD flags
LDFLAGS += $(patsubst %,-L%,$(LIB_DIRS)) $(patsubst %,-l%,$(LIBS))

# list of sources and object files required to compile
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(subst $(SRC_DIR),$(BUILD_DIR),$(patsubst %.c,%.c.o,$(SOURCES)))

# all phony entry
all: $(OUT_FILE)

# linking the objects together into an executable
$(OUT_FILE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# compiling all the source files into objects
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# clean up the temp build dir and the executable
clean:
	@echo Cleaning...
	@rm -rf $(BUILD_DIR)
	@rm $(OUT_FILE)

.PHONY: all clean
