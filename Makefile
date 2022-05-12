run:cc.exe
	cc
build:cc.c 
	gcc -m32 cc.c -o cc
.PHONY:clean
clean:
	del *.exe