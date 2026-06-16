all: test ulid

test: test.o ulid.o
	$(CC) $(LDFLAGS) -o $@ test.o ulid.o

ulid: ulid-main.o
	$(CC) $(LDFLAGS) -o $@ $<

ulid-main.o: ulid.c
	$(CC) $(CPPFLAGS) -DULID_MAIN $(CFLAGS) -c -o $@ $<

clean:
	$(RM) test.o ulid.o ulid-main.o test ulid
