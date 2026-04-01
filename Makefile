
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
TARGET  := TICK
 
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
 
.PHONY: all clean debug rebuild run
 
all: CFLAGS += -O2
all: $(TARGET)
 
debug: CFLAGS += -g -O0
debug: $(TARGET)
 
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lws2_32
 
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
 
-include $(OBJS:.o=.d)
 
clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)
 
rebuild: clean all
 
run: all
	./$(TARGET)
