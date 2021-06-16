SYSTEM = windows

ifeq ($(SYSTEM),windows)
MAKE = mingw32-make
else
MAKE = make
endif

all :
	cd server && $(MAKE)
	cd client && $(MAKE)

.PHONY: clean
clean :
	cd server && $(MAKE) clean
	cd client && $(MAKE) clean

.PHONY: remove-lib
remove-lib :
	cd server && $(MAKE) remove-lib
	cd client && $(MAKE) remove-lib
