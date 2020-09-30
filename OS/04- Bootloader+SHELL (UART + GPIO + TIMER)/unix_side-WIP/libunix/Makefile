#   http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# what i understood is we tell makefile the header files that must be present
# these are described in the dependency files
# sometimes we change the code and the dependency changes
# we need to update, but we can forget to do so
# the weird stuff below does it for us // maybe i don't need it yet? i'm still noob on these stuff


VPATH := .

SOURCES := $(wildcard ./*.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))

CFLAGS = -Og -Wall -Wno-unused-variable -Wno-unused-function -Werror -DLIB_UNUX -I.
CC = gcc

TARGET := libunix.a
BUILD_DIR := ./objs

OBJS := $(foreach o, $(OBJS), $(BUILD_DIR)/$(notdir $o))
DEPS := $(OBJS:.o=.d) 

all: $(TARGET)

# https://www.howtogeek.com/427086/how-to-use-linuxs-ar-command-to-create-static-libraries/
$(TARGET): $(OBJS) 
	ar cr $(TARGET) $(OBJS) 


$(BUILD_DIR)/%.o : %.c
$(BUILD_DIR)/%.o : %.c $(BUILD_DIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<


$(BUILD_DIR)/%.o : %.S
$(BUILD_DIR)/%.o : %.S $(BUILD_DIR)/%.d
	$(CC) $(CPP_ASFLAGS) $<


$(BUILD_DIR)/%.d: ;
.PRECIOUS: $(BUILD_DIR)/%.d


ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

clean:
	rm -rf $(BUILD_DIR) $(TARGET) *~ tags *.o

.PHONY: clean all