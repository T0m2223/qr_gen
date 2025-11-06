BIN := qr
CFLAGS := -Wall -Wextra -Werror
BUILD_DIR := build

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRCS))

$(BUILD_DIR)/$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean run

clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(BIN)
	./$(BUILD_DIR)/$(BIN) "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." "M"
