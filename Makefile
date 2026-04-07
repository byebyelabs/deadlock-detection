.PHONY: all clean

BUILD_SRC = src/locks.c src/detector.c src/rlocks.c src/utils.c
BUILD_DIR = build

LAB = byebye
PRJ = deadlocks
LIB = $(LAB)_$(PRJ)

all:
	@mkdir -p $(BUILD_DIR)
	clang -shared -fPIC -o $(BUILD_DIR)/$(LIB).so $(BUILD_SRC) -ldl

test/bank:
	@LD_PRELOAD=./$(BUILD_DIR)/$(LIB).so ./bank_test/bank-test

clean:
	@rm -rf $(BUILD_DIR)