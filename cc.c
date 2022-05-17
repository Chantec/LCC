#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int token;            
char *src, *old_src;  
int poolsize;         
int line;         

//构建虚拟机
//内存空间
int *text,*old_text,*stack;
char *data;

//寄存器们
int *PC,*SP,*BP,ax;//PC永远是下一条没有执行过的指令或者或 执行完一条指令后 PC++

//指令
enum {LEA,IMM,JMP,CALL,JZ,JNZ,ENT,ADJ,LEV,LI,LC,SI,SC,PUSH,
       OR,XOR,AND,EQ,NE,LT,GT,LE,GE,SHL,SHR,ADD,SUB,MUL,DIV,MOD,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT};

//词法分析器
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
//从128开始是为了 保留asci 给单符号留出位置

//符号表 下标的映射
enum {Token, Hash, Name, Type, Class, Value, GType, GClass, GValue, IdSize};
//type:标识符的类型，即如果它是个变量，变量是 int 型、char 型还是指针型。
//class:类别

//token是符号类型 符号值 如果是数字就放在token_val里
int token_val;

int *curr_id,symbols;

// 类型 type
enum { CHAR, INT, PTR };
int *idmain;                 //指向main在符号表中的位置


//global_decl
int basetype;




void next() 
{
    char *id_begin;
    int hash;

    while(token=*src)
    {
        src++;
        if(token=='\n')
            line++;
        else if(token=='#')
        {
            //不支持宏 忽略它
            while(*src!=0&&*src!='\n')
                src++;
        }
        //标识符
        else if(token>='a'&&token<='z'||token>='A'&&token<='Z'||token=='_')
        {
            id_begin=src-1;
            hash=token;

            while(*src>='a'&&*src<='z'||*src>='A'&&*src<='Z'||*src=='_'||*src>='0'&&*src<='9')
            {
                src++;
                hash=hash*147+*src;//ltd why 147
            }
                
            //最后有效字符 src-1 
            //id_begin->src-1  长度src-id_begin

            curr_id=symbols;
            while(curr_id[Token])//ltd 这里不用判断越界吗
            {
                if(curr_id[Hash]==hash&&memcmp((char*)curr_id[Name],id_begin,src-id_begin))
                {
                    //找到了
                    token=curr_id[Token];
                    return ;
                }
                curr_id+=IdSize;//这里为啥不是IdSize+1 ltd
            }

            //注册标识符
            curr_id[Token]=Id;
            curr_id[Hash]=hash;
            curr_id[Name]=(int)id_begin;
            token=Id;
            return ;
        }
        //数字
        else if(token>='0'&&token<='9')
        {
            token_val=token-'0';
            while(*src>='0'&&*src<='9')
            {
                token_val=token_val*10+*src-'0';
                src++;
            }
            token=Num;
            return ;
        }
        //字符串 字符
        else if(token=='\''||token=='"')
        {
            id_begin=data;
            while (*src != 0 && *src != token)
            {
                if((token_val=*src++)=='\\')//如果是\*的形式 如果是\n则是\n 否则就是*
                {
                    if((token_val=*src++)=='n')
                        token_val='\n';
                }
                if(token=='"') *data++=token_val;//存储 ltd data移动的话 如果标识data开始 不用标识？
            }
            src++;
            if(token=='"') 
                token_val=(int)id_begin;
            else 
                token=Char;
        }

        else if(token=='/')
        {
            if(*src=='/')//注释
            {
                while(*src!=0&&*src!='\n')
                    src++;
            }
            else 
            {
                token==Div;
                return ;
            }
        }//== = ; < << <=;> >> >=; !=
        else if(token=='=')
        {
            if(*src=='=')
            {
                src++;
                token=Eq;
            }
            else 
            {
                token=Assign;
            }
            return ;
        }
        else if(token=='<')
        {
            if(*src=='=')
            {
                src++;
                token=Le;
            }
            else if(*src=='<')
            {
                src++;
                token=Shl;
            }
            else 
            {
                token=Lt;
            }
            return ;
        }
        else if(token=='>')
        {
            if(*src=='=')
            {
                src++;
                token=Ge;
            }
            else if(*src=='>')
            {
                src++;
                token=Shr;
            }
            else 
            {
                token=Gt;
            }
            return ;
        }
        else if(token=='!')
        {   //ltd 没有实现not
            if(*src=='=')
            {
                token=Ne;
                src++;
            }
            return ;
        }
        else if(token=='|')
        {
            if(*src=='|')
            {
                src++;
                token=Lor;
            }
            else 
            {
                token=Or;
            }
            return ;
        }
        else if(token=='&')
        {
            if(*src=='&')
            {
                src++;
                token=Lan;
            }
            else 
            {
                token=And;
            }
            return ;
        }
        else if(token=='+')
        {
            if(*src=='+')
            {
                src++;
                token=Inc;
            }
            else
            {
                token=Add;
            }
            return ;
        }
        else if(token=='-')
        {
            if(*src=='-')
            {
                src++;
                token=Dec;
            }
            else 
            {
                token=Sub;
            }
            return ;
        }
        else if(token=='^')
        {
            token=Xor;
            return ;
        }
        else if(token=='*')
        {
            token=Mul;
            return ;
        }
        else if(token=='%')
        {
            token=Mod;
            return ;
        }
        else if(token=='[')//ltd
        {
            token=Brak;
            return ;
        }
        else if(token=='?')//ltd
        {
            token=Cond;
            return ;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') 
        {
            return;
        }
    }
}

void match(int tk)
{
    if(token!=tk)
    {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
    else 
        next();
}



void expression(int level) {
}


void enum_decl()
{
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'
    //id ['=' 'num'] {',' id ['=' 'num'] 

    int i;
    i=0;
    while(token!='}')
    {
        if(token!=Id)
        {
            printf("%d: bad enum id %d\n",line,token);
            exit(-1);//ltd
        }
        next();
        if(token==Assign)
        {
            next();
            if(token!=Num)
            {
                printf("%d: bad enum init\n",line);
                exit(-1);
            }
            i=token_val;
            next();
        }

        //更新符号表
        curr_id[Class]=Num;//ltd 为什么是num
        curr_id[Type]=INT;
        curr_id[Value]=i++;

        if(token==',') next();
    }
}


void global_decl()
{

    // global_declaration ::= enum_decl | variable_decl | function_decl

    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'

    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'

    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


    int var_type;


    if(token==Enum)
    {
        next();
        if(token!='{')
            match(Id);//这个id实际上我们不做任何处理
        match('{');
        enum_decl();
        match('}');
        match(';');
        return ;
    }
    //like int * func(...){}
    //or   int *a,b,...,**c;
    if(token==Int)
    {
        basetype=CHAR;
        next();
    }
    else if(token==Char)
    {
        basetype=CHAR;
        next();
    }
    //处理 *(指针)
    var_type=basetype;//basetype相当于变量最前面的那个基础值 逗号分割的变量可以加不同的指针
    while(token==Mul)
    {
        var_type=var_type+PTR;
        next();
    }
    if(token!=Id)
    {
        printf("%d: bad global decl\n",line);
        exit(-1);
    }
    //如果已经声明过
    if(curr_id[Class])//ltd
    {
        printf("%d: duplicate global decl\n",line);
        exit(-1);
    }

    next();//match id

    if(token=='(')
    {
        curr_id[Class]=Fun;
        curr_id[Type]=var_type;
        curr_id[Value]=(int)(text+1);//funcion的地址 text+1
        func_decl();//ltd

        return ;

    }
    //否则就是变量定义
    curr_id[Type]=var_type;
    curr_id[Class]=Glo;//global var
    curr_id[Value]=(int)data;//分配内存 全局变量
    data=data+sizeof(int);


    while(token!=';')
    {
        match(',');//li
        
        var_type=basetype;
        while(token==Mul)
        {
            var_type=var_type+PTR;
            next();
        }
        if(token!=Id)
        {
            printf("%d: bad global decl\n",line);
            exit(-1);
        }
        //如果已经声明过
        if(curr_id[Class])//ltd
        {
            printf("%d: duplicate global decl\n",line);
            exit(-1);
        }
        next();//match id

        curr_id[Type]=var_type;
        curr_id[Class]=Glo;//global var
        curr_id[Value]=(int)data;//分配内存 全局变量
        data=data+sizeof(int);
    }
}

void program() 
{
    next();                 
    while (token > 0)
    {
        global_decl();
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

    if ((fd = open(*argv, 0)) < 0)//打开文件 open(filename,access)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    //init 初始化符号表

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

    i=Char;
    while(i<=While)
    {
        next();
        curr_id[Token]=i++;
    }

    //system call 内置函数

    i=OPEN;
    while(i<=EXIT)
    {
        next();
        curr_id[Class]=Sys;
        curr_id[Type]=INT;//ltd 
        curr_id[Value]=i++;//ltd
    }

    next();curr_id[Token]=Char;//void type ltd
    next();idmain=curr_id;//绑定


    //read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0)//读到的字符个数 放到了src[0:i-1]
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);
    //读入完毕

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



    // program();
    // return eval();
}
