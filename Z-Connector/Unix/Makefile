SOURCES = ioutil.c main.c log.c serial.c ssl.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = z-agent
EXTRA_LIBS = -lssl -largtable2 -lcrypto

UNAME := $(shell uname)
PROCESSOR := $(shell uname -m)

# for FreeBSD
ifeq ($(UNAME), FreeBSD)
	LIBS += -L/usr/local/lib
	INCLUDES += -I/usr/local/include
endif

# for Mac OS X
ifeq ($(UNAME), Darwin)
	LIBS += -L/opt/local/lib
	INCLUDES += -I/opt/local/include
endif

# for OpenWRT lib dependacies
ifneq ($(STAGING_DIR), )
	LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
endif

all: $(EXECUTABLE) install

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJECTS) $(EXTRA_LIBS) -o $@

.c.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o $(EXECUTABLE)

install:
	# copy $(EXECUTABLE) to bin folder
	cp $(EXECUTABLE) Release/Raspberry/z-agent
	
	# copy $(EXECUTABLE) to /var/local
#	sudo cp $(EXECUTABLE) /var/local/z-agent
	
	# copy init.d script to /etc/init.d
#	sudo cp Release/init.d/zagent /etc/init.d/zagent
	
