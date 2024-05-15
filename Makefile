all: test

test: test.o ulid.o
	$(CC) -o $@ test.o ulid.o

clean:
	$(RM) test.o ulid.o test
