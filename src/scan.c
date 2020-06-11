#include "scan.h"
#include "global.h"

char* tokenstring();                                ///转换成字符串
static void get_nextline(FILE *in);                 ///获取下一行的代码
static void get_token();                            ///获取token
static int state_change(int state,char ch);         ///状态转换
static char *returnstring(int m,int n);             ///将识别的出来的token进行复制保存到数组中
static tokentype letterchange(char a[]);            ///键字的识别转换，如果为关键字， 则存储为关键字，不是关键字就是ID
static int is_digit(char ch);                       ///数字的判断
static int is_letter(char ch);                      ///字符的判断
static int is_keywords(char a[]);                   ///关键字的判断
static void print_token();                          ///输出
/**关键字数组**/
static char keywords[50][20]={ "char", "continue","break",
                                "else","for", "float","if","while",
                                "int","main","double","case",
                                "return","void","continue", "struct",
                                "switch","default"};
int tokenno=0;                      ///token的数量 语法分析还要用
tokenrecord token_table[1000];           /**用于存放识别的的token**/
char linebuf[1000];                                 /**每一行代码缓冲区**/
int lineno=0;                                       /**lineno用于记录代码的行数;**/
int currentchar;                                    /**。currentchar为代码缓冲区的计数器**/
int line_num_table[1000]={0};                       /**line_num_table记录每一行最后一个记号的位置**/
tokentype token;

/***********************************************************
 * 功能:	表驱动
 **********************************************************/
static int transform_table[16][13]={{0,0,0,0,0,0,0,0,0,0,0,0,0},    ///-1 为出口
                        {2,0,0,5,0,6,0,10,8,12,0,0,0},
                        {2,3,-1,0,0,0,0,0,0,0,0,0,0},
                        {4,0,0,0,0,0,0,0,0,0,0,0,0},
                        {4,0,-1,0,0,0,0,0,0,0,0,0,0},
                        {5,0,0,5,-1,0,0,0,0,0,0,0,0},
                        {0,0,0,0,0,0,-1,7,0,0,0,0,0},
                        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
                        {0,0,0,0,0,0,-1,9,0,0,0,0,0},
                        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
                        {0,0,0,0,0,0,-1,11,0,0,0,0,0},
                        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
                        {0,0,0,0,0,0,0,0,0,13,-1,0,0},
                        {0,0,0,0,0,0,0,0,0,0,0,14,15},
                        {0,0,0,0,0,0,0,0,0,0,0,14,15},
                        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};

/***********************************************************
 * 功能:	词法分析的入口函数
 1:利用行缓冲技术进行读取每一行的字符
 **********************************************************/
void scan()
{
    FILE *text;
    text= fopen("../demo/test2.c", "r");/**读取测试文件**/
	if(text!=NULL)
	{

		while(!feof(text))                                  /**文件直到循环中读到文件尾feof(fp)有两个返回值:如果遇到文件结束，函数feof（fp）的值为1，否则为0。**/
		{
		    printf("yes");
			currentchar=0;                                  /**每读完一行重新清零**/
			get_nextline(text);                               /**获取每一行，并放在linebuf中，以'/0'结束**/
			lineno++;
			if (strcmp(linebuf, " ") == 0) /**比较设否是空串**/
            {
                continue;
            }
			while(currentchar<strlen(linebuf))              /**注意currentchar。strlen是不包括'/0'的！但是有换行符‘\n’**/
			{
                    get_token();

			}
			line_num_table[lineno]=tokenno-1;
		}
	}
	fclose(text);
	print_token();
}



/***********************************************************
 * 功能:	行缓冲的具体实现
 **********************************************************/
static void get_nextline(FILE *text)
{
	if (!fgets(linebuf, 9999, text))/**当最后一行输入只是一个回车，会返回错误**/
	{
		strcpy(linebuf, " ");
	}
}


/***********************************************************
 * 功能:	识别具体的token
 通过自动机和表驱动
 **********************************************************/
static void get_token()
{
	char ch=linebuf[currentchar];;
	int ncurrentchar=currentchar;
	if(ch==' '||ch=='\n'||ch=='\t')
    {
        currentchar++;
        return;
    }
    /**以下是不需要进行状态表驱动，直接一步可以识别的token: / + - ! { } . [ ]**/
	switch (ch)
	{
	    case '!':
			{
				if(linebuf[currentchar+1]=='=')
				{
					token_table[tokenno].tokenval=NOTEQU;
					currentchar=currentchar+2;
				}
				break;
			}
		case '+':
			{
			    if(linebuf[currentchar+1]=='+')
				{
				    token_table[tokenno].tokenval=pPLUS;
                    currentchar=currentchar+2;
                    break;
				}
				else
                {
                    token_table[tokenno].tokenval=PLUS;
                    currentchar++;
                    break;
                }
			}
		case'-':
			{
			    if(linebuf[currentchar+1]=='-')
				{
				    token_table[tokenno].tokenval=mMINUS;
                    currentchar=currentchar+2;
                    break;
				}
				else
				{
                    token_table[tokenno].tokenval=MINUS;
                    currentchar++;
                    break;
				}
			}
		case '*':
			{
				token_table[tokenno].tokenval=TIMES;
				currentchar++;
				break;
			}
		case '%':
			{
				token_table[tokenno].tokenval=MODE;
				currentchar++;
				break;
			}
		case '{':
			{
				token_table[tokenno].tokenval=L_DA;
				currentchar++;
				break;
			}
		case '}':
			{
				token_table[tokenno].tokenval=R_DA;
				currentchar++;
				break;
			}
		case '[':
			{
				token_table[tokenno].tokenval=L_ZH;
				currentchar++;
				break;
			}
		case ']':
			{
				token_table[tokenno].tokenval=R_ZH;
				currentchar++;
				break;
			}
		case '(':
			{
				token_table[tokenno].tokenval=L_XI;
				currentchar++;
				break;
			}
		case ')':
			{
				token_table[tokenno].tokenval=R_XI;
				currentchar++;
				break;
			}
		case ';':
			{
				token_table[tokenno].tokenval=FENH;
				currentchar++;
				break;
			}
        case ':':
			{
				token_table[tokenno].tokenval=MAOH;
				currentchar++;
				break;
			}
		case ',':
			{
				token_table[tokenno].tokenval=DOUH;
				currentchar++;
				break;
			}
		case '.':
			{
				token_table[tokenno].tokenval=DIAN;
				currentchar++;
				break;
			}
        case '&':
			{
				token_table[tokenno].tokenval=ADE;
				currentchar++;
				break;
			}
        case '|':
			{
				token_table[tokenno].tokenval=HUO;
				currentchar++;
				break;
			}

	default:
	{
            int state=1,newstate;
            while(1)/**利用状态表驱动，识别token过程**/
            {
                int col=state_change(state,ch);
                if(col==999)
                {
                    printf("ERROR!!!!!");
                }
                newstate=transform_table[state][col];
                if(newstate == -1) break;
                currentchar++;
                ch=linebuf[currentchar];
                state=newstate;
            }
            switch(state)
            {
            case 2: /**整型数字**/
                {
                    token_table[tokenno].tokenval=NUM;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    token_table[tokenno].numval=atof(token_table[tokenno].stringval);/**atof()将字符串转换为数字**/
                    break;
                }

            case 4:/**浮点数**/
                {
                    token_table[tokenno].tokenval=NUM;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    token_table[tokenno].numval=atof(token_table[tokenno].stringval);
                    break;
                }
            case 5:  /**如果是保留字，则保存保留字，若不是，则保存变量**/
                {
                        printf("%s",returnstring(ncurrentchar,currentchar-1));
                        printf("%s",token_table[tokenno].stringval);
                        strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                        if(is_keywords(token_table[tokenno].stringval))
                        {
                            token_table[tokenno].tokenval=letterchange(token_table[tokenno].stringval);
                        }

                        else
                            token_table[tokenno].tokenval=ID;
                        break;
                }
            case 6:  /**布尔型 <**/
                {
                    token_table[tokenno].tokenval=SMALLER;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
            case 7:/**布尔型 <=**/
                {
                    token_table[tokenno].tokenval=SMALLEREQU;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
             case 8:/**布尔型 >**/
                {
                    token_table[tokenno].tokenval=BIGGER;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
            case 9:/**布尔型 >=**/
                {
                    token_table[tokenno].tokenval=BIGGEREQU;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }

            case 10:/**布尔型 =**/
                {
                    token_table[tokenno].tokenval=EQUAL;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
            case 11:/**布尔型 ==**/
                {
                    token_table[tokenno].tokenval=IFEQU;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
             case 12:/**布尔型 / **/
                {
                    token_table[tokenno].tokenval=DIVIDE;
                    strcpy(token_table[tokenno].stringval,returnstring(ncurrentchar,currentchar-1));
                    break;
                }
            case 15:/**注释**/
                {
                    token_table[tokenno].tokenval=COMMENT;
                    break;
                }
            default:
                {
                    printf("error");
                }
            }
	}
	}
	tokenno++;
}



/***********************************************************
 * 功能:关键字的识别转换，如果为关键字，
 则存储为关键字，不是关键字就是ID
 **********************************************************/
tokentype letterchange(char a[])
{
    if(strcmp(a,"int")==0) return INT;
    if(strcmp(a,"break")==0) return BREAK;
    if(strcmp(a,"case")==0) return CASE;
    if(strcmp(a,"char")==0) return CHAR;
    if(strcmp(a,"continue")==0) return CONTINUE;
    if(strcmp(a,"double")==0) return DOUBLE;
    if(strcmp(a,"float")==0)return FLOAT;
	if(strcmp(a,"char")==0)return CHAR;
	if(strcmp(a,"void")==0)return VOID;
	if(strcmp(a,"main")==0)return MAIN;
	if(strcmp(a,"if")==0)return IF;
	if(strcmp(a,"else")==0)return ELSE;
	if(strcmp(a,"while")==0)return WHILE;
	if(strcmp(a,"printf")==0)return PRINTF;
	if(strcmp(a,"scanf")==0)return SCANF;
	if(strcmp(a,"for")==0)return FOR;
	if(strcmp(a,"return")==0)return RETURN;
	if(strcmp(a,"struct")==0)return STRUCT;
	if(strcmp(a,"default")==0)return DEFAULT;
	if(strcmp(a,"switch")==0) return SWITCH;
	if(strcmp(a,"break")==0) return BREAK;
	printf("error!!!");
}


/***********************************************************
 * 功能:	判断是否是关键字
 **********************************************************/
static int is_keywords(char a[])
{
    int i=0;

    for(;i<50;i++)
        if(strcmp(a,keywords[i])==0)/**相等则是保留字**/
        {
             return 1;
        }

    return 0;
}


/***********************************************************
 * 功能:	状态变化的实现，通过状态转换表
 **********************************************************/
static int state_change(int state,char ch)
{

    if(state==1)
    {
        if (is_digit(ch)) return 0;
        if (is_letter(ch)) return 3;
        if (ch=='<') return 5;
        if(ch=='>') return 8;
        if(ch=='=') return 7;
        if(ch=='/') return 9;
    }
    if(state==2)
    {
        if(is_digit(ch)) return 0;
        if(ch=='.')  return 1;
        else return 2;
    }

    if(state==3)
    {
        if(is_digit(ch)) return 0;
    }

    if(state==4)
    {
        if(is_digit(ch)) return 0;
        else return 2;
    }

    if(state==5)
    {
        if(is_digit(ch)||is_letter(ch)) return 0;
        return 4;
    }

    if(state==6||state==8||state==10)
    {
        if(ch=='=') return 7;
        else return 6;
    }

     if(state==7||state==9||state==11||state==15)
    {
        return 0;
    }
    if(state==12)
    {
        if(ch=='/') return 9;
        if(ch!='=')return 10;
    }
     if(state==13||state==14)
    {

        if(ch=='\n'){ return 12;}
        else return 11;
    }
    return 999;

}


/***********************************************************
* 功能:	将识别的出来的token进行复制保存到数组中
**********************************************************/

static char* returnstring(int m,int n)
{
    int i=0;
    char s[200];
    for(;i<=(n-m);i++)
    {
        s[i]=linebuf[m+i];
    }
    s[i]='\0';
    char *p =s;
    return p;
}
/***********************************************************
* 功能:	1.判断是否为char
2.判断是否为数字
**********************************************************/

static int is_letter(char ch)
{
    if(('a'<=ch&&ch<='z')||('A'<=ch&&ch<='Z'))
    {
        return 1;
    }

	else
    {
        return 0;
    }
}

static int is_digit(char ch)
{
    if('0'<=ch&&ch<='9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/***********************************************************
* 功能: 输出的时候进行的字符转换
**********************************************************/
char* tokenstring()
{
	switch (token)
	{
	case INT :
	    return "int";
	case FLOAT:
	    return "float";
	case CHAR:
	    return "char";
	case DOUBLE:
	    return "double";
	case VOID:
	    return "void";
	case MAIN:
	    return "main";
	case IF:
	    return "if";
	case ELSE:
	    return "else";
	case WHILE:
	    return "while";
	case PRINTF:
	    return "printf";
	case SCANF:
	    return "scanf";
	case FOR:
	    return "for";
	case EQUAL:
	    return "=";
	case PLUS:
	    return "+";
	case pPLUS:
	    return "++";
	case MINUS:
	    return "-";
	case mMINUS:
	    return "--";
	case TIMES:
	    return "*";
	case DIVIDE:
	    return "/";
	case MODE:
	    return "%";
	case SMALLER:
	    return "<";
	case SMALLEREQU:
	    return "<=";
	case BIGGER:
	    return ">";
	case BIGGEREQU:
	    return ">=";
	case NOTEQU:
	    return "!=";
	case IFEQU:
	    return "==";
    case ADE:
	    return "&";
    case HUO:
	    return "|";
	case L_DA:
	    return "{";
	case R_DA:
	    return "}";
	case L_ZH:
	    return "[";
	case R_ZH:
	    return "]";
	case L_XI:
	    return "(";
	case R_XI:
	    return ")";
	case FENH:
	    return ";";
    case MAOH:
        return ":";
	case DOUH:
	    return ",";
	case DIAN:
	    return ".";
	case NUM:
	    return  "number";
	case ID:
	    return "id";
	case RETURN:
	    return "return";
	case COMMENT:
	    return "注释";
    case STRUCT:
        return "struct";
    case CASE:
        return "case";
    case DEFAULT:
        return "default";
    case SWITCH:
        return "switch";
    case BREAK:
        return "break";
    default:
        printf("error！！！无法匹配！！！");
	}
}


/***********************************************************
* 功能:用于输出token
**********************************************************/
static void print_token()
{

	int i;
	//printf("tokenno=%d\n",tokenno);
	int j;
	for(j=1;line_num_table[j]!=0;j++)
	{
	    printf("%d: ",j);
		for(i=line_num_table[j-1];i<=line_num_table[j];i++)
		{
		    if(j!=1&&i==line_num_table[j-1])
            {
                i++;
            }
			token=token_table[i].tokenval;
			printf("[%s] ",tokenstring());
		}
		printf("\n");
	}

}
