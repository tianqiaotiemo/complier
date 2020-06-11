#include"global.h"
#include"asm.h"
#include"symtab.h"
void flocal_table(char type[],char name[],char num[],int n);
static void build_asm(fourvarcode* t);
static void find_function(fourvarcode* tac_head);
static void build_define(fourvarcode* t);
static void build_assign(fourvarcode* t);
static void build_label(fourvarcode* t);
static void build_jump(fourvarcode* t);
static void build_return(fourvarcode* t);
static void build_muls(fourvarcode* t); ///乘
static void build_divs(fourvarcode* t);  ///除
static void build_adds(fourvarcode* t);
static void build_minu(fourvarcode* t);
static void build_mainfuns(fourvarcode* t);
static void build_mainfuns_end(fourvarcode* t);
static void build_smas(fourvarcode* t);///<
static void build_smas_eqs(fourvarcode* t);///<=
static void build_big_equs(fourvarcode* t);
static void  build_bigs(fourvarcode* t);
static void build_eqs_eqs(fourvarcode* t);
static void build_not_eqs(fourvarcode* t);
void fglocal_table(char type[],char name[],char num[],int n);
symboltable *lsptempg,*lsptempl;///用于构造符号表的临时变量
fourvarcode* ltag;
char empty[]="\0";
int AX_FLAG=0,BX_FLAG=0,CX_FLAG=0,DX_FLAG=0,SI_FLAG=0;///通用寄存器
char assign_table[20][20];
int num=0;
symboltable *local_table;
int stackn=0;
char function_table[20][20]; ///函数名
int function_num = 0; ///函数个数

void flocal_table(char type[],char name[],char num[],int n)
{
    symboltable *t;
    t = (symboltable *)malloc(sizeof(symboltable));
    t->next = NULL;
    strcpy(t->type, type);
    strcpy(t->name, name);
    t->location=n;
    lsptempl->next=t;
    lsptempl=t;

}
void fglocal_table(char type[],char name[],char num[],int n)
{
    symboltable *t;
    t = (symboltable *)malloc(sizeof(symboltable));
    t->next = NULL;
    strcpy(t->type, type);
    strcpy(t->name, name);
    t->location=n;
    lsptempg->next=t;
    lsptempg=t;

}

int searchlocal(char a[20])
{
    symboltable *t=local_table->next;

    while(t!=NULL)
    {


        if(strcmp(t->name,a)==0)
        {
            return t->location;
            break;
        }
        t=t->next;
    }
    return -1;
}
void registerfree(char b[10])
{
    if(strcmp(b,"AX")==0||strcmp(b,"EAX")==0)
    {
        AX_FLAG=0;
    }
    if(strcmp(b,"BX")==0||strcmp(b,"EBX")==0)
    {
        BX_FLAG=0;
    }
    if(strcmp(b,"CX")==0||strcmp(b,"ECX")==0)
    {
        CX_FLAG=0;
    }

}
void start_asm()
{
    printf(".MODEL SMALL\n");
    printf(".386\n");
    printf(".STACK 100H\n");
    printf(".DATA\n");
    printf(".CODE\n");
    printf(".STARTUP\n");
    find_function(tac_head);
    local_table=(symboltable *)malloc(sizeof(symboltable));
    lsptempl=local_table;
    local_table->next=NULL;

    fourvarcode* t=tac_head->next;
    if(t->op!=mainfuns);
    {
        while(t->op!=mainfuns)
        {
            t=t->next;
        }
    }

    fourvarcode* q=t->next;

    ///***先解决函数定义的问题***///
    while(q->op!=mainfuns_end)
    {
        if(q->op==defineparas)
        {
            build_asm(q);
        }
        q=q->next;
    }

    build_asm(t); ///此时t是mainstart
    printf("push bp\n");
    printf("mov bp,sp\n");
    printf("jmp alloc\n");
    printf("main:\n");

    t=t->next; ///没有结束  建立其他的汇编

    while(t->op!=mainfuns_end)
    {
        if(t->op!=defineparas)
        {
            build_asm(t);
        }
        t=t->next;
    }
    build_asm(t);
    printf("jmp over\n");
    int i;
    printf("alloc:\n");
    for(i=num; i>=1; i--)
    {
        if(strcmp(assign_table[i],empty)==0)
        {
            printf("MOV AX,0\n");
            printf("PUSH AX\n");
        }
        else ///有赋值
        {
            printf("MOV AX,%s\n",assign_table[i]);
            printf("PUSH AX\n");
        }
    }
    printf("jmp main\n");
    printf("over:\n");
    printf(".exit\n");
    printf("end\n");

}

/***********************************************************
* 功能:将自定义函数存储起来
**********************************************************/
void find_function(fourvarcode* tac_head)
{
	fourvarcode *t = tac_head->next;
	while (t!=NULL)
	{
		if (t->op == definefuns)
		{
			strcpy(function_table[function_num++], t->addr1.name);
		}
		t = t->next;
	}

}

/***********************************************************
* 功能:全局定义的变量
**********************************************************/
void define_para(fourvarcode* tac_head)
{
	fourvarcode *t=tac_head->next;
	int num=0;
	//如果不为函数定义或主函数
	if(t->op!=definefuns&&t->op!=mainfuns)
	{
		while(t->op!=definefuns && t->op!=mainfuns)
		{
             flocal_table(t->addr1.name,t->addr2.name,t->addr3.name,num+1);
			t=t->next;
			num++;
		}
	}
	registerfree("AX");
	registerfree("BX");
	registerfree("CX");
}

/***********************************************************
* 功能:开始根据op来生成汇编代码
**********************************************************/
void build_asm(fourvarcode* t)
{
    if(t->op==defineparas)
    {
        build_define(t);
        return;
    }
    if(t->op==asns)
    {
        build_assign(t);
        return ;
    }
    if(t->op==labss)
    {
        build_label(t);
        return ;
    }
    if(t->op==jumps)
    {
        build_jump(t);
        return ;
    }
    if(t->op==returns)
    {
        build_return(t);
        return ;
    }
    if(t->op==minus)//-
    {
        build_minu(t);
        return ;
    }
    if(t->op==divs)//÷
    {
        build_divs(t);
        return ;
    }
    if(t->op==adds)//+
    {
        build_adds(t);
        return ;
    }
    if(t->op==muls)//*
    {
        build_muls(t);
        return ;
    }
    if(t->op==mainfuns)
    {
        build_mainfuns(t);
        return ;
    }
    if(t->op==mainfuns_end)
    {
        build_mainfuns_end(t);
        return ;
    }
    if(t->op==smas)
    {
        build_smas(t);
        return ;
    }
    if(t->op==sma_eqs)
    {
        build_smas_eqs(t);
        return ;
    }
    if(t->op==bigs)
    {
        build_bigs(t);
        return ;
    }
    if(t->op==big_eqs)
    {
        build_big_equs(t);
        return ;
    }
    if(t->op==if_eqs)
    {
        build_eqs_eqs(t);
        return;
    }
    if(t->op==not_eqs)
    {
        build_not_eqs(t);
        return ;
    }
    if(t->op==whilestarts) ///while
    {
        //build_whilestarts(t);
        return ;
    }
    if(t->op==whileends)
    {
        //build_whileends(t);
        return ;
    }
    if(t->op==forstarts)
    {
        return ;
    }
    if(t->op==forends)
    {
        return ;
    }
    /*if(t->op==begin_args)
    {
        build_funpara(t);
    }
    if(t->op==callfuns)
    {
        build_callfun(t);
    }
    */
}




/***********************************************************
* 功能:用于申请寄存器 为0则可以用
**********************************************************/
char *registerspare()
{
    if(AX_FLAG==0)
    {
        AX_FLAG=1;
        return "AX";
    }
    if(BX_FLAG==0)
    {
        BX_FLAG=1;
        return "BX";
    }

    if(CX_FLAG==0)
    {
        CX_FLAG=1;
        return "CX";
    }
    return "-1";
}



/***********************************************************
* 功能:
        具体建立汇编代码

**********************************************************/

void build_define(fourvarcode* t)  ///跟符号表有关了！！
{
    flocal_table(t->addr1.name,t->addr2.name,t->addr3.name,num+1);
    if(strcmp(t->addr3.name,empty)==0)////这一定不是数组定义了
    {
        /* if(strcmp(t->addr1.content.name,"INT")==0)
         {
             ///if()如果是全局变量就要考虑 堆存储
             if()
             }*/
        num++;
    }
}


void build_assign(fourvarcode* t)
{
    char b[20],a1[20],a2[20];  ///a1 a2分别用来存储 赋值四元式的 值和变量名
    strcpy(a1,t->addr1.name);
    strcpy(a2,t->addr2.name);
    if(a1[0]>='0'&&a1[0]<='9')///数字赋给局部变量或者中间变量
    {
        if(((a2[0]>='a'&&a2[0]<='z')||(a2[0]>='A'&&a2[0]<='Z'))&&a2[1]!='#')///局部变量 形如这样的四元式= ,1,a,___
        {
            ///找一下当前变量是否在局部变量表
            if(searchlocal(t->addr2.name)!=(-1))
            {

                int n=searchlocal(t->addr2.name);
                if(strcmp(assign_table[n],empty)==0)
                {
                    strcpy(assign_table[n],t->addr1.name);
                    printf("MOV AX,%s\n",assign_table[n]);
                    printf("MOV [BP+%d],AX\n",n*2);

                }

                else  ///如果不是则为全局变量
                {
                    printf("MOV AX,%s\n",t->addr1.name);
                    printf("MOV [BP+%d],AX\n",n*2);
                }
            }
        }
        else  ///if(a2[1]=='#'形如 =，1，t#1
        {
            strcpy(b,registerspare());
            printf("MOV %s,%s\n",b,a1);
        }
    }

    else if(((a1[0]>='a'&&a1[0]<='z')||(a1[0]>='A'&&a1[0]<='Z'))&&a1[1]!='#')  ///形如这样的四元式 =,a,t#1,___ 主要翻译if while for (a<b)
    {
        ///中间变量寄存器的赋值

        int c1=searchlocal(a1);
        //if (a<b)中的a和b中间变量

        strcpy(b, registerspare());
        if (c1!=-1 )
        {
            printf("MOV %s,[BP+%d]\n", b, c1*2);
        }
        else
        {

            //printf("MOV %s,[BP+%d]\n", b, c1*2);
        }
    }
}
void build_label(fourvarcode* t)
{
    printf("%s:\n",t->addr1.name);
}

void build_jump(fourvarcode* t)
{
    printf("JMP %s\n",t->addr1.name);
}

void build_return(fourvarcode* t) ///为函数调用准备
{
    char a1[10];
    strcpy(a1,t->addr1.name);
    if(((a1[0]>='a'&&a1[0]<='z')||(a1[0]>='A'&&a1[0]<='Z'))&&a1[1]!='#')//返回局部变量
    {
        int d;
        //d=searchstack(a1);
        if(d>-1&&d<0)
        {
            d=-(d+2);
            printf("MOV BX,[BP+%d]\n",d);
            printf("MOV BP,SP\n");
        }
        else
        {
            printf("MOV [BP-2],AX \n");
        }

    }
    if(a1[1]=='#')//返回中间变量
    {

        if(AX_FLAG==1)
        {
            printf("MOV AX,BX\n");
            registerfree("AX");
        }
    }

}


void build_muls(fourvarcode* t) ///乘  x86中乘法的被乘数在AX里  结果在AX中
{
    char a3[10],axorbx[5];
    strcpy(a3,t->addr3.name);
    if(CX_FLAG==1)//用到cx寄存器了
    {

        printf("MUL BX,CX\n");
        strcpy(axorbx,"BX");
    }
    else
    {
        printf("MUL BX\n");
        AX_FLAG=1;
        strcpy(axorbx,"AX");
    }
    if(a3[1]!='#')//局部变量
    {
        int d;
        //d=searchstack(t->addr3.name);
        if(d>-1&&d<0)
        {
            d=-(d+2);
            printf("MOV [BP+%d],%s\n",d,axorbx);
            printf("MOV BP,SP\n");
        }
        else
        {
            if(stackn>=2)
                ;
            else
                printf("MOV [BP+%d],%s\n",d,axorbx);
        }

        if(strcmp(axorbx,"AX")==0)
        {
            registerfree("AX");
            registerfree("BX");
        }
        else
        {
            registerfree("BX");
            registerfree("CX");
        }
    }
    else//结果仍然留在AX 中
    {
        if(strcmp(axorbx,"AX")==0)
        {
            registerfree("BX");
        }
        else
        {
            registerfree("CX");
        }
    }
}

void build_divs(fourvarcode* t)  ///除
{
    char a3[10],axorbx[5];
    strcpy(a3,t->addr3.name);
    if(CX_FLAG==1)//用到cx寄存器了
    {
        printf("DIV BX,CX\n");
        strcpy(axorbx,"BX");
    }
    else
    {
        printf("DIV BX\n");
        strcpy(axorbx,"AX");
    }
    if(a3[1]!='#')
    {
        int d=searchlocal(t->addr3.name);
        if(d==-1)
        {
            printf("MOV [BP+%d],%s\n",d,axorbx);
            printf("MOV BP,SP\n");
        }
        //当数在局部变量符号表里
        else
        {

            printf("MOV [BP+%d],%s\n",d*2,axorbx);
        }

        if(strcmp(axorbx,"AX")==0)
        {
            registerfree("AX");
            registerfree("BX");
        }
        else
        {
            registerfree("BX");
            registerfree("CX");
        }
    }
    //结果仍然留在AX 中
    else
    {
        if(strcmp(axorbx,"AX")==0)registerfree("BX");
        else registerfree("CX");
    }
}



void build_adds(fourvarcode* t)
{
    char a2[10],a3[10],axorbx[5];
    strcpy(a2,t->addr2.name);
    strcpy(a3,t->addr3.name);
    if(strcmp(a2,"1")!=0)//不是自增运算
    {
        if(CX_FLAG==1)//用到cx寄存器了
        {
            printf("ADD BX,CX\n");
            strcpy(axorbx,"BX");
        }
        else
        {
            printf("ADD AX,BX\n");
            strcpy(axorbx,"AX");
        }
        if(a3[1]!='#')///结果赋值给局部变量
        {
            int d=searchlocal(t->addr3.name);
            if(d==-1)
            {;
                printf("MOV [BP+%d],%s\n",d,axorbx);
                printf("MOV BP,SP\n");
            }
            ///当数在局部变量符号表里
            else
            {

                printf("MOV [BP+%d],%s\n",d*2,axorbx);
            }
            if(strcmp(axorbx,"AX")==0)
            {
                registerfree("AX");
                registerfree("BX");
            }
            else
            {
                registerfree("BX");
                registerfree("CX");
            }
        }
        ///结果仍然留在AX 中
        else
        {
            if(strcmp(axorbx,"AX")==0)
            {
                registerfree("BX");
            }
            else
            {
                registerfree("CX");
            }
        }
    }
}



void build_minu(fourvarcode* t)
{
    char a2[10],a3[10],axorbx[5];
    strcpy(a2,t->addr2.name);
    strcpy(a3,t->addr3.name);
    if(strcmp(a2,"1")!=0)
    {
        if(CX_FLAG==1)///用到cx寄存器了
        {
            printf("SUB BX,CX\n");
            strcpy(axorbx,"BX");
        }
        else
        {
            printf("SUB AX,BX\n");
            strcpy(axorbx,"AX");
        }
        if(a3[1]!='#')
        {
            int d;
            //d=searchstack(t->addr3.name);
            if(d>-1&&d<0)
            {
                d=-(d+2);
                printf("MOV [BP+%d],%s\n",d,axorbx);
                printf("MOV BP,SP\n");
            }
            else
            {
                printf("MOV [BP+%d],%s\n",d,axorbx);
            }
            if(strcmp(axorbx,"AX")==0)
            {
                registerfree("AX");
                registerfree("BX");
            }
            else
            {
                registerfree("BX");
                registerfree("CX");
            }
        }
        else
        {
            if(strcmp(axorbx,"AX")==0)registerfree("BX");
            else registerfree("CX");
        }
    }

}

void build_mainfuns(fourvarcode* t)
{
    return;
}

void build_mainfuns_end(fourvarcode* t)
{
    return;
}
void build_smas(fourvarcode* t)///<
{
    printf("CMP AX,BX\n");
    printf("JAE %s\n",t->next->addr2.name);///a>=b 则跳走 --所以这里用来处理小于  也能用JNB
    registerfree("BX");
}

void build_smas_eqs(fourvarcode* t)///<=
{
    printf("CMP AX,BX\n");
    printf("JA %s\n",t->next->addr2.name);///a<b则跳走  否则继续执行
    registerfree("BX");
}

void  build_bigs(fourvarcode* t)
{
    printf("CMP AX,BX\n");
    printf("JBE %s\n",t->next->addr2.name); ///A<=B则跳走
}

void build_big_equs(fourvarcode* t)
{
    printf("CMP AX,BX\n");
    printf("JB %s\n",t->next->addr2.name); ///A<B则跳走
}
void build_eqs_eqs(fourvarcode* t)
{
    printf("CMP AX,BX\n");
    printf("JNE %s\n",t->next->addr2.name);///A!=B则跳走
}

void build_not_eqs(fourvarcode* t)
{
    printf("CMP AX,BX\n");
    printf("JE %s\n",t->next->addr2.name); ///相等就跳走
}
void build_funpara(fourvarcode* t)
{
    fourvarcode* t1;
    int c,i,cnt;
    sscanf(t->addr1.name,"%d",&c);

    //如果C=0则没有传递参数
    if(c==0)
        return;
    t1=t->next;
    for(i=c,cnt=c; i>0; cnt--)
    {
        if(i!=1)
        {
            while(i!=1)
            {
                if(t1->op==args)i--;
                t1=t1->next;
            }
        }
        else i--;
        if(t1->op!=args)
        {
            while(t1->op!=args)
            {
                build_asm(t1);
                t1=t1->next;
            }
        }
        if(cnt==c)ltag=t1;
        //t1->op=args

        //找参数地址
        char a[20];
        strcpy(a,t1->addr1.name);
        char b[20];
        //strcpy(b,registerspare());
        if (CX_FLAG == 1)
            strcpy(b,"CX");
        else if(BX_FLAG==1)
            strcpy(b,"BX");
        else//AX_FLAG=1
            strcpy(b,"AX");
        if(((a[0]>='a'&&a[0]<='z')||(a[0]>='A'&&a[0]<='Z'))&&a[1]!='#')//局部变量
        {
            int d;
            //d=searchstack(a);
            //如果找到了参数已经被定义
            if(d==-1)
            {

                printf("MOV %s,[BP+%d]\n",b,d*2);
                printf("MOV BP,SP\n");
            }
            else
                printf("MOV %s,[BP+%d]\n",b,d*2);
        }
        if(a[0]>='0'&&a[0]<='9')
        {
            printf("MOV %s,%s\n",b,a);
        }
        printf("PUSH %s\n",b);//传递中间变量
        registerfree(b);
        t1=t->next;//为传入的局部变量
    }

}
void build_callfun(fourvarcode *t)
{
    printf("call near ptr %s\n",t->addr1.name);

}
