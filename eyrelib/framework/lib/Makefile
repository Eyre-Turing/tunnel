# For build framework lib.

SYSTEM = windows

SHOWINFO = -DEYRE_DEBUG=1 -DEYRE_WARNING=1 -DEYRE_DETAIL=0
BYTEARRAY_SERVE = 
CODEC_SYS = 
CODEC_AUTO = 

COMPILE_OPTION = 

###############################################################
# Don't change any of the following.

TARGET = framework

INC = -I../inc/

SOURCE = $(wildcard *.cpp)
OBJECT = $(patsubst %.cpp, %.o, $(SOURCE))

OPTIONS = 

ifneq ($(BYTEARRAY_SERVE),)
OPTIONS += -DEYRE_BA_SERVE=$(BYTEARRAY_SERVE)
endif

ifneq ($(CODEC_SYS),)
OPTIONS += -DCODEC_SYS_DEF=$(CODEC_SYS)
endif

ifneq ($(CODEC_AUTO),)
OPTIONS += -DCODEC_AUTO_DEF=$(CODEC_AUTO)
endif

static : $(OBJECT)
	ar crv $(TARGET) $^

%.o : %.cpp
	g++ -c $(COMPILE_OPTION) $< $(SHOWINFO) $(OPTIONS) $(INC) -o $@

shared :
	g++ -shared -fPIC $(COMPILE_OPTION) $(SOURCE) $(SHOWINFO) $(OPTIONS) $(INC) -o $(TARGET)

.PHONY: clean
clean :
ifeq ($(SYSTEM),windows)
	del $(OBJECT)
else
	rm -f $(OBJECT)
endif
