  CC = g++
  CFLAGS  =-lpthread
 
  SERVER = server
  DEVICE = device
 
  all: $(SERVER) $(DEVICE)
 
  $(SERVER): $(SERVER).cpp
	$(CC) -o $(SERVER) $(SERVER).cpp $(CFLAGS)

  $(DEVICE): $(DEVICE).cpp
	$(CC) -o $(DEVICE) $(DEVICE).cpp 
 
  clean:
	$(RM) $(SERVER)
	$(RM) $(DEVICE)
