/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Local Copy Propagation
 */

#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "misc.h"
#include "vars.h"

/*
 * localcopyprop - perform copy propagation
 */
void localcopyprop(int *changes)
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct assemline *cline, *itrline;

   /* Iterate over all basic blocks line by line */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      for(cline = cblk->lines; cline; cline = cline->next)
      {
         /* Not mov inst, x <- x (redundant), or x <- y where x is not a reg */
         if(cline->type != MOV_INST || 
            !strcmp(cline->items[1], cline->items[2]) || 
            !isreg(cline->items[1]))
            continue;

         /* Look for propagation opportunity in remaining part of blook */
         for(itrline = cline->next; itrline; itrline = itrline->next)
         {
            /* If propagation reg is used, inst not a return, and reg in text */
            if(regexists(cline->items[2], itrline->uses) && 
               itrline->type != RETURN_INST &&
               strstr(itrline->text, cline->items[2]) != NULL)
            {
               delreg(cline->items[2], itrline->uses, 2);
               insreg(cline->items[1], itrline->uses, 2);
               replaceuse(itrline, cline->items[2], cline->items[1]);
               incropt(COPY_PROPAGATION);
            }

            /* Propagation regs are set again */
            if(regexists(cline->items[2], itrline->sets) || 
               regexists(cline->items[1], itrline->sets))
               break;
         }
      }
   }
}