/* 
 * Author     : David Miller
 * Course     : COP6622 Advanced Topics in Compilers 
 * Professor  : Dr. Whalley
 * Assignment : Peephole Optimization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"
#include "misc.h"
#include "vars.h"
#include "opts.h"
#include "io.h"

int checkrules(struct assemline *, int, char *, int);
void initmappings();

/* Tracks num of peeprules read in and how many times each is used */
extern int numpeeprules;
extern int numrulesapplied[MAXRULES];
/* Data structs holding rules, rule starting positions, and wildcard mappings */
char rules[500][MAXLINE];
int ruleindex[100];
char mapping[10][MAXLINE];

/*
 * readinrules - read in the peephole rules from rules.txt in current dir
 */
void readinrules()
{
   char line[MAXLINE];
   char (*lineptr)[MAXLINE] = rules;
   int linenum = 0;

   ruleindex[0] = 0;
   numpeeprules = 0;
   FILE *fp = fopen("rules.txt", "r");
   if(fp == NULL)
      fprintf(stderr, "File \"rules.txt\" not in dir\n");

   /* Assign rules[i] to line i */
   while(fgets(line, MAXLINE, fp) != NULL)
   {
      strcpy(lineptr[0], line);
      if(strcmp(lineptr[0], "=\n") == 0)
      {
         numpeeprules++;
         ruleindex[numpeeprules] = linenum + 3;  
      }
      lineptr++;
      linenum++;
   }
   fclose(fp);
}

/*
 * applypeeprules - apply peephole rules to the function
 */
void applypeeprules(int *changes)
{
   int i;

   /* No rules to apply peephole optimization */
   if(numpeeprules == 0)
      return;

   extern struct bblk *top;
   struct bblk *cblk;
   struct assemline *cline;

   /* Iterate over all lines to test if they qualify for rule replacement */
   for(cblk = top; cblk; cblk = cblk->down)
      for(cline = cblk->lines; cline; cline = cline->next)
         if(INST(cline->type))
         {
            initmappings();
            for(i = 0; i < checkrules(cline, 0, rules[0], 0); i++)
            {
               cline = cline->next;
               delline(cline->prev);
            }
         }
} 
    
/*
 * checkrules - recursively determines if a contiguous set of lines qualifies
 *              for a peephole replacement. Returns num of lines to be deleted
 *              line    : line in code we are testing
 *              rulenum : rulnum being tested against
 *              rulepos : line in rules.txt we are testing against
 *              numlines: tracks how many lines are mathced against a rule
 */
int checkrules(struct assemline *line, int rulenum, char *rulepos, int numlines)
{
   /* No more rules to test against */
   if(rulenum == numpeeprules)
      return 0;

   /* Found mathcing lines for the rulenum-th rule */
   if(strcmp(rulepos, "=\n") == 0)
   {
      int len, index = 0, i = 0, j = 1;
      char replace[MAXLINE] = "", comma[2] = ",", end[2] = "\0";
      char *ptr = replace;
      /* Build inst of replacement line */
      while(rules[ruleindex[rulenum+1]-2][i] != '$')
      {
         *ptr = rules[ruleindex[rulenum+1]-2][i];
         ptr++;
         i++;
      }
      /* Build wildcard mappings of replacement line */
      while(rules[ruleindex[rulenum+1]-2][i] == '$')
      {
         len = 1;
         index = rules[ruleindex[rulenum+1]-2][i+1] - '0';
         while(mapping[index][j] >= '0' && mapping[index][j] <= 'z')
         {
            len++;
            j++;
         }
         strncpy(ptr, strcat(mapping[index], comma), len + 1);
         ptr += len + 1;
         i += 3;
      }
      /* Terminate the replacement line and alloc space for it */
      strncpy(ptr-1, end, 1);
      j = 0;
      len = 0;
      while(replace[j] != '\0')
      {
         len++;
         j++;
      }
      char *result = alloc(len + 1);
      strncpy(result, replace, len);
      insline(line->blk, line, result);
      setupinstinfo(line->prev);
      numrulesapplied[++rulenum]++;

      /* From first line passed in, delete the preceding numlines inclusively */
      return numlines;
   }

   /* Skip define and comment lines */
   if(line->type == DEFINE_LINE || line->type == COMMENT_LINE)
      return checkrules(line->next, rulenum, rulepos, numlines);

   /* Parse line of rules */
   char inst[8] = "", wilds[8] = "", deads[8] = "", regs[8] = "";
   int tabcount = 0, i = 0, j = 0;
   while(TRUE)
   {
      /* End of rule line */
      if(*rulepos == '\n')
         break;
      /* Seperating character for the different fields */
      if(*rulepos == '\t')
      {
         tabcount++;
         rulepos++;
         i = 0;
         continue;
      }
      /* Skip commas */
      if(*rulepos == ',')
      {
         rulepos++;
         continue;
      }
      /* Parse inst */
      if(tabcount == 1)
      {
         inst[i] = rulepos[0];
         rulepos++;
         i++;
      }
      /* Parse wildcards */
      if(*rulepos == '$' && tabcount == 2)
      {
         wilds[i] = *(rulepos + 1);
         rulepos += 2;
         i++;
      }
      /* Parse deads */
      if(*rulepos == '$' && tabcount == 3)
      {
         deads[i] = *(rulepos + 1);
         rulepos += 2;
         i++;
      }
      /* Parse is_regs */
      if(*rulepos == '$' && tabcount == 4)
      {
         regs[i] = *(rulepos + 1);
         rulepos += 2;
         i++;
      }
   }

   /* Check if insts match */
   if(strcmp(line->items[0], inst) != 0)
   {
      rulenum++;
      return checkrules(line, rulenum, rules[ruleindex[rulenum]], 0);
   }
   
   /* Check if deads match */
   for(i = 0; i < 8 && deads[i] != '\0'; i++)
      for(j = 0; j < 8 && wilds[j] != '\0'; j++)
         if(deads[i] == wilds[j])
            if(regexists(line->items[j+1], line->deads) == FALSE)
            {
               rulenum++;
               return checkrules(line, rulenum, rules[ruleindex[rulenum]], 0);
            }

   /* Check if is_regs match */
   for(i = 0; i < 8 && regs[i] != '\0'; i++)
      for(j = 0; j < 8 && wilds[j] != '\0'; j++)
         if(regs[i] == wilds[j])
            if(isreg(line->items[j+1]) == FALSE)
            {
               rulenum++;
               return checkrules(line, rulenum, rules[ruleindex[rulenum]], 0);
            }

   /* Assign the mappings f(wildcard) --> reg */
   for(i = 0; i < 8 && wilds[i] != '\0'; i++)
   {
      if(mapping[wilds[i]-'0'][0] == '\0')
      {
         strcpy(mapping[wilds[i]-'0'], line->items[i+1]);
         continue;
      }
      if(strcmp(mapping[wilds[i]-'0'], line->items[i+1]) != 0)
         return 0;
   }

   /* Get next line in rule */
   while(*rulepos != '\t' && *rulepos != '=')
      rulepos++;

   /* Figure this case out, where line->next is NULL but rule can be applied */
   if(!line->next)
   {
      if(*rulepos == '=')
      {
         fprintf(stderr,
           "====> Opt opportunity for rule %d not applied <====\n", ++rulenum);
      }
      return 0;
   }

   /* Make sure assemlines are in the same block */
   if(line->blk->num != line->next->blk->num)
      return 0;

   /* Check next assemline against next line in rule */
   return checkrules(line->next, rulenum, rulepos, ++numlines);

   /* Return 0 as safety net */
   return 0;

}

/* 
 * initmappings - initialize mappings for wildcards
 */
void initmappings()
{
   int k;

   for(k = 0; k < 10; k++)
      mapping[k][0] = '\0';
}