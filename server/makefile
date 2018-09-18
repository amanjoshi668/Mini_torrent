CC = g++
CFLAGS = -Wall -std=c++1z -w -pthread
FLAGS = -I. -L. -L/usr/lib -lssl -lcrypto
DEPS = common.h message.h respond_to_request.h 
OBJ = server.o message.o respond_to_request.o 

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) $(FLAGS) -c -o $@ $<

server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm $(OBJ)