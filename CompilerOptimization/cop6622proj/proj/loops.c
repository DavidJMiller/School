/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Loop Detection
 */

#include <stdio.h>
#include "opt.h"
#include "misc.h"
#include "analysis.h"
#include "vect.h"

struct loopnode *loops;    /* head of linked list of loops */

void computedom();
void computenatloop();
void dfs(struct loopnode *, struct bblk *);
void bubblesort();
void swap(struct loopnode *, struct loopnode *, struct loopnode *);

/*
 * findloops - locate loops in the program and build the loop data structure
 */
void findloops()
{
   /* Free up loop structures */
   freeloops();

   /* Compute dominators */
   computedom();

   /* Compute natural loops */
   computenatloop();

   /* Sort blocks in loops */
   bubblesort();
}

/* 
* Compute dominators for each basic block
*/
void computedom()
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct blist *clist;
   bvect tmp;
   int flag;

   /* Initialize dom for each block */
   cblk = top;
   cblk->dom = bnone();
   binsert(&cblk->dom, cblk->num);
   for(cblk = top->down; cblk; cblk = cblk->down)
      cblk->dom = ball();

   /* While dom sets are still being changed */
   do
   {
      flag = 0;
      tmp = ball();

      /* Update dom set for each block */
      for(cblk = top->down; cblk; cblk = cblk->down)
      {
         bcpy(&tmp, cblk->dom);
         for(clist = cblk->preds; clist; clist = clist->next)
            binter(&cblk->dom, clist->ptr->dom);
         binsert(&cblk->dom, cblk->num);
         flag |= !bequal(cblk->dom, tmp);
      }
   }while(flag);
}

/*  
 * Compute natural loops via backedge traversal
 */
void computenatloop()
{
   extern struct bblk *top;
   struct bblk *cblk;
   struct blist *clist;

   /* Iterate over all basic blocks */
   for(cblk = top->down; cblk; cblk = cblk->down)
   {
      /* Iterate over each block's successors */
      for(clist = cblk->succs; clist; clist = clist->next)
      {
         /* Not a backedge */
         if(!bin(cblk->dom, clist->ptr->num))
            continue;

         /* Start a new loop */
         if(!clist->ptr->loop)
            clist->ptr->loop = newloop();

         /* Construct loop and loopnest levels for each block in loop */
         addtoblist(&clist->ptr->loop->blocks, cblk);
         addtoblist(&clist->ptr->loop->blocks, clist->ptr);
         clist->ptr->loop->header = clist->ptr;
         clist->ptr->loop->header->loopnest = 1;
         dfs(clist->ptr->loop, cblk);
      }
   }
}

/* 
 * Recursive Depth-First Search too add all blocks between head and tail of loop
 * and assign loopnest level
 */
void dfs(struct loopnode *loop, struct bblk *node)
{
   struct blist *clist;

   /* Back at the header of loop */
   if(node->num == loop->header->num)
      return;

   /* Increment node loopnest and add node preds until we get back to header */
   node->loopnest += 1;
   for(clist = node->preds; clist; clist = clist->next)
      if(!inblist(loop->blocks, clist->ptr))
      {
         addtoblist(&loop->blocks, clist->ptr);
         dfs(loop, clist->ptr);
      }

   return;
}

/* 
 * Bubbesort loops by nest-level (most deeply nest first). Break ties via header
   block num (lower block num first). Terrible time complexity but easy
   implementation :)
 */
void bubblesort()
{
   extern struct loopnode *loops;
   struct loopnode *loopptr, *lptr;
   int flag;

   /* Empty list and single element list are vacuously sorted */
   if(!loops || !loops->next)
      return;

   /* Keep sort element-wise until list is completely sorted*/
   lptr = NULL;
   do
   {
      flag = 0;
      loopptr = loops;
      /* Keep swapping adjacent elements if l < r or l = r and l_num > r_num */
      while(loopptr->next != lptr)
      {
         if(loopptr->header->loopnest < loopptr->next->header->loopnest ||
           (loopptr->header->loopnest ==  loopptr->next->header->loopnest &&
            loopptr->header->num > loopptr->next->header->num))
         {
            swap(loops, loopptr, loopptr->next);
            flag = 1;
         }
         loopptr = loopptr->next;
      }
      lptr = loopptr;
   }while(flag);
}

/*  
 * Helper funcion for bubblesort to swap to elements in linked list 
 */
void swap(struct loopnode *header, struct loopnode *a, struct loopnode *b)
{
   struct bblk *btmp;
   struct blist *bltmp;

   /* Swap node data rather than node ptrs (it is less messy this way) */
   btmp = a->header;
   a->header = b->header;
   b->header = btmp;
   bltmp = a->blocks;
   a->blocks = b->blocks;
   b->blocks = bltmp;
}