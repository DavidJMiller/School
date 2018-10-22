/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Local Common Subexpression Elimination
 */

#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "misc.h"
#include "vars.h"

/*
 * cseinblk - perform local common subexpression elimination in a block
 */
void cseinblk(struct bblk *cblk, int *changes)
{
	extern struct instinfo insttypes[];
	struct assemline *cline, *nline, *iline;
	int i, overwrite, redundant, cse;
	varstate regs;

	/* Iterate over lines of cblk */
	varinit(regs);
	unionvar(regs, regs, cblk->ins);
	for(cline = cblk->lines; cline; cline = cline->next)
	{
		/* Skip insts that are not arithmetic inst */
		if(cline->type != ARITH_INST)
			continue;

		/* Iterate over remaining lines in cblk */
		overwrite = FALSE;
		cse = FALSE;
		unionvar(regs, regs, cline->sets);
		for(nline = cline->next; nline; nline = nline->next)
		{
			/* Close off set regs */
			unionvar(regs, regs, nline->sets);

			/* Determine if overwriting occurs and easy redundant check */
			overwrite |= varcommon(cline->sets, nline->sets);
			redundant = (nline->type == cline->type && 
			            nline->numitems == cline->numitems);     
			/* Can't apply cse if use regs have new values or if inst sets and 
			   uses dest reg */
			if(varcommon(cline->uses, nline->sets) ||
			   (varcommon(cline->sets, nline->sets) && 
			    varcommon(cline->sets, nline->uses)))
				break;                                                                                    

			/* Check for matching items */
			for(i = 0; (i < cline->numitems - 1) && redundant; i++)
				redundant = (strcmp(cline->items[i], nline->items[i]) == 0);

			/* Check if we can save value beforew overwrite */
			if(overwrite && redundant)
			{
				/* Cant save value to reg, skip this cline */
				char *newreg = allocstring(""), 
				     *oldreg = cline->items[cline->numitems-1];
				if(allocreg(insttypes[cline->instinfonum].datatype, 
					        regs, newreg) == FALSE)
					break;

				/* Replace oldreg with new reg in uses */
				for(iline = nline; iline != cline; iline = iline->prev)
					if(regexists(oldreg, iline->uses))
					   replaceuse(iline, oldreg, newreg);

				/* Update dst in cline with newreg */
				replacedst(cline, newreg);
				overwrite = FALSE;
			}
			/* Make redudant computation inst into mov inst */
			if(redundant)
			{
				createmove(insttypes[nline->instinfonum].datatype, 
				  	       cline->items[cline->numitems-1], 
				    	   nline->items[nline->numitems-1],
				       	   nline);
				cse = TRUE;
			}
		}

		/* Was able to apply cse using cline */
		if(cse)
		{
			incropt(LOCAL_CSE);
			*changes = TRUE;
		}
	}
}

/*
 * localcse - perform local common subexpression elimination
 */
void localcse(int *changes)
{
   struct bblk *cblk;
   extern struct bblk *top;

   for (cblk = top; cblk; cblk = cblk->down)
      cseinblk(cblk, changes);
}