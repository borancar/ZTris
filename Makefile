CC=gcc
RC=windres

OBJS = about.o bmpload.o bodlevel.o iduca.o igra.o main.o resource.o randgen.o rang.o settings.o skin.o

.SUFFIXES: .c .o .rc .o

.c.o:
	$(CC) -c -DUNICODE -D_UNICODE $<

.rc.o:
	$(RC) -DUNICODE -D_UNICODE $< $@

ZTris.exe: $(OBJS)
	$(CC) -mwindows $(OBJS) -lgdi32 -lcomctl32 -lversion -o ZTris.exe

clean:
	rm *.o
