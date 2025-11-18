BUILD_DIR      := build
RELEASE_DIR    := $(BUILD_DIR)/release
TEST_DIR       := $(BUILD_DIR)/test
TARGET_RELEASE := $(RELEASE_DIR)/qr-gen
TARGET_TEST    := $(TEST_DIR)/test

SRCS  := $(wildcard qr/*.c)
OBJS  := $(patsubst qr/%.c, $(RELEASE_DIR)/%.o, $(SRCS))
TESTS := $(wildcard test/*.c)
TOBJS := $(patsubst test/%.c, $(TEST_DIR)/%.o, $(TESTS))

CFLAGS := -Wall -Wextra -Werror -I.
ifdef NDEBUG
CFLAGS += -DNDEBUG
endif
TESTFLAGS := -Wl,--allow-multiple-definition

all: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJS) | $(RELEASE_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_TEST): $(filter-out $(RELEASE_DIR)/main.o,$(OBJS)) $(TOBJS) | $(TEST_DIR)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@ $^

$(RELEASE_DIR)/%.o: qr/%.c | $(RELEASE_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%.o: test/%.c | $(TEST_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(RELEASE_DIR) $(TEST_DIR):
	mkdir -p $@

.PHONY: all clean run test

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR) $(TEST_DIR)

run: $(TARGET_RELEASE)
	./$(TARGET_RELEASE) $(ARGS)

test: $(TARGET_TEST)
	./$(TARGET_TEST) $(ARGS)
