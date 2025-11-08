BIN := qr
CFLAGS := -Wall -Wextra -Werror
BUILD_DIR := build

SRCS := $(wildcard qr/*.c)
OBJS := $(patsubst qr/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TESTFLAGS := -I.

$(BUILD_DIR)/$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: qr/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean run

clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(BIN)
	./$(BUILD_DIR)/$(BIN) $(ARGS)
