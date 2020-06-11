#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED
#include "scan.h"
typedef enum{stmtk,expk} Nodekind; ///结点类型

typedef enum{ifk,fork,whilek,returnk,assignk,
             funck,defineparak,maink,define_arrayk,
             structk,switchk,casek,breakk,defaultk} Stmtkind;  ///句子类型

typedef enum{ opk, constk, idk, cite_arrayk} Expkind; ///表达式类型



typedef struct treenode
{
	struct treenode* child[4];///也就for能有四个儿子
	struct treenode* sibling;
	int lineno;
	Nodekind nodekind;
	union{Stmtkind stmt;Expkind exp;}kind;

	union{tokentype op;double val;char name[100];}attr;
	///（Expkind）操作符，常量，标识符（变量名，函数名（idk））
	tokentype dtype;
	///变量、常量、函数返回值的类型 包括INT,FLOAT,CHAR,DOUBLE,VOID
}treenode;  ///参考编译原理及实践的树结构的定义



extern tokenrecord token_table[10000];
extern int line_num_table[1000];
extern int lineno;
extern int tokenno;
extern tokentype token;


#endif // PARSE_H_INCLUDED
