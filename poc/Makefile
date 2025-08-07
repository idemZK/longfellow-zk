all:
	clang++ mdoc.cc -o mdoc ../install/lib/libmdoc_static.a -I../lib -lzstd -lcrypto

.PHONY: clean
clean:
	rm -rf mdoc
