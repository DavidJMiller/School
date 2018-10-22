/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Dead Assignment Elimination
 */

#include <stdio.h>
#include "opt.h"
#include "vars.h"
#include "analysis.h"
#include "opts.h"
#include "misc.h"

int setinst(struct assemline *);

/*
 * deadasgelim - perform dead assignment elimination
 */
void deadasgelim()
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct assemline *cline, *itrline, *tmpline;
   varstate nongen;
   int flag;

   varinit(nongen);
   insreg("fp", nongen, 1);
   insreg("g0", nongen, 1);
   insreg("sp", nongen, 1);

   /* Iterate over all basic blocks line by line */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      for(cline = cblk->lines; cline;)
      {
         /* Check if inst is one that sets a value */
         if(!setinst(cline))
         {
            cline = cline->next;
            continue;
         }

         /* Check if inst sets a value and doesn't use non-general regs */
         if(varempty(cline->sets) || varcommon(nongen, cline->sets))
         {
            cline = cline->next;
            continue;
         }

         /* Check remaining block for dead assignment opportunity */
         flag = TRUE;
         for(itrline = cline->next; itrline; itrline = itrline->next)
         {
            /* Used in inst, it is not dead */
            if(varcommon(cline->sets, itrline->uses)) 
            {
               flag = FALSE;  
               break;
            }

            /* Reassigned before used, it is dead */
            if(varcommon(cline->sets, itrline->sets))
            {
               tmpline = cline->next;
               delline(cline);
               incropt(DEAD_ASG_ELIM);
               cline = tmpline;
               flag = FALSE;
               break;
            }
         }

         /* It is set without being used in some succ (not in block's out) */
         if(!varcommon(cline->sets, cblk->outs) && flag)
         {
            tmpline = cline->next;
            delline(cline);
            incropt(DEAD_ASG_ELIM);
            cline = tmpline;
         }
         cline = cline->next;
      }
   }
}

/*  
 * setinst - checks if there exists an inst in line that sets a value
 */
int setinst(struct assemline *line)
{
   if(!INST(line->type))
      return FALSE;

   if(TOC(line->type) || line->type == SAVE_INST || line->type == RESTORE_INST)
      return FALSE;

   return TRUE;
}