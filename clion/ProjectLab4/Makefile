# Build target
all: main.out

# Compile main program
main.out: main.c sbuffer.c
	gcc main.c sbuffer.c -o main.out -lpthread

# Compile file creator program
gen: config.h file_creator.c
	gcc -g file_creator.c config.h -o gen.out
	./gen.out

# Clean target
clean:
	rm -f main.out file_creator.out sensor_data sensor_data_text room_sensor.map sensor_data_out.csv

# Run main program
run: main.out
	./main.out

# Run file creator
create_files: file_creator.out
	./file_creator.out

# Zip target
zip:
	zip milestone3.zip *.c *.h Makefile
