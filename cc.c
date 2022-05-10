#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int token;            
char *src, *old_src;  
int poolsize;         
int line;         

/**********构建虚拟机**********/
//内存空间
int *text,*old_text,*stack;
char *data;

//寄存器们
int *PC,*SP,*BP,ax;//PC永远是下一条没有执行过的指令或者或 执行完一条指令后 PC++

//指令
enum {LEA,IMM,JMP,CALL,JZ,JNZ,ENT,ADJ,LEV,LI,LC,SI,SC,PUSH,
       OR,XOR,AND,EQ,NE,LT,GT,LE,GE,SHL,SHR,ADD,SUB,MUL,DIV,MOD,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT};
/**********构建虚拟机end**********/


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
    int op,*tmp;//ltd
    while(1)
    {
        op=*PC++;
        if(op==IMM) 
        {
            ax=*PC++;
        }
        else if(op==LC)//load char
        {
            ax=*(char *)ax;
        } 
        else if(op==LI)
        {
            ax=*(int *)ax;
        }
        else if(op==SC)
        {
            *(char *)*SP=ax;//出栈SP++
            SP++;
        }
        else if(op==SI)
        {
            *(int *)*SP=ax;
            SP++;
        }
        else if(op==PUSH)
        {
            *(--SP)=ax;
        }
        else if(op==JMP)
        {
            PC=(int *)*PC;
        }
        else if(op==JZ)
        {
            PC=ax?PC+1:(int *)*PC;
        }
        else if(op==JNZ)
        {
            PC=ax?(int *)*PC:PC+1;
        }
        else if(op==CALL)
        {
            *(--SP)=(int)(PC+1);
            PC=(int *)*PC;
        }
        else if(op==ENT)
        {
            *(--SP)=(int)BP;
            BP=SP;
            SP=SP-*PC++;//这个PC里面存的应该是变脸的个数？
        }
        else if(op==ADJ)
        {
            SP=SP+*PC++;//
        }
        else if(op==LEV)//相当于函数返回的时候
        {
            SP=BP;
            BP=(int*)*SP++;
            PC=(int*)*SP++;
        }
        else if(op==LEA)
        {
            ax=(int)(BP+*PC++);//真实的计算机中用的是SP吧
        }//op1 栈上 op2在ax 
        else if (op == OR)  ax = *SP++ | ax;
        else if (op == XOR) ax = *SP++ ^ ax;
        else if (op == AND) ax = *SP++ & ax;
        else if (op == EQ)  ax = *SP++ == ax;
        else if (op == NE)  ax = *SP++ != ax;
        else if (op == LT)  ax = *SP++ < ax;
        else if (op == LE)  ax = *SP++ <= ax;
        else if (op == GT)  ax = *SP++ >  ax;
        else if (op == GE)  ax = *SP++ >= ax;
        else if (op == SHL) ax = *SP++ << ax;
        else if (op == SHR) ax = *SP++ >> ax;
        else if (op == ADD) ax = *SP++ + ax;
        else if (op == SUB) ax = *SP++ - ax;
        else if (op == MUL) ax = *SP++ * ax;
        else if (op == DIV) ax = *SP++ / ax;
        else if (op == MOD) ax = *SP++ % ax;//下面的未看 ltd
        else if (op == EXIT) { printf("exit(%d)", *SP); return *SP;}
        else if (op == OPEN) { ax = open((char *)SP[1], SP[0]); }
        else if (op == CLOS) { ax = close(*SP);}
        else if (op == READ) { ax = read(SP[2], (char *)SP[1], *SP); }
        else if (op == PRTF) { tmp = SP + PC[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (int)malloc(*SP);}
        else if (op == MSET) { ax = (int)memset((char *)SP[2], SP[1], *SP);}
        else if (op == MCMP) { ax = memcmp((char *)SP[2], (char *)SP[1], *SP);}
        else 
        {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
   
    int i, fd;
    

    argc--;//第一个是文件名
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;
    
    goto test;

    if ((fd = open(*argv, 0)) < 0)//打开文件 open(filename,access)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    //read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0)//读到的字符个数 放到了src[0:i-1]
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);
    //读入完毕
test:

    //构建虚拟机
    
    //分配内存空间
    if((text=old_text=malloc(poolsize))==NULL||(data=malloc(poolsize))==NULL||(stack=malloc(poolsize))==NULL)
    {
        printf("could not malloc for vm");
        return -1;
    }
    //初始化空间
    memset(text,0,poolsize);
    memset(data,0,poolsize);
    memset(stack,0,poolsize);

    //初始化寄存器
    SP=BP=(int*)((int)stack+poolsize);
    PC=0;
    ax=0;

    //test 中间代码
    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    PC = text;
    
    eval();
    
   




    // program();
    // return eval();
}