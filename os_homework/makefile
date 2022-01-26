HEADERS = shell.h
OBJECTS = shell.o

default: shell

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

shell: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f shell

run: shell
	./shell
