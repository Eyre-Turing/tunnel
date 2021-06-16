# For build network lib.

SYSTEM = windows

SHOWINFO = -DNETWORK_DETAIL=0

COMPILE_OPTION = 

###############################################################
# Don't change any of the following.

TARGET = network

INC = -I../inc/ -I../../framework/inc/

SYSTEM_LIB_LINK = -lpthread

MAKELIB_LINK = -L../../framework/lib/ -leyre_framework

ifeq ($(SYSTEM),windows)
SYSTEM_LIB_LINK += -lws2_32
endif

SOURCE = $(wildcard *.cpp)
OBJECT = $(patsubst %.cpp, %.o, $(SOURCE))

static : $(OBJECT)
	ar crv $(TARGET) $^

%.o : %.cpp
	g++ -c $(COMPILE_OPTION) $< $(SHOWINFO) $(INC) -o $@

shared :
	g++ -shared -fPIC $(COMPILE_OPTION) $(SOURCE) $(SHOWINFO) $(INC) $(MAKELIB_LINK) $(SYSTEM_LIB_LINK) -o $(TARGET)

.PHONY: clean
clean :
ifeq ($(SYSTEM),windows)
	del $(OBJECT)
else
	rm -f $(OBJECT)
endif
