SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk

all: mac

mac:
	@echo Compiling…;
	@gcc src/mac.c -mdynamic-no-pic -dead_strip \
	-I$(SDKROOT)/System/Library/Frameworks/System.framework/PrivateHeaders \
	 -DINET6 -DNO_IPX -o MACSpoof;
	@echo Done.;

install:
	cp MACSpoof /usr/local/bin/MACSpoof