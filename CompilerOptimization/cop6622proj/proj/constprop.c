/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Local Constant Propagation
 */

#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "io.h"
#include "misc.h"
#include "vars.h"

/*
 * localconstprop - perform constant propagation
 */
void localconstprop(int *changes)
{
   extern struct bblk *top;
   extern struct instinfo insttypes[];
   struct bblk *cblk;
   struct assemline *cline, *itrline;

   /* Iterate over all basic blocks line by line */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      for(cline = cblk->lines; cline; cline = cline->next)
      {
         /* Not mov inst, x <- x (redundant), or x <- y where x is not a reg */
         if(cline->type != MOV_INST || !isconst(cline->items[1]))
            continue;

         /* Look for const propagation opportunity in remaining part of blook */
         for(itrline = cline->next; itrline; itrline = itrline->next)
         {
            /* If propagation reg is used, inst not a return, reg in text, and
               the src reg can be used to hold the const */
            if(regexists(cline->items[2], itrline->uses) && 
               itrline->type != RETURN_INST &&
               strstr(itrline->text, cline->items[2]) != NULL &&
               insttypes[itrline->instinfonum].lastsrccanbeconst)
            {
               delreg(cline->items[2], itrline->uses, 
                      insttypes[itrline->instinfonum].numdstregs);
               replaceuse(itrline, cline->items[2], cline->items[1]);
               incropt(CONSTANT_PROPAGATION);
            }

            /* Propagation regs are set again */
            if(regexists(cline->items[2], itrline->sets))
               break;
         }
      }
   }
}