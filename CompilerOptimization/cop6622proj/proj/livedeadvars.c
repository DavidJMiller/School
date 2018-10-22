/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Live and Dead Variable Analysis
 */

#include <stdio.h>
#include "opt.h"
#include "misc.h"
#include "vars.h"
#include "analysis.h"
#include "opts.h"

void rmvspecregs(varstate);

/*
 * calclivevars - calculate live variable information
 */
void calclivevars()
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct blist *slist;
   struct assemline *cline;
   varstate tmp;
   int flag;

   /* Iterate over all basic blocks */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      varinit(cblk->defs);
      varinit(cblk->uses);

      /* Iterate over each assemline */
      for(cline = cblk->lines; cline; cline = cline->next)
      {   
         /* Skip empty insts */
         if(!INST(cline->type))
            continue;

         /* Add qualifiable regs to cblk's uses */
         varinit(tmp);
         minusvar(tmp, cline->uses, cblk->defs);
         unionvar(cblk->uses, cblk->uses, tmp);

         /* Add qualifiable regs to cblk's defs */
         varinit(tmp);   
         minusvar(tmp, cline->sets, cblk->uses);
         //minusvar(tmp, tmp, cblk->uses);
         unionvar(cblk->defs, cblk->defs, tmp);
      }
      /* Remove non-general registers */
      rmvspecregs(cblk->defs);
      rmvspecregs(cblk->uses);
   }

   /* Compute ins and outs for each basic block */
   for(cblk = top; cblk; cblk = cblk->down)
      varinit(cblk->ins);
   /* Keep updating as long as some block's ins has changed */
   do 
   {
      flag = FALSE;
      for(cblk = top; cblk; cblk = cblk->down)
      {
         varinit(cblk->outs);
         for(slist = cblk->succs; slist; slist = slist->next)
            unionvar(cblk->outs, cblk->outs, slist->ptr->ins);
         minusvar(tmp, cblk->outs, cblk->defs);
         unionvar(tmp, cblk->uses, tmp);
         flag |= !varcmp(cblk->ins, tmp);
         varcopy(cblk->ins, tmp);
      }
   }while(flag);
}

/*
 * calcdeadvars - calculate dead variable information
 */
void calcdeadvars()
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct assemline *cline, *itrline;
   varstate tmp, tmp2;

   /* Iterate over all basic blocks */
   for(cblk = top; cblk; cblk = cblk->down)
   {
      /* Skip empty blocks */
      if(!cblk->lineend)
         continue;

      varinit(tmp);
      minusvar(tmp, cblk->ins, cblk->outs);

      /* Iterate over each assemline */
      for(cline = cblk->lines; cline; cline = cline->next)
      {
         /* Skip useless lines */
         if(varempty(cline->uses))
            continue;

         /* Last use of reg in basic block */
         intervar(cline->deads, cline->uses, tmp);
         for(itrline = cline->next; itrline; itrline = itrline->next)
            minusvar(cline->deads, cline->deads, itrline->uses);

         /* Reg is set and used in same line */
         intervar(tmp, cline->sets, cline->uses);
         unionvar(cline->deads, cline->deads, tmp);

         /* Last time reg is used before set or used without being an in */
         varinit(tmp2);
         minusvar(tmp, cline->uses, cblk->outs);
         for(itrline = cline->next; itrline; itrline = itrline->next)
         {
            intervar(tmp2, tmp, itrline->sets);
            unionvar(cline->deads, cline->deads, tmp2);
            minusvar(tmp, tmp, itrline->uses);
         }
         unionvar(cline->deads, cline->deads, tmp);

         /* Remove non-general registers */
         rmvspecregs(cline->deads);
      }
   }
}

/* 
 * Remove non-general registers 
 */
void rmvspecregs(varstate v)
{
   varstate nongen;
   varinit(nongen);
   insreg("%fp", nongen, 1);
   insreg("%sp", nongen, 1);
   insreg("%g0", nongen, 1);
   minusvar(v, v, nongen);
}