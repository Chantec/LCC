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
//class里 num专指enum 
//lor logic or

//符号表 下标的映射
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};
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

//func_para
int index_of_bp;

//expression
int expr_type;


void debug_print_stack()
{
    printf("***hign address***\n");
    for(int *i=BP;i>=SP;i--)
        printf("%d\n",*(int *)i);
    printf("***stack over***\n");
}

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
                hash=hash*147+*src;//ltd why 147
                src++;
            }

           

                
            //最后有效字符 src-1 
            //id_begin->src-1  长度src-id_begin

            curr_id=symbols;
            while(curr_id[Token])//ltd 这里不用判断越界吗
            {
                if(curr_id[Hash]==hash&&memcmp((char*)curr_id[Name],id_begin,src-id_begin)==0)//bug 没有==0 ltd 为什么那样就dup了
                //原因：如果没有这个 int main中的int也就不是Int了，而是id，从前面的语法就不匹配了后面更乱
                {
                    //找到了
                    //debug print_id
                    // for(int i=id_begin;i<=src-1;++i)
                    //     printf("%c",*(char *)i);
                    // printf("\n");
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
                //这里需要(char)吗 ltd 这里用int行吗
            }
            src++;
            if(token=='"') 
                token_val=(int)id_begin;//字符串的起始地址
            else 
                token=Char;

            // printf("debug next str\n");
            // for(int p=(int)id_begin;p<(int)data;p++)
            //     printf("%c-",*(char *)p);
            // printf("\n");
            return;//bug 没有return
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
    return ;//ltd
}

void match(int tk)
{
    if(token!=tk)
    {
        printf("%d: expected token: %d %c\n", line, tk,tk);
        printf("current token : char:%c int:%d\n",token,token);
        exit(-1);
    }
    else 
        next();
}



void expression(int prec) 
{
    int *this_id;
    int tmp;//在函数模块是 充当函数参数个数
    if(token==Num)
    {
        match(Num);

        *++text=IMM;
        *++text=token_val;
        expr_type=INT;
    }
    else if(token=='"')
    {
        //这个时候已经存储str了，token这个其实是后面的引号
        *++text=IMM;
        *++text=token_val;//char *开始的地址

        match('"');//我们这里不实现多个引号连起来的str

        //需要用\0填充str后面的一个位置 因为默认是\0，所以直接跳过一个位置即可
        //data=data+1;//liangtodo
        data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
        expr_type = PTR;
    }
    else if(token==Sizeof)
    {
        
    }
    else if(token==Id)
    {
        this_id=curr_id;
        match(Id);

        //函数调用 add(1,2) printf("%d")
        if(token=='(')
        {
            tmp=0;
            match('(');
            while(token!=')')
            {
                
                expression(Assign);

                *++text=PUSH;
                tmp++;
                if(token!=')')
                    match(',');
            }
            match(')');

            if(this_id[Class]==Sys)
            {
                //系统调用
                *++text=this_id[Value];
            }
            else if(this_id[Class]==Fun)
            {
                //函数调用
                *++text=CALL;
                *++text=this_id[Value];
            }
            else 
            {
                printf("%d: bad func call\n",line);
                exit(-1);
            }

            //函数参数由调用者入栈 也由调用者出栈
            if(tmp>0)
            {
                *++text=ADJ;
                *++text=tmp;
            }
            expr_type = this_id[Type];//函数返回值的类型
        }
        else if(this_id[Class]==Num)
        {
            //枚举类型
            //ltd
        }
        else 
        {
            //普通变量
            if(this_id[Class]==Glo)
            {
                *++text=IMM;
                *++text=this_id[Value];
            }
            else if(this_id[Class]==Loc)
            {
                *++text=LEA;
                *++text=index_of_bp-this_id[Value];//value 存的是地址 从bp位置++
                //bp+(bp-地址)
                //bp+(bp-(bp+1))
                //bp-1
            }
            else 
            {
                printf("%d: undefined var\n", line);
                exit(-1);
            }
            //上面得到的只是地址
            expr_type = this_id[Type];
            *++text = (expr_type == CHAR) ? LC : LI;
        }
    }
    else if(token=='(')
    {
        //暂时不考虑强制转换
        match('(');
        expression(Assign);//ltd
        match(')');
    }
    else if(token==Mul)
    {
        //指针
    }
    else if(token==And)
    {
        //取地址
    }
    else if(token=='!')
    {
        //逻辑取反
    }
    else if(token=='~')
    {
        //按位取反
    }
    else if(token==Add)
    {
        //加号
    }
    else if(token==Sub)
    {
        //减号 转换成-1*某个东西
        next();
        *++text=IMM;
        *++text=-1;
        *++text=PUSH;
        expression(Inc);
        *++text=MUL;
    }
    else if(token==Inc||token==Dec)
    {
        //自增自减
    }
   


    //
    while(token>=prec)
    {
        tmp = expr_type;
        if(token==Assign)
        {
            match(Assign);
            if(*text==LC||*text==LI)
            {
                *text=PUSH;
            }
            else 
            {
                printf("%d: bad lvalue in assignment\n", line);
                exit(-1);
            }
            expression(Assign);
            expr_type = tmp;
            *++text = (expr_type == CHAR) ? SC : SI;
        }
        //cond 暂不实现 ltd
        else if(token==Lor)
        {

        }
        else if(token==Lan)
        {}
        else if(token==Or) 
        {}
        else if(token==And)//取地址
        {}
        else if(token==Eq)
        {
            //==
            next();
            *++text=PUSH;
            expression(Ne);//ltd
            *++text=EQ;
            expr_type=INT;//ltd
        }
        else if(token==Ne)
        {
            next();
            *++text=PUSH;
            expression(Lt);//ltd
            *++text=NE;
            expr_type=INT;
        }
        else if(token==Lt)
        {
            next();
            *++text=PUSH;
            expression(Shl);//ltd
            *++text=LT;
            expr_type=INT;
        }
        else if(token==Gt)
        {
            next();
            *++text=PUSH;
            expression(Shl);//ltd
            *++text=GT;
            expr_type=INT;
        }
        else if(token==Le)
        {
            next();
            *++text=PUSH;
            expression(Shl);//ltd
            *++text=LE;
            expr_type=INT;
        }
        else if(token==Ge)
        {
            next();
            *++text=PUSH;
            expression(Shl);//ltd
            *++text=GE;
            expr_type=INT;
        }
        else if(token==Shl)
        {}
        else if(token==Shr)
        {}
        else if(token==Add)
        {
            //先不考虑指针加法
            next();
            *++text=PUSH;
            expression(Mul);
            *++text=ADD;
            expr_type=INT;
        }
        else if(token==Sub)
        {
            //先不考虑指针减法
            next();
            *++text=PUSH;
            expression(Mul);
            *++text=SUB;
            expr_type=INT;
        }
        else if(token==Mul)
        {
            next();
            *++text=PUSH;
            expression(Inc);
            *++text=MUL;
            expr_type=INT;
        }
        else if(token==Div)
        {
            next();
            *++text=PUSH;
            expression(Inc);
            *++text=DIV;
            expr_type=INT;
        }
        else if(token==Mod)
        {
            next();
            *++text=PUSH;
            expression(Inc);
            *++text=MOD;
            expr_type=INT;//ltd
        }
        else if(token==Inc)
        {}
        else if(token==Dec)
        {}
        else if(token==Brak)
        {
            match(Brak);
            if(*text!=LC&&*text!=LI)
            {
                printf("%d: bad array use\n");
                exit(-1);
            }
            *text = PUSH;//
            expression(Assign);
            match(']');
        
            *++text = PUSH;
            
            *++text = IMM;
            *++text = sizeof(int);
            *++text = MUL;
          
            expr_type = tmp-PTR;
            *++text = ADD;
            *++text = (expr_type == CHAR) ? LC : LI;
        }
        else 
        {
            printf("%d: bad expr op\n");
            exit(-1);
        }

    }

}


void statement()
{
    // if (...) <statement> [else <statement>]
    // while (...) <statement>
    // { <statement> }
    // return xxx;
    // <empty statement>;
    // expression; (expression end with semicolon)
    
    //if

    // cond 
    // jz a 
    // true_statement
    // jmp b 
    // a: false_statement
    // b:next 


    int *a,*b;

    if(token==If)
    {
        match(If);
        match('(');
        expression(Assign);//ltd
        match(')');

        *++text=JZ;
        a=++text;

        statement();

        if(token==Else)
        {
            match(Else);

            *++text=JMP;
            b=++text;

            *a=(int)(text+1);//这里加的不是1个字节 而是一个单位 int

            statement();

            *b=(int)(text+1);   
        }
        else 
        {
            *a=(int)(text+1);
        }
    }
    else if(token==While)
    {
        match(While);
        
        match('(');
        //*a=(int)(text+1);
        a=text+1;
        expression(Assign);
        match(')');

        *++text=JZ;
        b=++text;

        statement();

        *++text=JMP;
        // *++text=*a;
        *++text=(int)a;
        *b=(int)(text+1);
    }
    else if(token==Return)
    {
        match(Return);

        if(token!=';') 
            expression(Assign);
        //*++text=PUSH;//ltd
        
        match(';');

        *++text=LEV;//ltd 返回值放哪了
    }
    else if(token=='{')//语句块
    {
        match('{');
        while(token!='}')
            statement();
        match('}');
    }
    else if(token==';')//空语句
    {
        match(';');
    }
    else 
    {
        //一般的语句 
        expression(Assign);
        match(';');
    }
}
void func_para()
{
    // parameter_decl ::= type {'*'} id {',' type {'*'} id}
    int var_type;
    int params;
    params=0;

    while(token!=')')
    {
        //like int a,int *b,char c
        if(token==Int)//bug 写成INT了 type是int
        {
            var_type=INT;
            next();
        }else if(token==Char)
        {
            var_type=CHAR;
            next();
        }

        while(token==Mul)
        {
            var_type=var_type+PTR;
            next();
        }

        //id
        if(token!=Id)
        {
            printf("%d: bad func para decl\n",line);
            exit(-1);
        }
        if(curr_id[Class]==Loc)//ltd
        {
            printf("%d duplicate func para decl\n",line);
            exit(-1);
        }
        match(Id);

        curr_id[BClass] = curr_id[Class]; curr_id[Class]  = Loc;
        curr_id[BType]  = curr_id[Type];  curr_id[Type]   = var_type;
        curr_id[BValue] = curr_id[Value]; curr_id[Value]  = params++; //距离bp的偏移量

        if(token!=')')//bug 写成}了
            match(',');
    }
    index_of_bp=params+1;//ltd


}
void func_body()
{
    // body_decl ::= {variable_decl}, {statement}
    int var_type;
    int pos_var;
    pos_var=index_of_bp;

    //变量声明部分 int a,*b;
    while(token==Int||token==Char)
    {
        if(token==Int)
        {
            basetype=INT;
            next();
        }
        else if(token==Char)
        {
            basetype=CHAR;
            next();
        }

        while(token!=';')
        {
            var_type=basetype;
            while(token==Mul)
            {
                var_type=var_type+PTR;
                next();
            }
            if (token!=Id) 
            {
                printf("%d: bad local decl\n", line);
                exit(-1);
            }
            if (curr_id[Class] == Loc)//此变量已经出现过了（在本函数内）
            {
                printf("%d: duplicate local decl\n", line);
                exit(-1);
            }
            match(Id);
            //把他的全局属性隐藏

            curr_id[BClass] = curr_id[Class]; curr_id[Class]  = Loc;
            curr_id[BType]  = curr_id[Type];  curr_id[Type]   = var_type;
            curr_id[BValue] = curr_id[Value]; curr_id[Value]  = ++pos_var;   

            if(token!=';')
                match(',');
        }
        match(';');
    }

    //为局部变量流出空间 bug写循环里面了 会有什么后果？
    *++text=ENT;
    *++text=pos_var-index_of_bp;//ltd

    while(token!='}')
        statement();
    
    *++text=LEV;//ltd

}
void func_decl()
{
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
    match('(');//其实这里应该是next 但是这样写看起来比较清楚
    func_para();
    match(')');
    match('{');
    func_body();
    match('}');

    //使所有的局部变量 恢复他之前的值
    curr_id=symbols;
    while(curr_id[Token])
    {
        if(curr_id[Class]==Loc)
        {
            curr_id[Class]=curr_id[BClass];
            curr_id[Type]=curr_id[BType];
            curr_id[Value]=curr_id[BValue];
        }
        curr_id=curr_id+IdSize;
    }
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
    int array;

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
        basetype=INT;
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

    match(Id);

    if(token=='(')
    {
        curr_id[Class]=Fun;
        curr_id[Type]=var_type;
        curr_id[Value]=(int)(text+1);//funcion的地址 text+1
        func_decl();
        return ;

    }

    if(token==Brak)
    {
        next();
        if(token!=Num)
        {
            printf("%d: bad array decl\n");
            exit(-1);
        }
        curr_id[Type]=PTR+var_type;
        curr_id[Class]=Glo;//global var
        curr_id[Value]=(int)data;
        
        data=(int)data+sizeof(int)*token_val;
        match(Num);
        match(']');
        match(';');
        return ;
    }


    //否则就是变量定义
    curr_id[Type]=var_type;
    curr_id[Class]=Glo;//global var
    curr_id[Value]=(int)data;//分配内存 全局变量
    data=data+sizeof(int);
    printf("\nvalue%d\n",curr_id[Value]);


    while(token!=';')
    {
        match(',');
        
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
    match(';');//bug 未写

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

        //debug_print_stack();
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
        else if (op == EXIT) { /*printf("exit(%d)", *SP); */return *SP;}
        else if (op == OPEN) { ax = open((char *)SP[1], SP[0]); }
        else if (op == CLOS) { ax = close(*SP);}
        else if (op == READ) { ax = read(SP[2], (char *)SP[1], *SP); }
        else if (op == PRTF) { tmp = SP+PC[1] ; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
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
void debug_print_asm()
{
    #ifndef debug 
    return ;
    #endif 

    printf("text:\n");
    printf("main:%d\n",idmain[Value]);
    for(int * first_inst=old_text;first_inst<=text;first_inst++)
    {
        printf("%d:",(int)first_inst);

        if(*first_inst==0) {printf("%s","LEA"); printf(" %d",*(++first_inst));}
        else if(*first_inst==1) {printf("%s","IMM"); printf(" %d",*(++first_inst));}
        else if(*first_inst==2) {printf("%s","JMP"); printf(" %d",*(++first_inst));}
        else if(*first_inst==3) {printf("%s","CALL"); printf(" %d",*(++first_inst));}
        else if(*first_inst==4) {printf("%s","JZ"); printf(" %d",*(++first_inst));}
        else if(*first_inst==5) {printf("%s","JNZ"); printf(" %d",*(++first_inst));}
        else if(*first_inst==6) {printf("%s","ENT"); printf(" %d",*(++first_inst));}
        else if(*first_inst==7) {printf("%s","ADJ"); printf(" %d",*(++first_inst));}
        else if(*first_inst==8) {printf("%s","LEV");}
        else if(*first_inst==9) {printf("%s","LI");}
        else if(*first_inst==10) {printf("%s","LC");}
        else if(*first_inst==11) {printf("%s","SI");}
        else if(*first_inst==12) {printf("%s","SC");}
        else if(*first_inst==13) {printf("%s","PUSH");}
        else if(*first_inst==14) {printf("%s","OR");}
        else if(*first_inst==15) {printf("%s","XOR");}
        else if(*first_inst==16) {printf("%s","AND");}
        else if(*first_inst==17) {printf("%s","EQ");}
        else if(*first_inst==18) {printf("%s","NE");}
        else if(*first_inst==19) {printf("%s","LT");}
        else if(*first_inst==20) {printf("%s","GT");}
        else if(*first_inst==21) {printf("%s","LE");}
        else if(*first_inst==22) {printf("%s","GE");}
        else if(*first_inst==23) {printf("%s","SHL");}
        else if(*first_inst==24) {printf("%s","SHR");}
        else if(*first_inst==25) {printf("%s","ADD");}
        else if(*first_inst==26) {printf("%s","SUB");}
        else if(*first_inst==27) {printf("%s","MUL");}
        else if(*first_inst==28) {printf("%s","DIV");}
        else if(*first_inst==29) {printf("%s","MOD");}
        else if(*first_inst==30) {printf("%s","OPEN");}
        else if(*first_inst==31) {printf("%s","READ");}
        else if(*first_inst==32) {printf("%s","CLOS");}
        else if(*first_inst==33) {printf("%s","PRTF");}
        else if(*first_inst==34) {printf("%s","MALC");}
        else if(*first_inst==35) {printf("%s","MSET");}
        else if(*first_inst==36) {printf("%s","MCMP");}
        else if(*first_inst==37) {printf("%s","EXIT");}

        printf("\n");
    }
}

int main(int argc, char **argv)
{  
    int i, fd;

    int *tmp;
    

    argc--;//第一个是文件名
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    

    //构建虚拟机
    
    //分配内存空间
    if((text=old_text=malloc(poolsize))==NULL||(data=malloc(poolsize))==NULL||(stack=malloc(poolsize))==NULL)
    {
        printf("could not malloc for vm");
        return -1;
    }
     if (!(symbols = malloc(poolsize))) {
        printf("could not malloc for symbol table\n");
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


    
    //debug 修改 *argv->hello.c
    //以恢复
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


    program();

    if (!(PC = (int *)idmain[Value]))
    {
        printf("main() not defined\n");
        return -1;
    }

    //ltd
    // setup stack
    SP = (int *)((int)stack + poolsize);
    *--SP = EXIT; // call exit if main returns
    *--SP = PUSH; tmp = SP;
    *--SP = argc;
    *--SP = (int)argv;
    *--SP = (int)tmp;

    debug_print_asm();
    return eval(); 
}
