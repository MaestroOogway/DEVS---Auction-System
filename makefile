CC = g++
CFLAGS = -std=c++17
INCLUDECADMIUM = -I ../../cadmium/include
INCLUDEDESTIMES = -I ../../DESTimes/include

# Crear carpetas para guardar los archivos compilados
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

# Compilar message.cpp
message.o: data_structures/message.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) \
	data_structures/message.cpp -o build/message.o

# Compilar main_affective_test.cpp
main_affective_test.o: test/main_affective_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) \
	test/main_affective_test.cpp -o build/main_affective_test.o

# Enlazar los archivos para crear el ejecutable
affective: main_affective_test.o message.o
	$(CC) -g -o bin/AFFECTIVE_TEST build/main_affective_test.o build/message.o

# Compilar main_rational_test.cpp
main_rational_test.o: test/main_rational_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) \
	test/main_rational_test.cpp -o build/main_rational_test.o

# Enlazar los archivos para crear el ejecutable
rational: main_rational_test.o message.o
	$(CC) -g -o bin/RATIONAL_TEST build/main_rational_test.o build/message.o

# Compilar main_auctioneer_test.cpp
main_auctioneer_test.o: test/main_auctioneer_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) \
	test/main_auctioneer_test.cpp -o build/main_auctioneer_test.o

# Enlazar los archivos para crear el ejecutable
auctioneer: main_auctioneer_test.o message.o
	$(CC) -g -o bin/AUCTIONEER_TEST build/main_auctioneer_test.o build/message.o

# Compilar main_top.cpp (ABP SIMULATOR)
main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o

# Enlazar los archivos para crear el ejecutable ABP SIMULATOR
simulator: main_top.o message.o
	$(CC) -g -o bin/ABP build/main_top.o build/message.o

# Compilar todo
all: simulator

# Limpiar archivos compilados
clean:
	rm -f bin/* build/*
