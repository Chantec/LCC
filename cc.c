#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int token;            
char *src, *old_src;  
int poolsize;         
int line;             

void next() 
{
    token = *src++;
    return;
}

void expression(int level) {
}

void program() 
{
    next();                 
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}


int eval() 
{
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;


    argc--;//第一个是文件名
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;
    
    if ((fd = open(*argv, 0)) < 0)//打开文件 open(filename,access)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0)//读到的字符个数 放到了src[0:i-1]
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);

    program();
    return eval();
}