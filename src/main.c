#include "global.h"
#include "parse.h"


int main()
{
    printf(">>> scan     =====================================\n");
    scan();
    //printf("\nscan success!!\n\n");
    //printf("syntax tree ----------\n");
    printf("\n\n>>> ast     =====================================\n");
   parse();
    //printf("\nparse success!!\n\n");
    //start_symbtab();
    printf("\n\n>>> tac     =====================================\n");
  tac();
    printf("\n\n>>>after optimize     =====================================\n");
    printf("\nafter optimize\n\n");
 start_optimize();
    printf("\n\n>>> asm     =====================================\n");
  start_asm();
    //get_table();
    return 0;
}
