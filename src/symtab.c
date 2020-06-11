#include"symtab.h"
#include"global.h"

static void insertNode(treenode * t);
static void nullProc(treenode * t);
void st_insert( char * name, int lineno, int loc );
int st_lookup ( char * name );
static void checkNode(treenode * t);
void printSymTab();
void buildSymtab(treenode * syntaxTree);
static void traverse(treenode * t,
                     void (* preProc) (treenode *),
                     void (* postProc) (treenode *) );


/* 基准选择16，2的4次方*/
#define SHIFT 4
#define SIZE 211
/* 返回哈希值，哈希函数 */
int location = 0;
static int hash(char* key)
{
    int t = 0;
    int i = 0;
    while(key[i] != '\0')
    {
        t = ((t<<SHIFT) + key[i]) % SIZE;
        i++;
    }
    return t;
}
typedef struct LineListRec
{
    int lineno;
    struct LineListRec * next;
} * LineList;

typedef struct BucketListRec
{
    char * name;
    LineList lines;
    int memloc ; /* memory location for variable */
    struct BucketListRec * next;
} * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

void start_symbtab()
{
    buildSymtab(tree_gen);
    //typeCheck(tree_gen);
}

void buildSymtab(treenode * syntaxTree)
{
    traverse(syntaxTree,insertNode,nullProc);
    if (1)
    {
        printf("\nSymbol table:\n\n");
        printSymTab();
    }
}


static void nullProc(treenode * t)
{
    if (t==NULL) return;
    else return;
}


/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(treenode * t)
{
    switch (t->nodekind)
    {
    case stmtk:
        switch (t->kind.stmt)
        {
        case assignk:
        case defineparak:
            if (st_lookup(t->attr.name) == -1)
                /* not yet in table, so treat as new definition */
                st_insert(t->attr.name,t->lineno,location++);
            else
                /* already in table, so ignore location,
                   add line number of use only */
                st_insert(t->attr.name,t->lineno,0);
            break;
        default:
            break;
        }
        break;
    case expk:
        switch (t->kind.exp)
        {
        case idk:
            if (st_lookup(t->attr.name) == -1)
                /* not yet in table, so treat as new definition */
                st_insert(t->attr.name,t->lineno,location++);
            else
                /* already in table, so ignore location,
                   add line number of use only */
                st_insert(t->attr.name,t->lineno,0);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void traverse(treenode * t,
                     void (* preProc) (treenode *),
                     void (* postProc) (treenode *) )
{
    if (t != NULL)
    {
        preProc(t);
        {
            int i;
            for (i=0; i < 4; i++)
                traverse(t->child[i],preProc,postProc);
        }
        postProc(t);
        traverse(t->sibling,preProc,postProc);
    }
}




void st_insert( char * name, int lineno, int loc )
{
    int h = hash(name);
    BucketList l =  hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
        l = l->next;
    if (l == NULL) /* variable not yet in table */
    {
        l = (BucketList) malloc(sizeof(struct BucketListRec));
        l->name = name;
        l->lines = (LineList) malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->memloc = loc;
        l->lines->next = NULL;
        l->next = hashTable[h];
        hashTable[h] = l;
    }
    else /* found in table, so just add line number */
    {
        LineList t = l->lines;
        while (t->next != NULL) t = t->next;
        t->next = (LineList) malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
    }
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{
    int h = hash(name);
    BucketList l =  hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
        l = l->next;
    if (l == NULL) return -1;
    else return l->memloc;
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab()
{
    int i;
    printf("Variable Name  Location   Line Numbers\n");
    printf("-------------  --------   ------------\n");
    for (i=0; i<SIZE; ++i)
    {
        if (hashTable[i] != NULL)
        {
            BucketList l = hashTable[i];
            while (l != NULL)
            {
                LineList t = l->lines;
                printf("%-14s ",l->name);
                printf("%-8d  ",l->memloc);
                while (t != NULL)
                {
                    printf("%4d ",t->lineno);
                    t=t->next;
                }
                printf("\n");
                l = l->next;
            }
        }
    }
} /* printSymTab */




/*void typeCheck(treenode * syntaxTree)
{
    traverse(syntaxTree,nullProc,checkNode);
}

static void typeError(treenode * t, char * message)
{
    fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);

}*/
/*static void checkNode(treenode * t)
{ switch (t->nodekind)
  { case expk:
      switch (t->kind.exp)
      {
          case opk:
          if ((t->attr.op ==BIGGER ) || (t->attr.op == SMALLER)|| (t->attr.op == SMALLEREQU)||(t->attr.op ==BIGGEREQU)||(t->attr.op ==  NOTEQU)|| (t->attr.op ==IFEQU))
            t->type = Boolean;
          else
            t->type = Integer;
          break;
        case constk:
        case idk:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case stmtk:
      switch (t->kind.stmt)
      { case ifk:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case assignk:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
        case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}
*/





