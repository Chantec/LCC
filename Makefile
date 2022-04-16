build:cc.c 
	gcc -std=c11 cc.c -o cc
	cc
.PHONY:clean
clean:
	del *.exe