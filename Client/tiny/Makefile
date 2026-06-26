CFLAGS= -O -i

all: tcp tcat tstat tprdir tls trm tln tmv tmkdir \
	trmdir tchmod tchown tto

../libbrf.a:
	(cd ..; make)

tcp: tcp.c ../libbrf.a
	cc -o tcp $(CFLAGS) tcp.c ../libbrf.a

tcat: tcat.c ../libbrf.a
	cc -o tcat $(CFLAGS) tcat.c ../libbrf.a

tstat: tstat.c ../libbrf.a
	cc -o tstat $(CFLAGS) tstat.c ../libbrf.a

tprdir: tprdir.c ../libbrf.a
	cc -o tprdir $(CFLAGS) tprdir.c -u _open -u _close -u _read ../libbrf.a

tls: tls.c ../libbrf.a
	cc -o tls $(CFLAGS) tls.c -u _open -u _close -u _read ../libbrf.a

trm: trm.c ../libbrf.a
	cc -o trm $(CFLAGS) trm.c ../libbrf.a

tln: tln.c ../libbrf.a
	cc -o tln $(CFLAGS) tln.c ../libbrf.a

tmv: tmv.c ../libbrf.a
	cc -o tmv $(CFLAGS) tmv.c ../libbrf.a

tmkdir: tmkdir.c ../libbrf.a
	cc -o tmkdir $(CFLAGS) tmkdir.c ../libbrf.a

trmdir: trmdir.c ../libbrf.a
	cc -o trmdir $(CFLAGS) trmdir.c ../libbrf.a

tchmod: tchmod.c ../libbrf.a
	cc -o tchmod $(CFLAGS) tchmod.c ../libbrf.a

tchown: tchown.c ../libbrf.a
	cc -o tchown $(CFLAGS) tchown.c ../libbrf.a

tto: tto.c ../libbrf.a
	cc -o tto $(CFLAGS) tto.c ../libbrf.a

clean:
	rm -f tcp tcat tstat tprdir tls trm tln tmv \
	tmkdir trmdir tchmod tchown tto
