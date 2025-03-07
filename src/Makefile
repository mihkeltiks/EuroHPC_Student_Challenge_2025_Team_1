###########################################################################################
# setup your parameters here
###########################################################################################
TARGET_EXEC ?= chromatic
BUILD_DIR ?= ./build
SRC_DIR ?= ./

INC_DIRS ?= 
LIB_DIRS ?= 
CPP_FLAGS ?=  -march=native -std=c++0x -O3 -s -mtune=native -funroll-loops -ffast-math  -fomit-frame-pointer -fopenmp #-ftree-parallelize-loops=8
CPP_LIBS ?= rt pthread gomp


# this works for linux:
MKDIR_P ?= mkdir -p
###########################################################################################



###########################################################################################
# automated execution (no modifications required) below this point
###########################################################################################
AUTO_INC_DIRS := 
INC_FLAGS := $(addprefix -I,$(AUTO_INC_DIRS) $(INC_DIRS) ./)
CPP_LIB_FLAGS := $(addprefix -l, $(CPP_LIBS))
LIB_DIRS_FLAGS := $(addprefix -L, $(LIB_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP $(CPP_FLAGS)
LDFLAGS := $(LIB_DIR_FLAGS) $(CPP_LIB_FLAGS)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: $(OBJS) $(TARGET_EXEC)

$(OBJS): | $(BUILD_DIR)

$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(MKDIR_P) $(BUILD_DIR)

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)


