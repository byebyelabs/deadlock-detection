.PHONY: all clean

BUILD_SRC = src/locks.c src/detector.c src/rlocks.c
BUILD_DIR = build

FORMATTER = clang-format
SOURCES = $(shell find . -name "*.cpp" -o -name "*.c" -o -name "*.h")

LAB = byebye
PRJ = deadlocks
LIB = $(LAB)_$(PRJ)

all:
	@mkdir -p $(BUILD_DIR)
	clang -shared -fPIC -o $(BUILD_DIR)/$(LIB).so $(BUILD_SRC) -ldl

test/bank:
	@LD_PRELOAD=./$(BUILD_DIR)/$(LIB).so ./bank_test/bank-test || true

format:
	$(FORMATTER) -i $(SOURCES)

clean:
	@rm -rf $(BUILD_DIR)