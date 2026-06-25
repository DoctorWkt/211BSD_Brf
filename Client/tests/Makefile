all: test01 test02 test03 test04 test05 test06 test07

../libbrf.a:
	(cd ..; make)

test01: test01.c ../libbrf.a
	cc -o test01 test01.c ../libbrf.a

test02: test02.c ../libbrf.a
	cc -o test02 test02.c ../libbrf.a

test03: test03.c ../libbrf.a
	cc -o test03 test03.c ../libbrf.a

test04: test04.c ../libbrf.a
	cc -o test04 test04.c ../libbrf.a

test05: test05.c ../libbrf.a
	cc -o test05 test05.c ../libbrf.a

test06: test06.c ../libbrf.a
	cc -o test06 test06.c ../libbrf.a

test07: test07.c ../libbrf.a
	cc -o test07 test07.c ../libbrf.a

clean:
	rm -f test??
