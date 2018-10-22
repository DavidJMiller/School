/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Register Allocation
 */

#include <stdio.h>
#include <strings.h>
#include "opt.h"
#include "misc.h"
#include "vars.h"
#include "opts.h"

void replaceandfix(struct varinfo, char *);

/*
 * regalloc - perform register allocation
 */
void regalloc(int *changes)
{
   extern struct bblk *top;
   extern struct varinfo vars[];
   extern int numvars;
   struct bblk *cblk;
   struct assemline *cline;
   varstate regs, full;
   char *allocedreg, tmp[32], newline[MAXLINE];
   int i, flag;


   /* Create varinfo for regs in use and one to represent all regs in use */
   varinit(regs);
   varinit(full);
   for(i = 0; i < NUMVARWORDS; i++)
      full[i] = -1;

   /* Compute which registers are currently in use */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      for(cline = cblk->lines; cline; cline = cline->next)
      {
         unionvar(regs, regs, cline->sets);
         unionvar(regs, regs, cline->uses);
      }
   }

   /* Iterate over current vars */
   for(i = 0; i < numvars; i++)
   {
      /* No more allocatable regs available */
      if(varcmp(regs, full))
         break;

      /* Var is referenced indirectly or can't be allocated (no free reg) */
      allocedreg = allocstring("");
      if(vars[  i].indirect || !allocreg(vars[i].type, regs, allocedreg))
         continue;

      /*
       * Iterate over all blocks and lines to find var define line and replace
       * all load/store occurences with move
       */
      flag = FALSE;
      for(cblk = top; cblk; cblk = cblk->down)
      {
         for(cline = cblk->lines; cline; cline = cline->next)
         {
            /* Check for load/store inst so it can be made into a move inst */
            memset(tmp, 0, sizeof(tmp));
            if(sscanf(cline->text, "%*[^(fp)]fp + %[^]]", tmp) == 1)
            {
               if(strcmp(vars[i].name, tmp) == 0 && cline->type == STORE_INST)
                  createmove(vars[i].type, cline->items[1], allocedreg, cline);
               if(strcmp(vars[i].name, tmp) == 0 && cline->type == LOAD_INST)
                  createmove(vars[i].type, allocedreg, cline->items[2], cline);
            }
            
            /* 
             * Without this check, I was getting a segfault at strcmp for some 
             * reason (minor issue but still needs fixing/understanding).
             */
            if(flag)
               continue;

            /* Change def line from "name = value" to "! name = allocedreg" */
            if(strcmp(cline->items[0], vars[i].name) == 0)
            {
               strcpy(newline, "! ");
               strcat(newline, cline->items[0]);
               strcat(newline, " = ");
               strcat(newline, allocedreg);
               cline->text = allocstring(newline);
               flag = TRUE;
            }
         }
      }

      /* Set newly allocated register as used */
      if(vars[i].type == DOUBLE_TYPE)
         insreg(allocedreg, regs, 2);
      else
         insreg(allocedreg, regs, 1);

      /* Performed a register allocation optimization */
      incropt(REGISTER_ALLOCATION);
   }
}