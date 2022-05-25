build:cc.c 
	gcc -m32 -w cc.c -o cc
.PHONY:clean
clean:
	del *.exe