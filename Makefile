NAME =timerfd_handler
MAJOR_VERSION=1
MINOR_VERSION=0
OPTIMIZE = 	-O0
WARN = 		-Wall -Wextra -Wno-padded -pedantic
CC = gcc
AR = ar
DEBUG=-g
DEFINE_VERSION = -DHIGH_VERSION=$(MAJOR_VERSION) -DMIDDLE_VERSION=$(MINOR_VERSION)
DEFINE_TEST = -DTEST_IMU_FOG_KVH

src_root := src
CFLAGS += -fPIC ${WARN} ${OPTIMIZE} $(CF_EXTRA) $(DEBUG) -D_GNU_SOURCE $(DEFINE_VERSION)
INCLUDES = -Iinclude
LFLAGS = 
LIBS = 
LDFLAGS=$(LFLAGS) $(LIBS)

BUILD_DIR := build
SRC_DIRS := src
HEADER_DIRS := include

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Excluding from compiling testing file
TEST_FILE=test.c
LIB_SRCS := $(shell find $(SRC_DIRS) -type f -name '*.c' ! -name $(TEST_FILE))


# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Object files for library only
LIB_OBJS := $(LIB_SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)


# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(HEADER_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

.PHONY: clean

all:	CFLAGS+=$(DEFINE)
all:	clean $(BUILD_DIR)/$(NAME)

lib: 	clean $(BUILD_DIR)/lib$(NAME)
		
clean:
	@echo "\e[1;37m Cleaning build root \e[0m"
	@rm -rf $(BUILD_DIR)

# The final build step.
$(BUILD_DIR)/$(NAME): $(OBJS)
	@echo "\e[1;37m Creating executable: $@ \e[0m"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo "\e[1;37m Compiling: $@ \e[0m"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	
# Build static library
$(BUILD_DIR)/lib$(NAME): $(LIB_OBJS)
	@echo "\e[1;37m Creating library: lib$(NAME).a\e[0m"
	$(AR) rs $(BUILD_DIR)/lib$(NAME).a $(LIB_OBJS)
