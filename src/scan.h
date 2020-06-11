#ifndef SCAN_H_INCLUDED
#define SCAN_H_INCLUDED
#include "global.h"
typedef enum{INT,FLOAT,CHAR,DOUBLE,VOID,MAIN,IF,ELSE,WHILE,PRINTF,SCANF,FOR,
             EQUAL,PLUS,pPLUS,MINUS,mMINUS,TIMES,DIVIDE,MODE,        ///= + - * / %
			 SMALLER,SMALLEREQU,BIGGER,BIGGEREQU,NOTEQU,IFEQU,///< <= > >= != ==
             L_DA,R_DA,L_ZH,R_ZH,L_XI,R_XI,///{}[]()
			 FENH,DOUH,DIAN,           ///;, .
			 NUM,ID,COMMENT,RETURN,
			 AUTO,BREAK,CASE,CONST,CONTINUE,
			 STRUCT,MAOH,DEFAULT,SWITCH , ///后面补充的 结构体struct 以及switch语句
			 HUO,ADE}tokentype;  ///后面补充的 &&  ||
typedef struct
{
	tokentype tokenval;         /**一个token的**/
	char stringval[100];        /**token的值 数字,字母,关键字等会有值，其他像（{ ; 都没有值***/
	double numval;              /**值先设为double，有可能为浮点数**/
	}tokenrecord;
#endif



