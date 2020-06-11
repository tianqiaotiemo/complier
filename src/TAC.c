#include"global.h"
#include"TAC.h"
char *opkinds_string(opkinds op);///这个在优化还要用，所以不能私有化
static void start_tac(treenode* lnode);
static char *deal_expk(treenode* lnode);  ///处理算术表达式
static void MAIN_K(treenode *lnode);      ///处理main
static void DEFINEPARA_K(treenode *lnode); ///处理定义结点
static void ASSIGN_K(treenode *lnode);    ///处理赋值结点
static void RETURN_K(treenode *lnode);      ///处理return结点
static void IF_K(treenode *lnode);          ///处理if结点
static void WHILE_K(treenode *lnode);       ///处理while结点
static void FOR_K(treenode *lnode);         ///处理for结点
static void FUNC_K(treenode *lnode);        ///处理函数调用结点
static void get_tac(opkinds op,char a[],char b[],char c[]); ///产生四元式
static char * newlabel();     ///申请跳转label
static char * newtemp();      ///申请个临时寄存器
fourvarcode *tac_temp;     ///四元式链表的一个变量指针
fourvarcode *tac_head;     ///四元式的链表头指针
static int newtemp_no=0;    ///中间变量的寄存器的计数器
static int newlabel_no=0;  ///跳转标志的计数器
static char adr[10]= {'\0'};
static char empty[10]= "\0";
void  tac()
{
    tac_head=(fourvarcode*)malloc(sizeof(fourvarcode));
    tac_temp=tac_head;
    treenode *lnode=tree_gen;

    start_tac(lnode);

	print_tac();
}


void start_tac(treenode* lnode)
{
    while(lnode!=NULL)
    {
        if(lnode->nodekind==stmtk) ///是语句节点(比如：if while main  等等)
        {
            if(lnode->kind.stmt==maink)
            {
                MAIN_K(lnode);
            }
            if(lnode->kind.stmt==defineparak)
            {
                DEFINEPARA_K(lnode);
            }
            if(lnode->kind.stmt==assignk)
            {
                ASSIGN_K(lnode);
            }
            if(lnode->kind.stmt==returnk)
            {
                RETURN_K(lnode);
            }
            if(lnode->kind.stmt==ifk)
            {
                IF_K(lnode);
            }
            if(lnode->kind.stmt==whilek)
            {
                WHILE_K(lnode);
            }
            if(lnode->kind.stmt==fork)
            {
                FOR_K(lnode);
            }
            if(lnode->kind.stmt==funck)
            {
                FUNC_K(lnode);
            }
        }
        lnode=lnode->sibling;
    }
}


///处理main节点
void MAIN_K(treenode *lnode)
{

    char a[10];
    a[0]='0';
    a[1]='\0';
    get_tac(mainfuns,adr,adr,adr); ///加了main开始的标志

    if(lnode->child[1]!=NULL)  ///main函数中有参数
    {
        token=lnode->child[1]->dtype;
        strcpy(a,tokenstring());
        get_tac(defineparas,a,lnode->child[1]->child[0]->attr.name,adr);
        treenode *t;
        t=lnode->child[1]->sibling;

        while(t!=NULL)
        {
            token=t->dtype;
            strcpy(a,tokenstring());
            get_tac(defineparas,a,t->child[0]->attr.name,adr);
            t=t->sibling;
        }
    }
    start_tac(lnode->child[2]);  ///递归--进入main结构
    get_tac(mainfuns_end,adr,adr,adr);
}

///处理define 语句
void DEFINEPARA_K(treenode *lnode)
{
    treenode *t;
    token=lnode->dtype;
    char a[10];
    strcpy(a,tokenstring()); ///存储变量类型
    t=lnode->child[0];
    while(t!=NULL)///后期考虑数组!!!!!!
    {
        if(t->kind.stmt==assignk)  ///定义+变量赋值
        {
            get_tac(defineparas,a,t->child[0]->attr.name,adr);
            start_tac(t);
        }

        else if(t->kind.stmt==define_arrayk)  ///数组！！未完成！！
        {
            char b[10];
            sprintf(b,"%f",t->child[1]->attr.val);
        }

        else  ///变量定义
        {
            get_tac(defineparas,a,t->attr.name,adr);
        }

        t=t->sibling;  ///可能有连续定义--所以要搜寻一下它的兄弟节点
    }
}


///处理赋值节点
void ASSIGN_K(treenode *lnode)
{
    char t1[10],t2[10];
    strcpy(t1,newtemp());
    strcpy(t2,deal_expk(lnode->child[1]));  ///一定要处理assign等式右边的
    get_tac(asns,t2,t1,adr);
    if(lnode->child[0]->kind.exp==idk)
    {
        get_tac(asns,t1,lnode->child[0]->attr.name,adr); ///将值给id
    }
   /* else ///if(lnode->child[0]->kind.exp==cite_arrayk 是数组的话--
    {
        char t3[10];
        strcpy(t3,deal_expk(lnode->child[0]));
        get_tac(asns,t1,t3,adr);
    }*/
}

///处理return节点
void RETURN_K(treenode *lnode)
{
    char t1[10],t2[10];
    if(lnode->child[0]!=NULL)
    {
        strcpy(t1,newtemp());
        strcpy(t2,deal_expk(lnode->child[0]));
        get_tac(asns,t2,t1,adr);
        get_tac(returns,t1,adr,adr);
    }
    else
    {
        get_tac(returns,adr,adr,adr);
    }
}

///处理 if节点
void IF_K(treenode *lnode)
{
    get_tac(ifstarts,adr,adr,adr);  ///if的开始四元式
    char t1[10],t2[10],t3[10];  ///寄存器数组
    char L1[10],L2[10];      ///跳转临时数组
    char a[10];

    strcpy(t1,newtemp()); ///申请寄存器
    if(lnode->child[0]->kind.exp==opk)///一定要为关系表达式，不然就有问题 opk为关系操作
    {
        strcpy(t2,deal_expk(lnode->child[0]));
    }

    get_tac(asns,t2,t1,adr);
    strcpy(L1,newlabel());///申请个跳走lable

    get_tac(if_fs,t1,L1,adr);  ///如果布尔表达式为假，直接跳走
    start_tac(lnode->child[1]);///进去if的执行结构

    strcpy(L2,newlabel());  ///if执行完就跳出循环了
    get_tac(jumps,L2,adr,adr);

    get_tac(labss,L1,adr,adr);///if错了之后 跳过来的位置
    start_tac(lnode->child[2]);///递归解决else if (else)结构

    get_tac(labss,L2,adr,adr);
    get_tac(ifends,adr,adr,adr);
}

void WHILE_K(treenode *lnode)
{
    char t1[10],t2[10],t3[10];
    char L1[10],L2[10],L3[10];
    strcpy(L1,newlabel());
    get_tac(labss,L1,adr,adr); ///不同于if语句 while是符合条件则一直循环，所以提前设置好跳转位置
    get_tac(whilestarts,adr,adr,adr);

    strcpy(t1,newtemp());

    if(lnode->child[0]->kind.exp==opk)  ///while里面都应该为布尔表达式
    {
        strcpy(t2,deal_expk(lnode->child[0]));
    }
    get_tac(asns,t2,t1,adr);

    strcpy(L2,newlabel());
    get_tac(if_fs,t1,L1,adr);

    start_tac(lnode->child[1]); ///进入while结构进行递归

    get_tac(whileends,adr,adr,adr);
    get_tac(jumps,L1,adr,adr); ///继续执行

    get_tac(labss,L2,adr,adr);
}


void FOR_K(treenode *lnode)
{
    char t1[10],t2[10],t3[10];
    char L1[10],L2[10],L3[10];
    start_tac(lnode->child[0]);  ///第一个是赋值语句，递归解决

    strcpy(L1,newlabel());
    get_tac(labss,L1,adr,adr);
    get_tac(forstarts,adr,adr,adr);

    if(lnode->child[1]!=NULL)
    {
        strcpy(t1,newtemp());
        if(lnode->child[1]->kind.exp==opk) ///中间一部分布尔表达式
        {
            strcpy(t2,deal_expk(lnode->child[1]));
        }
        get_tac(asns,t2,t1,adr);
    }

    strcpy(L2,newlabel());
    get_tac(for_cmp,t1,L2,adr);

    start_tac(lnode->child[3]); ///for循环要先执行循环，再执行for中的第三条语句
    start_tac(lnode->child[2]);

    get_tac(forends,adr,adr,adr);
    get_tac(jumps,L1,adr,adr);
    get_tac(labss,L2,adr,adr);
}



void FUNC_K(treenode *lnode)
{
    if(lnode->child[2]==NULL) ///为函数调用
    {
        char t1[10],t2[10];
        char para_num[10];

        if(lnode->child[1]!=NULL) ///有参数
        {
            sprintf(para_num,"%d",count_para(lnode->child[1]));
        }
        else
        {
            para_num[0]='0';
            para_num[1]='\0'; ///做个标记,参数为空
        }
        get_tac(begin_args,para_num,adr,adr);

        if(lnode->child[1]!=NULL)
        {
            strcpy(t1,newtemp());
            strcpy(t2,deal_expk(lnode->child[1]));
            get_tac(asns,t2,t1,adr);
            get_tac(args,t1,adr,adr);
            treenode *t;
            t=lnode->child[1]->sibling;
            while(t!=NULL)
            {
                strcpy(t1,newtemp());
                strcpy(t2,deal_expk(t));
                get_tac(asns,t2,t1,adr);
                get_tac(args,t1,adr,adr);
                t=t->sibling;
            }
        }
        get_tac(callfuns,lnode->child[0]->attr.name,adr,adr);
    }
    else  ///自定义的函数结构
    {
        char a[10];
        if(lnode->child[1]!=NULL)
        {
            sprintf(a,"%d",count_para(lnode->child[1]));
        }
        else
        {
            a[0]='0';
            a[1]='\0';
        }

        get_tac(definefuns,lnode->child[0]->attr.name,a,adr);
        if(lnode->child[1]!=NULL)  ///解决fun(括号里的函数定义)
        {
            token=lnode->child[1]->dtype;
            strcpy(a,tokenstring());
            get_tac(defineparas,a,lnode->child[1]->child[0]->attr.name,adr);
            treenode *t;
            t=lnode->child[1]->sibling;
            while(t!=NULL)
            {
                token=t->dtype;
                strcpy(a,tokenstring());
                get_tac(defineparas,a,t->child[0]->attr.name,adr);
                t=t->sibling;
            }
        }
        start_tac(lnode->child[2]);
        get_tac(definefuns_end,lnode->child[0]->attr.name,adr,adr);
    }
}


void get_tac(opkinds op,char a[],char b[],char c[])
{
    fourvarcode* t=NULL;
    t=(fourvarcode*)malloc(sizeof(fourvarcode));
    t->op=op;

    if(a[0]=='\0')
    {
        t->addr1.kind=emptys;
        strcpy(t->addr1.name,"\0");
    }
    else if((a[0]>='a'&&a[0]<='z')||(a[0]>='A'&&a[0]<='Z'))
    {
        t->addr1.kind=strings;
        strcpy(t->addr1.name,a);
    }
    else
    {
        t->addr1.kind=consts;
        strcpy(t->addr1.name,a);
    }


    if(b[0]=='\0')
    {
        t->addr2.kind=emptys;
        strcpy(t->addr2.name,"\0");
    }
    else if((b[0]>='a'&&b[0]<='z')||(b[0]>='A'&&b[0]<='Z'))
    {
        t->addr2.kind=strings;
        strcpy(t->addr2.name,b);
    }
    else
    {
        t->addr2.kind=consts;
        strcpy(t->addr2.name,b);
    }


    if(c[0]=='\0')
    {
        t->addr3.kind=emptys;
        strcpy(t->addr3.name,"\0");
    }
    else if((c[0]>='a'&&c[0]<='z')||(c[0]>='A'&&c[0]<='Z'))
    {
        t->addr3.kind=strings;
        strcpy(t->addr3.name,c);
    }
    else
    {
        t->addr3.kind=consts;
        strcpy(t->addr3.name,c);
    }

 /*printf("(");
    printf("%s ,",opkinds_string(t->op));
    if(t->addr1.kind==emptys)
        printf("_ ,");
    else
        printf("%s ,",t->addr1.name);

    if(t->addr2.kind==emptys)
        printf("_ ,");
    else
        printf("%s ,",t->addr2.name);

    if(t->addr3.kind==emptys)
        printf("_ ,");
    else
        printf("%s ,",t->addr3.name);
    printf(")");
    printf("\n");
*/
    t->next=NULL;
    tac_temp->next=t;
    tac_temp=t;

}
///中间变量，在汇编中是存在寄存器中
char *newtemp()
{
    char s[10];
    sprintf(s,"t#%d",newtemp_no);
    newtemp_no++;
    return s;
}

char* deal_expk(treenode *lnode)///专门处理expk的式子
{
    char empty[10];
    empty[0]='\0';
    while(lnode!=NULL)
    {
        switch(lnode->kind.exp)
        {
        case opk:
        {
            char t2[10],t3[10],t4[10],a[10],b[10];
            strcpy(t2,newtemp());
            strcpy(t3,newtemp());

            if(lnode->child[1]->kind.exp==constk)
            {
                char c[10];
                sprintf(c,"%d",lnode->child[1]->attr.val);
                if((lnode->attr.op==PLUS||lnode->attr.op==MINUS)&&strcmp(c,"1")==0)
                {
                    if (lnode->child[0]->sibling!=NULL)///如果只是a+1的运算 先不考虑
                        printf("error");
                }
            }
            strcpy(a,deal_expk(lnode->child[0]));
            strcpy(b,deal_expk(lnode->child[1]));
            switch (lnode->attr.op)
            {
            case PLUS:///+
            {
                get_tac(asns,a,t2,empty);
                    ///表达式形如 a=b+7,需要从内存中取数
                if(b[1]!='#')
                {
                    get_tac(asns,b,t3,empty);
                    strcpy(t4,newtemp());
                    get_tac(adds,t2,t3,t4);
                }
                else   ///不需要从内存中取数
                {
                    strcpy(t4,newtemp());
                    get_tac(adds,t2,b,t4);
                }
                return t4;
            }

            case MINUS:///-
            {
                get_tac(asns,a,t2,empty);
                if(b[1]!='#')
                {
                    get_tac(asns,b,t3,empty);
                    strcpy(t4,newtemp());
                    get_tac(minus,t2,t3,t4);
                }
                else
                {
                    strcpy(t4,newtemp());
                    get_tac(minus,t2,b,t4);
                }
                return t4;
            }

            case TIMES: ///乘
            {
                get_tac(asns,a,t2,empty); //取a值到t2
                get_tac(asns,b,t3,empty);//取b值到t3
                strcpy(t4,newtemp()); //生成一个新的符号t4
                get_tac(muls,t2,t3,t4);// t4=t2*t3
                return t4;
            }
            case DIVIDE:  ///除
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(divs,t2,t3,t4);
                return t4;
            }
            case MODE:  ///取模
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(mods,t2,t3,t4);
                return t4;
            }
            case NOTEQU: ///!=
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(not_eqs,t2,t3,t4);
                return t4;
            }
            case IFEQU:///==
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(if_eqs,t2,t3,t4);
                return t4;
            }
            case SMALLER: ///<
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(smas,t2,t3,t4);
                return t4;
            }
            case SMALLEREQU:///<=
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(sma_eqs,t2,t3,t4);
                return t4;
            }
            case BIGGER:///>
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(bigs,t2,t3,t4);
                return t4;
            }
            case BIGGEREQU:///>=
            {
                get_tac(asns,a,t2,empty);
                get_tac(asns,b,t3,empty);
                strcpy(t4,newtemp());
                get_tac(big_eqs,t2,t3,t4);
                return t4;
            }

            }
        }
        case constk:///为常量
        {
            char t6[10];
            sprintf(t6,"%g",lnode->attr.val);
            return t6;
        }
        case idk:  ///如果为变量
        {

            return lnode->attr.name;
        }
        case funck:///函数调用
        {
            char t5[10],t6[10],a[10];
            if(lnode->child[1]!=NULL)
            {
                sprintf(a,"%d",count_para(lnode->child[1]));
            }
            else
            {
               a[0]='0',a[1]='\0';
            }
            get_tac(begin_args,a,empty,empty);
            if(lnode->child[1]!=NULL)
            {
                strcpy(t5,newtemp());
                strcpy(t6,deal_expk(lnode->child[1]));///没有走t->sibling
                get_tac(asns,t6,t5,empty);
                get_tac(args,t5,empty,empty);
                treenode *t;
                t=lnode->child[1]->sibling;
                while(t!=NULL)
                {
                    strcpy(t5,newtemp());
                    strcpy(t6,deal_expk(t));
                    get_tac(asns,t6,t5,empty);
                    get_tac(args,t5,empty,empty);
                    t=t->sibling;
                }
            }
            get_tac(callfuns,lnode->child[0]->attr.name,empty,empty);

            char t7[10];
            get_tac(funreturns,lnode->child[0]->attr.name,empty,empty);
            strcpy(t7,newtemp());
            return t7;

        }
        }
    }
}
char *newlabel()
{
    char s[10];
    sprintf(s,"L%d",newlabel_no);
    newlabel_no++;
    return s;
}

int count_para(treenode *lnode)
{
    treenode *t=lnode->sibling;
    int count=1;
    while(t!=NULL)
    {
        count++;
        t=t->sibling;
    }
    return count;
}



/*************************输出四元式*************************/

void print_tac()
{
	fourvarcode* t=tac_head->next;
	while(t!=NULL)
	{
	    if(t->op==ifstarts||t->op==ifends||t->op==mainfuns_end||t->op==mainfuns||t->op==forstarts||t->op==forends||t->op==defineparas
            ||t->op==whilestarts ||t->op==whileends)
        {
            t=t->next;
        }
        else
        {
	    printf("(");
		printf("%s ,",opkinds_string(t->op));
		if(t->addr1.kind==emptys)
			printf("_ ,");
		else
			printf("%s ,",t->addr1.name);

		if(t->addr2.kind==emptys)
			printf("_ ,");
		else
			printf("%s ,",t->addr2.name);

		if(t->addr3.kind==emptys)
			printf("_ ,");
		else
			printf("%s ,",t->addr3.name);

		t=t->next;
		printf(")");
		printf("\n");
        }
	}
}




///转换为字符型，符号表
char *opkinds_string(opkinds op)
{
    switch (op)
    {
    case (if_fs):
        return "if_f"; ///if_false
    case (asns):
        return "=";  ///赋值
    case (labss):
        return "lab";
    case (muls):
        return "*";
    case (adds):
        return "+";
    case (minus):
        return "-";
    case (callfuns):
        return "callfun";
    case (definefuns):
        return "definefun";
    case (mainfuns):
        return "mainfun";
    case (divs):
        return "/";
    case (if_eqs):
        return "==";
    case (not_eqs):
        return "!=";
    case (bigs):
        return ">";
    case (big_eqs):
        return ">=";
    case (smas):
        return "<";
    case (sma_eqs):
        return "<=";
    case (returns):
        return "return";
    case (jumps):
        return "jump";
    case (args):
        return "arg";
    case (begin_args):
        return "begin_arg";
    case(mainfuns_end):
        return "mainfun_end";
    case(definefuns_end):
        return "definefun_end";
    case(funreturns):
        return "funreturn";
    case(defineparas):
        return "definepara";
    case(ifstarts):
        return "ifstart";
    case(ifends):
        return "ifend";
    case(whilestarts):
        return "whilestart";
    case(whileends):
        return "whileend";
    case(forstarts):
        return "forstart";
    case(forends):
        return "forend";
    case(add_nexts):
        return "add_next";
    case(minus_nexts):
        return "minus_next";
    case(for_cmp):
        return "for_cmp";
    default:
        printf("匹配有误");
    }

}
