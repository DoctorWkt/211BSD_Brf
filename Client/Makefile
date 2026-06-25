CFLAGS= -O
#CFLAGS= -DDEBUG

OBJS=brf.o close.o open.o read.o write.o dup.o dup2.o stat.o lstat.o \
	fstat.o link.o unlink.o lseek.o rename.o access.o mkdir.o \
	rmdir.o chmod.o fchmod.o chown.o fchown.o
SOBJS=sysclose.o sysopen.o sysread.o syswrite.o sysdup.o sysdup2.o \
	sysfstat.o syslstat.o sysstat.o syslink.o sysunlink.o \
	syslseek.o sysrename.o sysaccess.o sysmkdir.o sysrmdir.o \
	syschmod.o sysfchmod.o syschown.o sysfchown.o

all: libbrf.a maketiny

test:
	(cd tests; ./runtests.sh)

maketiny:
	(cd tiny; make)

libbrf.a: $(OBJS) $(SOBJS)
	rm -f libbrf.a
	ar q libbrf.a $(OBJS) $(SOBJS)
	ranlib libbrf.a

clean:
	rm -f *.a *.o
	(cd tests; make clean)
	(cd tiny; make clean)
