SYSTEM = windows

ifeq ($(SYSTEM),windows)
MAKE = mingw32-make
else
MAKE = make
endif

all :
	cd server && $(MAKE) SYSTEM=$(SYSTEM)
	cd client && $(MAKE) SYSTEM=$(SYSTEM)

.PHONY: clean
clean :
	cd server && $(MAKE) SYSTEM=$(SYSTEM) clean
	cd client && $(MAKE) SYSTEM=$(SYSTEM) clean

.PHONY: remove-lib
remove-lib :
	cd server && $(MAKE) SYSTEM=$(SYSTEM) remove-lib
	cd client && $(MAKE) SYSTEM=$(SYSTEM) remove-lib
