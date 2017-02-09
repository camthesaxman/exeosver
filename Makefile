CC:=gcc
CFLAGS:=-ansi -Wall -Wextra -Wpedantic -Werror -Os -fwhole-program -s
TARGET:=exeosver
SOURCE:=exeosver.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET) $(TARGET).exe
