CC := clang
CFLAGS := -g

all: bank-test

clean:
	rm -f bank-test

bank-test: main.c bank.c bank.h
	$(CC) $(CFLAGS) -o bank-test main.c bank.c -lpthread