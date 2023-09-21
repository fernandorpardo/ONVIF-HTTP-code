CFLAGS = -Wall -g -fmax-errors=2
CC= g++ -std=c++0x
BIN=tapo
HEADERS= tapo.h cl.h glib.h cstr.h HTTPlib.h onvif.h server.h local.h
OBJS=tapo.o cl.o glib.o cstr.o HTTPlib.o onvif.o server.o 
ZLIBOBJS= ../zlib-1.2.13/adler32.o ../zlib-1.2.13/gzlib.o ../zlib-1.2.13/compress.o ../zlib-1.2.13/gzread.o \
../zlib-1.2.13/crc32.o ../zlib-1.2.13/gzwrite.o ../zlib-1.2.13/trees.o ../zlib-1.2.13/deflate.o \
../zlib-1.2.13/infback.o  ../zlib-1.2.13/uncompr.o ../zlib-1.2.13/inffast.o ../zlib-1.2.13/zutil.o \
../zlib-1.2.13/inflate.o ../zlib-1.2.13/gzclose.o ../zlib-1.2.13/inftrees.o
SSL_LIBS= -lcrypto -lssl

# make ZLIB=1 to include ZLIB support option

all: tapo
ifdef ZLIB
	@echo "ZLIB support included"
endif
version: 
	$(CC) $(CFLAGS) -c version.cpp -o version.o	
cl.o: cl.cpp cl.h
	$(CC) $(CFLAGS) -c cl.cpp -o cl.o
glib.o: glib.cpp glib.h
	$(CC) $(CFLAGS) -c glib.cpp -o glib.o
cstr.o: cstr.cpp cstr.h
	$(CC) $(CFLAGS) -c cstr.cpp -o cstr.o
HTTPlib.o: HTTPlib.cpp HTTPlib.h glib.h cstr.h
ifdef ZLIB
	$(CC) $(CFLAGS) -DZLIB_SUPPORT -c HTTPlib.cpp -o HTTPlib.o
else
	$(CC) $(CFLAGS) -c HTTPlib.cpp -o HTTPlib.o
endif
onvif.o: onvif.cpp onvif.h
	$(CC) $(CFLAGS) -c onvif.cpp -o onvif.o
server.o: server.cpp server.h
	$(CC) $(CFLAGS) -c server.cpp -o server.o
tapo.o: tapo.cpp tapo.h $(HEADERS)
	$(CC) $(CFLAGS) -c $(BIN).cpp -o $(BIN).o
tapo: $(OBJS) version
ifdef ZLIB
	$(CC) -o $(BIN) $(OBJS) $(ZLIBOBJS) $(SSL_LIBS) version.o
else
	$(CC) -o $(BIN) $(OBJS) $(SSL_LIBS) version.o
endif
	mv tapo ~/bin
clean:
	rm -f ~/bin/tapo *.o
	@rm -f ~/bin/$(BIN)