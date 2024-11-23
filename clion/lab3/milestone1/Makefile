

ourtest : dplist.c ourtest.c
	mkdir -p build
	gcc dplist.c ourtest.c -o build/dplist_ourtest
	./build/dplist_ourtest

clean:
	rm -r build

zip:
	zip milestone1.zip *.c *.h Makefile