/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Eliminating Unconditional Jumps by Reversing Branches
 */

#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "misc.h"
#include "opts.h"
#include "flow.h"

int single_inst(struct bblk *);
char *reverse(char *);

/*
 * reversebranches - avoid jumps by reversing branches
 */
void reversebranches()
{
  extern struct bblk *top;
  struct bblk *itrblk;
  char *rbranch;
  char newline[MAXLINE];

  /* Iterate over all basic blocks */
  for(itrblk = top; itrblk; itrblk = itrblk->down)
  {
    /* Skip empty blocks */
    if(!(itrblk->lineend))
      continue;
    /* Check for suitable conditions */
    if(itrblk->lineend->type == BRANCH_INST && 
       single_inst(itrblk->down) == 1 &&
       itrblk->down->lineend->type == JUMP_INST && 
       itrblk->succs->next->ptr == itrblk->down->down)
    {
      /* Build new assemline and update branch type and target */
      rbranch = reverse(itrblk->lineend->items[0]);
      sprintf(newline, "\t%s\t%s", rbranch, itrblk->down->lineend->items[1]);
      itrblk->lineend->text = allocstring(newline);
      strcpy(itrblk->lineend->items[0], rbranch);
      strcpy(itrblk->lineend->items[1], itrblk->down->lineend->items[1]);

      /* Update itrblk fall thru and branch target */
      addtoblist(&(itrblk->succs->next->next), itrblk->down->succs->ptr);
      addtoblist(&(itrblk->down->succs->ptr->preds), itrblk);
      /* Eliminate unconditional jump and fix flow of its pred and succs */
      deleteblk(itrblk->down);
      itrblk->down = itrblk->succs->ptr;
      itrblk->down->up = itrblk;

      incropt(REVERSE_BRANCHES);
    }
  }
  check_cf();
}

/* Returns 1 if block has single instruction and 0 otherwise */
int single_inst(struct bblk *blk)
{
  struct assemline *line = blk->lines;

  while(!INST(line->type))
    line = line->next;

  for(line = line->next; line; line = line->next)
    if(INST(line->type))
      return 0;

  return 1;
}

/* Get the reverse of a branch */
char *reverse(char *branch)
{
  char *rbranch;

  /* a = b to a != b and vice versa */
  if(strcmp("be", branch) == 0)
    rbranch = allocstring("bne");
  else if(strcmp("bne", branch) == 0)
    rbranch = allocstring("be");
  /* a > b to a <= b and vice versa */
  else if(strcmp("bg", branch) == 0)
    rbranch = allocstring("ble");
  else if(strcmp("ble", branch) == 0)
    rbranch = allocstring("bg");
  /* a < b to a >= b and vice versa */
  else if(strcmp("bl", branch) == 0)
    rbranch = allocstring("bge");
  else if(strcmp("bge", branch) == 0)
    rbranch = allocstring("bl");
  /* a > b to a <= b (unsigned) and vice versa */
  else if(strcmp("bgu", branch) == 0)
    rbranch = allocstring("bleu");
  else if(strcmp("bleu", branch) == 0)
    rbranch = allocstring("bgu");
  /* Was not a branch */
  else
    rbranch = NULL;

  return rbranch;
}