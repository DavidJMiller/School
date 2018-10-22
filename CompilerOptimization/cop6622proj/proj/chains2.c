/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Branch Chaining
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "misc.h"
#include "opts.h"
#include "flow.h"

int singleinst(struct bblk *);

/* Remove branch chains */
void remvbranchchains()
{
  struct bblk *itrblk, *sblk;
  extern struct bblk *top;
  char *jumptarget;

  /* Iterate over all basic blocks */
  for(itrblk = top; itrblk; itrblk = itrblk->down)
  {
    /* Disregard empty blocks */
    if(itrblk->lineend)
    {
      /* Last line is transfer of control */
      if(itrblk->lineend->type == BRANCH_INST || 
         itrblk->lineend->type == JUMP_INST)
      {
        /* Get succ and check if it is empty */
        if(itrblk->lineend->type == BRANCH_INST)
          sblk = itrblk->succs->next->ptr;
        else
          sblk = itrblk->succs->ptr;
        if(!sblk->lineend)
          continue;
        /* Check if succ is chainable jump */
        if(sblk->label && sblk->lineend->type == JUMP_INST && 
           singleinst(sblk) == 1)
        {
          /* Remove start of chains from chainable block preds */
          delfromblist(&sblk->preds, itrblk);
          /* Remove chainable block from start of chain succs */
          delfromblist(&itrblk->succs, sblk);
          /* Traverse the chain */
          while(sblk->succs->ptr->lineend && 
                sblk->succs->ptr->lineend->type == JUMP_INST && 
                singleinst(sblk->succs->ptr) == 1)
            sblk = sblk->succs->ptr;
          /* Found end of chain so construct new line with jump target */
          jumptarget = allocstring(sblk->lineend->items[1]);
          char newline[MAXLINE];
          sprintf(newline, "\t%s\t%s", itrblk->lineend->items[0], jumptarget);
          strcpy(itrblk->lineend->items[1], jumptarget);
          itrblk->lineend->text = allocstring(newline);
          /* Add start of chain to end of chain preds */
          addtoblist(&sblk->succs->ptr->preds, itrblk);
          /* Add as second successor if branch */
          if(itrblk->lineend->type == BRANCH_INST)
            addtoblist(&itrblk->succs->next, sblk->succs->ptr);
          /* Add as only successor if jump */
          else
            addtoblist(&itrblk->succs, sblk->succs->ptr);
          /* Increment number of branch optimizations */
          incropt(BRANCH_CHAINING);
        }
      }
    }
  }
  check_cf();
}

/* Returns 1 if block has single instruction and 0 otherwise */
int singleinst(struct bblk *blk)
{
  struct assemline *line = blk->lines;

  while(!INST(line->type))
    line = line->next;

  for(line = line->next; line; line = line->next)
    if(INST(line->type))
      return 0;

  return 1;
}