/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Filling Delay Slots
 */

#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "vars.h"
#include "misc.h"
#include "opts.h"
#include "io.h"

/*
 * filldelayslots - fill the delay slots of the transfers of control in a
 *                  function
 */
void filldelayslots()
{
	extern struct instinfo insttypes[];
	extern struct bblk *top;
	struct bblk *cblk;
	struct assemline *cline, *pline, *iline;
	int flag;

	/* Iterate over each block line by line */
	for(cblk = top; cblk; cblk = cblk->down)
	{
		for(cline = cblk->lines; cline; cline = cline->next)
		{
			/* Skip if inst is not branch, call, or jump */
			if(cline->type != BRANCH_INST && cline->type != CALL_INST &&
			   cline->type != JUMP_INST)
				continue;

			/* Check backwards for fill line */
			for(pline = cline->prev; pline; pline = pline->prev)
			{
				/* Skip if inst is save or sets control code */
				if(!INST(pline->type) || pline->type == SAVE_INST || 
				   insttypes[pline->instinfonum].setscc)
					continue;

				/* Check if inst can be used to fill */
				flag = TRUE;
				for(iline = pline->next; iline != cline && flag; 
					iline = iline->next)
				{
					if(iline->type == CALL_INST)
						continue;

					flag = !(varcommon(pline->sets, iline->uses) ||
					         varcommon(pline->uses, iline->sets));
				}

				/* Found a fill. Set up the fill */
				if(flag)
				{
					if(cline->type == JUMP_INST)
   					{
      					strcpy(cline->text,"\tba\t");
      					strcat(cline->text,cline->items[1]);
   					}
   					unhookline(pline);
                    hookupline(cblk, cline->next, pline);
					incropt(FILL_DELAY_SLOTS);
					break;
				}
			}
			
		}
	}
}