CC = g++
INCLUDES = -I../include 
LIBS += -L"../lib" -lTof -L"/usr/local/lib" -lusb-1.0 -lpthread 
# CPPFLAGS += $(INCLUDES) -Wall -O2 -g
CPPFLAGS += $(INCLUDES) -Wall -g

LDFLAGS	= $(LIBS)
TARGET = Mars04test

#.SILENT:

all: $(TARGET)

Mars04test:  SampleCode.o	
	$(CC) $(CFLAGS) $? -o $@ $(LIBS)
	
.c.o:	
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(TARGET) *.o
