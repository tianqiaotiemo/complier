#include "parse.h"
#include "global.h"
static treenode * entitypara_list();  ///用于自定义函数的参数
static treenode * BUER_stmt();        ///布尔运算语句
static treenode * newlnodestmt(Stmtkind stmt); ///申请语句结构结点
static treenode * newlnodeexp(Expkind exp);    ///申请算式结构结点
static treenode * term();                      ///带乘除的算术式
static treenode * factor();                    ///主要对于加减已经更低等级模块
static treenode * structure_stmt();             ///一个代码块{}
static treenode * instmt_sequence();            ///语句结构集合
static treenode * for_stmt();                   ///for语句
static treenode * while_stmt();                 ///while语句
static treenode * if_stmt();                    ///if语句
static treenode * switch_stmt();                ///switch语句
static treenode * case_stmt();                  ///case语句
static treenode * break_stmt();                 ///break语句
static treenode * default_stmt();               ///default语句
static treenode * statement();                  ///基本语句
static treenode * definepara_stmt();            ///定义参数
static treenode * quotepara_list();             ///函数调用的参数
static treenode * exp2();                       ///四则运算
static treenode * fun_apply();                  ///函数调用
static treenode * assign_stmt();                ///赋值语句
static treenode * define_assign();              ///定义语句
static treenode * exp();                        ///四则运算的更低模块
static treenode * simple_exp();                 ///更简单的运算模块
static treenode * input_para_list();            ///输入的参数
static treenode * struct_stmt();                ///结构体语句
int isin_first_instmt_sequence();               ///token是否在语句序列中
void parse_printtree();                         ///输出抽象语法树
void printtree(treenode *lnode);
void printnode(treenode *lnode);
void parse();                                   ///语法分析开始
void gettoken();                                ///获取token
void match(tokentype expectedtoken);            ///匹配
int count_row();                                ///计算token所在行数，主要用与语义分析
int isdtype();                                  ///是否是定义类型
int iscomparison_op();                          ///判断是否是比较运算符

int mainno=0;/**用于判断main函数的个数**/
int ptokenno=0;/**当前记号的计数器。从第0个开始，总是对应当前记号的前一个**/
int printtree_t=0;/**输出语法树的参数**/
treenode *tree_gen;/**语法树的根**/
tokentype dtype[4]= {INT,FLOAT,CHAR,DOUBLE}; /**变量或函数返回值（除VOID以外）的类型**/

///匹配词法中识别的token,并取下一个记号
void match(tokentype expectedtoken)
{

    if(expectedtoken==token)
    {
        gettoken();
    }
    else
    {
        printf("error");
    }
}
/**获得结点**/
void gettoken()
{
    token=token_table[ptokenno].tokenval;
    ptokenno++;
}

///计算记号位于源代码的哪一行
int count_row()
{
    int i;
    for(i=1; i<=lineno; i++)
    {
        if((ptokenno-1)<=line_num_table[i])return i;
    }
}

void parse()
{
    treenode *newtemp=NULL;
    treenode *temp=NULL;
    treenode *gen=NULL;
    treenode *dakuohao;
    gettoken();

    int root_tag=0;  ///用于根节点的判断

    while(ptokenno<tokenno)
    {
        if(isdtype()||(token==STRUCT))
        {
            if(token==INT&&token_table[ptokenno].tokenval == MAIN)
            {
                root_tag++;
                newtemp = newlnodestmt(maink);
                match(INT);
                match(MAIN);
                match(L_XI);
                if(token!=R_XI)
                {
                    newtemp->child[1]=entitypara_list();  ///参数列表
                }
                match(R_XI);
                if(token!=L_DA)
                {
                    printf("error",count_row());
                }
                else
                {
                    newtemp->child[2]=structure_stmt();/**函数实体结构**/
                }
                mainno++;
            }
            else if(token_table[ptokenno].tokenval==ID&&token_table[ptokenno+1].tokenval==L_XI) ///自定义函数
            {
                root_tag++;
                newtemp=newlnodestmt(funck);///动态申请一条语句结构的节点
                match(token);
                treenode *j=newlnodeexp(idk);
                match(ID);
                newtemp->child[0]=j; ///自定义函数名
                match(L_XI);

                if(token!=R_XI)
                {
                    newtemp->child[1]=entitypara_list();  ///参数列表
                }

                match(R_XI);
                if(token!=FENH&&token!=L_DA)
                {
                    printf("error missing&& function error\n");
                }

                else
                {
                    newtemp->child[2]=structure_stmt();
                }

            }
            else if(token==STRUCT&&token_table[ptokenno].tokenval==ID&&token_table[ptokenno+1].tokenval==L_DA)
            {
                root_tag++;
                newtemp=newlnodestmt(structk);
                match(token);
                treenode *j=newlnodeexp(idk);
                match(ID);
                newtemp->child[0]=j;

                if(token==L_DA)
                {
                    newtemp->child[1]=structure_stmt(); ///这里为了简化赋值
                }
            }
            else ///变量声明--全局  if(token==ID)
            {
                root_tag++;
                newtemp=newlnodestmt(defineparak);

                match(token);

                if(token!=ID)
                {
                    printf("变量定义有误\n",count_row());
                }

                newtemp->child[0]=define_assign();

                if(token!=FENH)
                {
                    printf("error ;",count_row);
                }
                else
                    match(token);
            }



            if(temp!=NULL)
                temp->sibling=newtemp;
            temp=newtemp;

            if(root_tag==1)
                gen=temp;
        }

        else
        {
            if(token==ID)
            {
                printf("未定义\n");
            }
            else
            {
                printf("非法188\n");

            }
        }

    }
    if(mainno!=1)
    {
        printf("error\n",count_row());
    }

    tree_gen=gen;
    ///打印抽象语法树
    parse_printtree();
}



/**申请一条语句结构的节点
if for while return assign fun define main define**/
treenode *newlnodestmt(Stmtkind stmt)
{
    /**为新结点初始化**/
    treenode *t=(treenode *)malloc(sizeof(treenode));
    t->nodekind=stmtk;
    t->kind.stmt=stmt;
    t->lineno=count_row();
    int i;
    for(i=0; i<4; i++)
        t->child[i]=NULL;
    t->sibling=NULL;

    if(stmt==maink)/**如果是主函数**/
    {
        t->dtype=token;
    }

    if(stmt==defineparak) /**如果是定义**/
    {
        t->dtype=token;
    }

    if(stmt==funck)
    {
        if(isdtype())
            t->dtype=token;
    }
    return t;
}

///服务于自定义函数的参数  形如 fun(int a,int b)
treenode *entitypara_list()
{
    treenode *t;
    treenode *temp;
    treenode *temp1;
    treenode *newtemp;

    temp=newlnodestmt(defineparak);
    t=temp; ///先把返回的首地址控制住
    if(isdtype())
    {
        match(token);
    }
    else
    {
        printf("error 函数参数有误");
    }

    newtemp=newlnodeexp(idk);
    match(ID);
    temp->child[0]=newtemp;

    while(token==DOUH) /// 不止一个参数时
    {
        match(DOUH);
        temp1=newlnodestmt(defineparak);

        if(isdtype())
        {
            match(token);
        }
        else
        {
            printf("error ");
        }

        newtemp=newlnodeexp(idk);
        match(ID);
        temp1->child[0]=newtemp;
        temp->sibling=temp1;  ///出过bug 注意这里指针的变换
        temp=temp1;
    }

    if(token!=R_XI)
    {
        printf("error !  括号不匹配！！");
    }

    return t;
}

/**
*申请一条算式结构expk的节点，语句内容的类型如下
*opk(+ - * / 还有一些关系操作符),constk,idk
**/
///操作符，常量，标识符（变量名，函数名（idk））
treenode *newlnodeexp(Expkind exp)
{
    ///初始化
    treenode *t=(treenode *)malloc(sizeof(treenode));
    t->nodekind=expk;
    t->kind.exp=exp;
    t->lineno=count_row();
    int i;
    for(i=0; i<4; i++)
    {
        t->child[i]=NULL;
    }

    t->sibling=NULL;

    ///如果为操作符
    if(exp==opk)
    {
        t->attr.op=token;
    }

    ///如果为常量
    if(exp==constk)
    {
        t->attr.val=token_table[ptokenno-1].numval;
    }


    ///如果为变量名、或函数名（标识符）
    if(exp==idk&&token_table[ptokenno].tokenval!=DIAN)
    {
        strcpy(t->attr.name,token_table[ptokenno-1].stringval);
    }


    ///后期加进来的 为struct类型服务
    if(exp=idk&&token_table[ptokenno].tokenval==DIAN)
    {
        char struct_temp1[100]= {'\0'};
        //char struct_temp2[100]= {'\0'};
        char struct_temp3[100]= {'\0'};
        char struct_temp4[100]= {'\0'};
        strcpy(struct_temp1,token_table[ptokenno-1].stringval);
        //strcpy(struct_temp2,token_table[ptokenno].stringval);
        strcpy(struct_temp3,token_table[ptokenno+1].stringval);
        int i=0,j=0;
        while(struct_temp1[j]!='\0')
        {
            struct_temp4[i]=struct_temp1[j];
            i++;
            j++;
        }
        struct_temp4[i]='.';
        i++;
        j=0;
        while(struct_temp3[j]!='\0')
        {
            struct_temp4[i]=struct_temp3[j];
            i++;
            j++;
        }
        strcpy(t->attr.name,struct_temp4);
    }
    return t;
}


/***********************************************************
* 功能:以大括号开始的一个大结构
        比如： for if  while  main fun
**********************************************************/
treenode *structure_stmt()
{
    treenode *t=NULL;
    treenode *temp=NULL;
    treenode *newtemp;
    int k=0;
    match(L_DA);
    while(isin_first_instmt_sequence())/**当前Token是否在instmt-sequence的定义集合里**/
    {
        k++;
        newtemp=instmt_sequence();  ///开始各自的语句

        if(temp!=NULL)
        {
            temp->sibling=newtemp;
        }

        temp=newtemp;
        if(k==1)
        {
            t=temp; ///把头结点固定住 以便返回
        }
    }
    match(R_DA);
    return t;
}
///为了判断是不是这些语句比如 for if while 定义 返回
int isin_first_instmt_sequence()
{
    tokentype first_instmt[20]= {FOR,WHILE,IF,INT,FLOAT,
                                 CHAR,DOUBLE,ID,RETURN,
                                 STRUCT,SWITCH,CASE,BREAK,
                                    DEFAULT};
    int i=0;
    for(i=0; i<20; i++)
    {
        if(token==first_instmt[i])
            return 1;
    }
    return 0;
}


/**FOR IF WHILE 定义 PRINTF SCANF语句 RETURN 等等**/
treenode *instmt_sequence()
{
    treenode *t=NULL;

    switch (token)
    {
    /**for循环 !!**/
    case FOR :
    {
        t=for_stmt();
        break;
    }
    /**IF语句 !!**/
    case IF :
    {
        t=if_stmt();
        break;
    }
    /**while语句 !!**/
    case WHILE :
    {
        t=while_stmt();
        break;
    }
    /**struct语句 !!**/
    case SWITCH:
    {
        t=switch_stmt();
        break;
    }
     /**case语句 !!**/
    case CASE:
    {
       t=case_stmt();
       break;
    }
     /**case语句 !!**/
    case BREAK:
    {
       t=break_stmt();
       break;
    }
    case DEFAULT:
    {
       t=default_stmt();
       break;
    }
    default:
    {
        t=statement(); ///  普通语句有很多--函数调用，定义，赋值 return等等
        if(token!=FENH)
        {
            printf("missing ';' %d ",count_row());///如果为定义语句；打印；输入语句，则后面判断有没有分号
        }
        else
        {
            match(FENH);
        }
        break;
    }
    }
    return t;
}


///for语句
treenode *for_stmt()
{
    treenode *t=NULL;
    treenode *temp;

    t=newlnodestmt(fork);

    match(FOR);
    match(L_XI);

    if(token!=FENH)
    {
        if(token==INT)
        {
            t->child[0]=definepara_stmt();
        }
        else
        {
            t->child[0]=exp2();
        }
    }

    match(FENH);
    if(token!=FENH)
    {
        if(token_table[ptokenno].tokenval==EQUAL)
        {
            printf("warning!! 不能为赋值等式");
        }
        else
        {
            t->child[1]=exp2();
        }
    }

    match(FENH);

    if(token!=R_XI)
    {
        t->child[2]=exp2();
    }

    match(R_XI);

    if(token==FENH)
    {
        match(FENH);
        return t;
    }

    t->child[3]=structure_stmt();
    return t;
}

int ifelse_tag=0;
treenode* if_stmt()
{
    treenode *t;
    t=newlnodestmt(ifk);
    match(IF);
    match(L_XI);
    if(token_table[ptokenno].tokenval==EQUAL&&token_table[ptokenno+1].tokenval!=EQUAL)
    {
        printf("warning");
    }
    t->child[0]=BUER_stmt();
    match(R_XI);
    t->child[1]=structure_stmt();

    if(token==ELSE&&token_table[ptokenno].tokenval==IF)
    {
        match(ELSE);
        t->child[2]=if_stmt();
        ifelse_tag=1;
    }
    if(token==ELSE&&token_table[ptokenno].tokenval!=IF)
    {
        match(ELSE);
        t->child[2]=structure_stmt();
    }

    return t;
}
treenode *BUER_stmt()
{
    treenode *t;
    treenode *t1;
    treenode *t2;
    if(token_table[ptokenno+2].tokenval==ADE || token_table[ptokenno+3].tokenval==ADE||
       token_table[ptokenno+2].tokenval==HUO ||token_table[ptokenno+3].tokenval==HUO)
    {
    }
    else
    {
        t=exp2();
    }
}
///主要用于FOR ,IF while ,你不知道是赋值还是关系运算。
treenode *exp2()
{
    treenode *t;
    if(token==ID&&token_table[ptokenno].tokenval==EQUAL)
    {
        t=assign_stmt();
    }
    else
    {
        t=exp();
    }
    return t;
}

/***********************************************************
 * 功能:	while语句
 **********************************************************/
treenode *while_stmt()
{
    treenode *t=newlnodestmt(whilek);
    match(WHILE);
    match(L_XI);
    t->child[0]=exp2();
    match(R_XI);
    if(token==L_DA)
        t->child[1]=structure_stmt();
    return t;///!!!这个bug 找了好久呀！！！
}
/***********************************************************
 * 功能:	switch语句
 **********************************************************/
treenode *switch_stmt()
{
    treenode *t=newlnodestmt(switchk);
    match(SWITCH);
    match(L_XI);
    t->child[0]=exp2();
    match(R_XI);
    if(token==L_DA)
    {
        t->child[1]=structure_stmt();
    }
    return t;
}
///用于case语句段
treenode *case_stmt()
{
    treenode *t=newlnodestmt(casek);
    match(CASE);
    treenode *j=newlnodeexp(idk);
    t->child[0]=j;
    match(ID);
    match(MAOH);
    if(token==L_DA)
    {
        t->child[1]=structure_stmt();
    }
    return t;
}
///用于break语句
treenode *break_stmt()
{
    treenode *t=newlnodestmt(breakk);
    match(BREAK);
    match(FENH);
    return t;
}
///用于default语句
treenode *default_stmt()
{
    treenode *t=newlnodestmt(defaultk);
    match(DEFAULT);
    match(MAOH);
    if(token==L_DA)
    {
        t->child[0]=structure_stmt();
    }
    return t;
}
treenode *statement()
{
    treenode *t=NULL;
    /**如果是dtype，则为定义语句**/
    if(isdtype())
    {
        t=definepara_stmt();
    }
    else if(token==RETURN)
    {
        t=newlnodestmt(returnk);
        match(RETURN);
        t->child[0]=exp();
    }
    else if(token==STRUCT)
    {
        t=definepara_stmt();
    }
    else if(token==ID)  /**函数调用,数组定义，变量的赋值**/
    {
        int tag_array=array_judge(ptokenno);

        if(token_table[ptokenno].tokenval==EQUAL||tag_array==1)
        {
            t=assign_stmt();///数组或变量赋值(变量可能为函数调用的结果)
        }
        else if(token_table[ptokenno].tokenval==L_XI)
        {
            t=fun_apply();  /// 函数调用
        }
        else if(token_table[ptokenno].tokenval==DIAN&&token_table[ptokenno+2].tokenval==EQUAL)
        {
            t=assign_stmt();
        }
        else ///if()待写  感觉不用条件
        {
            t=exp();
        }

    }
    return t;
}
/***********************************************************
 * 功能:	函数调用  fun(a,b)
 **********************************************************/
treenode *fun_apply()
{
    treenode *t;
    t=newlnodestmt(funck);
    treenode *temp1=newlnodeexp(idk);
    match(ID);
    match(L_XI);
    treenode *temp2=input_para_list();
    t->child[0]=temp1;
    t->child[1]=temp2;
    match(R_XI);
    return t;
}

///参数列表 服务于函数调用里的参数 比如fun(1,2)
treenode *input_para_list()
{
    treenode *t;
    treenode *temp;
    temp=factor();
    t=temp;
    while(token==DOUH)
    {
        match(DOUH);
        temp->sibling=factor();
        temp=temp->sibling;
    }
    return t;
}
int array_judge(int p)
{
    if(token_table[p].tokenval==L_ZH)
    {
        int i;
        for(i=p+1; i<1000; i++) ///有bug！！。。。
        {
            if(token_table[i].tokenval==R_ZH)
            {
                return 1;
            }
        }
        return -1;
    }
    return -1;
}

/**判断当前记号是否是变量或函数返回值（除VOID以外）**/
int isdtype()
{
    int i=0;
    for(; i<4; i++)
    {
        if(token==dtype[i])
            return 1;
    }
    return 0;
}


/**定义语句**/
treenode *definepara_stmt()
{
    treenode *t=NULL;
    t=newlnodestmt(defineparak);

    if (isdtype())
    {
        match(token);//dtype
    }
    else if(token==STRUCT)
    {
        match(token);
        match(ID);  ///把整个struct XX match掉
    }

    else
        printf("arguement has no type", count_row());

    t->child[0]=define_assign();
    return t;
}


/**变量赋值定义  数组的定义,初始化，初始化和定义的同时进行**/
treenode *define_assign()
{
    treenode *t=NULL;
    treenode *temp=NULL;
    treenode *newtemp;
    int k=0;


    if(token_table[ptokenno].tokenval==FENH)
    {
        k++;
        newtemp=newlnodeexp(idk);
        match(ID);
        //match(FENH);
        return t=newtemp;
    }
    /**支持变量定义初始化，连续定义等**/
    while (token_table[ptokenno].tokenval == EQUAL || token_table[ptokenno].tokenval == DOUH
            || token_table[ptokenno].tokenval == FENH||token_table[ptokenno].tokenval==L_ZH)

    {
        switch (token_table[ptokenno].tokenval)
        {

        case EQUAL:  /**赋值语句**/
        {
            k++;
            newtemp=assign_stmt();
            if(temp!=NULL)
            {
                temp->sibling=newtemp;
            }
            temp=newtemp;
            if(token==DOUH)
            {
                match(DOUH);
            }
            if(k==1)
            {
                t=temp;
            }
            break;
        }
        case DOUH:
        {
            k++;
            newtemp=newlnodeexp(idk);
            match(ID);
            match(DOUH);
            if(temp!=NULL)
            {
                temp->sibling=newtemp;
            }
            temp=newtemp;
            if(k==1)
            {
                t=temp;
            }
            break;
        }

        case FENH:
        {
            k++;
            newtemp=newlnodeexp(idk);
            match(ID);
            if(temp!=NULL)
            {
                temp->sibling=newtemp;
            }
            temp=newtemp;
            if(k==1)
            {
                t=temp;
            }
            break;
        }
        case L_ZH:  ///数组部分！！
        {
            k++;

            newtemp=newlnodestmt(define_arrayk);
            treenode *t1=newlnodeexp(idk);
            match(ID);
            newtemp->child[0]=t1;

            match(L_ZH);
            if(token!=NUM&&token!=R_ZH)  ///一些语义分析
            {
                printf("error!!数组定义有误");
            }
            if(token==NUM)
            {
                treenode *t2=newlnodeexp(constk);
                match(NUM);
                newtemp->child[1]=t2;
            }
            match(R_ZH);
            if(temp!=NULL)
            {
                temp->sibling=newtemp;
            }
            temp=newtemp;
            if(token==DOUH)
            {
                match(DOUH);
            }
            if(k==1)
            {
                t=temp;
            }
            break;
        }
        default:
        {
            printf("there should be assignment statement or variable",count_row());
        }
        }
    }
    return t;
}


/**赋值语句  数组的赋值后期加进来！！！！**/
treenode *assign_stmt()
{
    treenode *t1;
    if(token_table[ptokenno].tokenval==L_ZH) ///数组赋值  形如a[1] a[2]
    {
        t1=newlnodeexp(cite_arrayk);
        treenode *t2=newlnodeexp(idk);
        match(ID);
        t1->child[0]=t2;
        match(L_ZH);
        t1->child[1]=exp2(); ///直接是factor里的数字结构了；
        match(R_ZH);
    }
    else if(token_table[ptokenno].tokenval==DIAN)  ///为struct结构专门定义
    {
        t1 = newlnodeexp(idk);
        match(ID);
        match(DIAN);
        match(ID);
    }
    else
    {
        t1=newlnodeexp(idk);
        match(ID);
    }
    match(EQUAL);///不需要将=匹配成op

    ///opk=EQUAL
    treenode *t=newlnodestmt(assignk);
    t->child[0]=t1;
    t->child[1]=exp();
    return t;
}


///  simple_exp conpare simple_exp   ||simple_exp
treenode *exp()
{
    treenode *t;
    treenode *temp;

    temp=simple_exp();
    ///简单表达式
    if(iscomparison_op())
    {
        t=newlnodeexp(opk);
        match(token);
        t->child[0]=temp;
        t->child[1]=simple_exp();
    }
    else
    {
        t=temp;
    }

    return t;
}

int iscomparison_op()
{
    tokentype compare_op[6]= {SMALLER,SMALLEREQU,
                              BIGGER,BIGGEREQU,
                              NOTEQU,IFEQU
                             } ;//< <= > >= != ==
    int i;
    for(i=0; i<6; i++)
    {
        if(token==compare_op[i])
        {
            return 1;
        }
    }
    return 0;///没有匹配上
}

///简单表达式 加减
treenode *simple_exp()
{
    treenode *t=NULL;
    treenode *newtemp;
    treenode *temp;

    temp=term();

    ///加法或减法语句
    while(token==PLUS||token==MINUS)
    {
        newtemp=newlnodeexp(opk);
        match(token);
        ///子节点分别为左右两个term
        newtemp->child[0]=temp;
        newtemp->child[1]=term();
        temp=newtemp;
    }
    t=temp;
    return t;
}

///主要是乘除
treenode *term()
{
    treenode *t=NULL;
    treenode *temp;
    treenode *newtemp;

    temp=factor();

    while(token==TIMES||token==DIVIDE)
    {
        newtemp=newlnodeexp(opk);
        match(token);
        newtemp->child[0]=temp;
        newtemp->child[1]=factor();
        temp=newtemp;
    }

    return t=temp;
}

///最基本的单模块
treenode *factor()
{
    treenode *t=NULL;
    switch (token)
    {
    ///带括号的表达式
    case L_XI:
    {
        match(L_XI);
        t=exp();
        match(R_XI);
        break;
    }

    ///变量---可能为函数调用语句，可能为数组，可能为普通变量
    case ID:
    {
        if(token_table[ptokenno].tokenval==L_XI)  ///函数调用
        {
            t=fun_apply();
        }
        else if(token_table[ptokenno].tokenval==L_ZH) ///数组
        {
            t=newlnodeexp(cite_arrayk);
            treenode *temp1=newlnodeexp(idk);
            match(ID);
            match(L_ZH);
            treenode *temp2=exp2() ;  ///可缩小范围factor 即可;
            t->child[0]=temp1;
            t->child[1]=temp2;
            match(L_ZH);
        }
        else   ///普通变量
        {
            t = newlnodeexp(idk);
            match(ID);
        }
        break;
    }

    ///数字
    case NUM:
    {
        t=newlnodeexp(constk);
        t->attr.val=token_table[ptokenno-1].numval;
        match(NUM);
        break;
    }
    }
    return t;
}









/** ** **** **** *****   *******       输出抽象语法树的过程  *******   *****  ******   *****/

void parse_printtree()
{
    treenode *t=tree_gen;
    printtree(t);
}


///树
void printtree(treenode *lnode)
{
    int i;
    treenode *pnode;
    int TAG=printtree_t;
    for(i=0; i<TAG; i++)
        printf("        ");
    printnode(lnode);
    printf("\n");
    printtree_t++;
    for(i=0; i<4; i++)
        if(lnode->child[i]!=NULL)
        {
            pnode=lnode;
            lnode=lnode->child[i];
            printtree(lnode);
            printtree_t--;
            lnode=pnode;
        }
    if(lnode->sibling!=NULL)
    {
        lnode=lnode->sibling;
        printtree_t=TAG;
        printtree(lnode);
    }
}
///打印结点
void printnode(treenode *lnode)
{
    if(lnode->nodekind==expk) ///表达式结点
    {

        switch (lnode->kind.exp)
        {
        case idk:
        {
            printf("id=  [%s]",lnode->attr.name);
            break;
        }
        case opk:
        {
            token=lnode->attr.op;
            printf("op=  [%s]",tokenstring());///tokenstring()不需要参数
            break;
        }
        case constk:
        {
            printf("num=  [%g]",lnode->attr.val);
            break;
        }
        case cite_arrayk:
        {
            printf("array_id'[%s]'",lnode->child[0]->attr.name);
            break;
        }

        }
    }
    else if(lnode->nodekind==stmtk) ///stmtk  //函数结点
    {
        switch(lnode->kind.stmt)
        {
        case ifk:
        {
            printf("[if]");
            break;
        }
        case fork:
        {
            printf("[for]");
            break;
        }
        case whilek:
        {
            printf("[while]");
            break;
        }

        case returnk:
        {
            printf("[return]");
            break;
        }
        case assignk:
        {
            printf("assign:");
            break;
        }
        case funck:
        {
            token=lnode->dtype;
            if(lnode->child[2]!=NULL)
            {
                printf("function : type is");
                printf(" [%s]",tokenstring());
            }
            else
            {
                printf("callfunc :");
            }

            break;
        }
        case defineparak:
        {
            token=lnode->dtype;
            printf("definepara : ");
            printf("[%s]",tokenstring());
            break;
        }
        case maink:
        {
            printf("[main]");
            break;
        }
        case define_arrayk:
        {
            printf("definearray");
            break;
        }
        case structk:
        {
            printf("[struct]");
            break;
        }
        case casek:
        {
            printf("[case]");
            break;
        }
        case breakk:
        {
            printf("[break]");
            break;
        }
        case switchk:
        {
            printf("[switch]");
            break;
        }

        case defaultk:
        {
            printf("[default]");
            break;
        }
        default:
            printf("error1!!!");
        }
    }
    else
    {
        printf("节点有误！！");
    }
}
