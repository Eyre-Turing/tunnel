# For build all eyre_turing_lib and test.

# Input windows or linux.
SYSTEM = windows

# RELEASE_MODE input static or shared.
RELEASE_MODE = static

LIBPATH = ../eyrelib/

# LIBNEED is which lib you want to build.
ifeq ($(RELEASE_MODE),static)
LIBNEED = network framework
else
LIBNEED = framework network
endif

# Input which system lib compile link need.
SYSTEM_LIB_LINK = -lpthread
ifeq ($(SYSTEM),windows)
SYSTEM_LIB_LINK += -lws2_32
endif

# Like input -m32 for compilation a 32bit lib.
COMPILE_OPTION = 

TEST_USE_FOR = NOTHING

################################################################
# Don't change any of the following.

ifeq ($(SYSTEM),windows)
MAKE = mingw32-make
SHARED_LIB_SUFFIX = .dll
SHARED_LIB_PREFIX = 
STATIC_LIB_SUFFIX = .a
else
MAKE = make
SHARED_LIB_SUFFIX = .so
SHARED_LIB_PREFIX = lib
STATIC_LIB_SUFFIX = .a
endif

TARGET = server
OBJECT = server.o

MAKELIB_INC = $(patsubst %, -I$(LIBPATH)%/inc/, $(LIBNEED))

ifeq ($(RELEASE_MODE),static)
MAKELIB_OBJ = $(foreach n, $(LIBNEED), $(LIBPATH)$(n)/lib/eyre_$(n)$(STATIC_LIB_SUFFIX))
else
MAKELIB_OBJ = $(foreach n, $(LIBNEED), $(LIBPATH)$(n)/lib/$(SHARED_LIB_PREFIX)eyre_$(n)$(SHARED_LIB_SUFFIX))
MAKELIB_LINK = $(foreach n, $(LIBNEED), -L$(LIBPATH)$(n)/lib/ -Wl,-rpath=. -leyre_$(n))
endif

$(TARGET) : $(OBJECT) $(MAKELIB_OBJ)
ifeq ($(RELEASE_MODE),static)
	g++ $(COMPILE_OPTION) $^ $(MAKELIB_INC) $(SYSTEM_LIB_LINK) -o $@
else
	g++ $(COMPILE_OPTION) $(OBJECT) $(SYSTEM_LIB_LINK) $(MAKELIB_LINK) $(MAKELIB_INC) -o $@
endif

%.o : %.cpp
	g++ -c $(COMPILE_OPTION) $< $(MAKELIB_INC) -DUSE_FOR=$(TEST_USE_FOR) -o $@

%$(STATIC_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) SYSTEM=$(SYSTEM) static

%$(SHARED_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) SYSTEM=$(SYSTEM) shared

.PHONY: clean
clean :
ifeq ($(SYSTEM),windows)
	del $(subst /,\, $(OBJECT))
	del $(foreach n, $(subst /,\, $(dir $(MAKELIB_OBJ))), $(n)\*.o)
else
	rm -f $(OBJECT)
	rm -f $(foreach n, $(dir $(MAKELIB_OBJ)), $(n)/*.o)
endif

.PHONY: remove-lib
remove-lib :
ifeq ($(SYSTEM),windows)
	del $(subst /,\, $(MAKELIB_OBJ))
else
	rm -f $(MAKELIB_OBJ)
endif
