/* N.B. THIS IS NOT THE VERSION ON THE VAX.
         VARIOUS FIXES MADE!
*/

/************************************************************************
      Andrew C.R. Martin                            20.07.90

      Laboratory of Mathematical Biology            V1.1
      National Institue for Medical Research,
      The Ridgeway,
      Mill Hill,
      London,
      NW7 1AA

      This code is in the public domain. It may be copied and/or
      modified by anyone providing this notice is retained and 
      that the code is not sold for profit. Programs resulting
      from the use of this code may be sold commercially, providing
      suitable acknowledgements are included and that notice is
      given that this portion of the code is used with permission
      of the author.
*************************************************************************

   Revision History
   ================
   V1.0  20.07.90 Original
   V1.1  10.09.93 Cleaned up and ANSIed. Now compiles cleanly with
                  STRICT ANSI options. Still needs to be properly
                  documented.

*************************************************************************
   BATCH is a simple command language for creating repetitive batch 
   files, etc. Each command is introduced by a | character and variables 
   may also be created using this character. The language has only 9 
   basic commands:
   |SARRAY
   |SET
   |INC
   |DEC
   |STEP
   |REPEAT
   |FORMAT
   |FILE
   |END
   |EXIT
   and two associated commands
   |STRING
   |NUMBER
   The simplest of these is the |REPEAT |END pair. |REPEAT is followed
   by a variable name and two values. This is formally equvalent to the
   DO loop of FORTRAN or the for() loop of C. The |END command ends the
   set of lines to be repeated. Note that |REPEAT |END pairs may *NOT*
   be nested.
   For example:
   |REPEAT |I 1 5
   $WRITE SYS$OUTPUT "Hello |I "
   |END
   |EXIT
   
   would create a VAX DCL batch file thus:
   $WRITE SYS$OUTPUT "Hello 1"
   $WRITE SYS$OUTPUT "Hello 2"
   $WRITE SYS$OUTPUT "Hello 3"
   $WRITE SYS$OUTPUT "Hello 4"
   $WRITE SYS$OUTPUT "Hello 5"
   
   Had the |STEP command been issued before the |REPEAT command, the 
   variable would have been incremented by this value. The effect of 
   |STEP is removed once the |END command is found.
   e.g.
   |STEP 2
   |REPEAT |I 1 5
   $WRITE SYS$OUTPUT "Hello |I "
   |END
   |EXIT
   
   would create a VAX DCL batch file thus:
   $WRITE SYS$OUTPUT "Hello 1"
   $WRITE SYS$OUTPUT "Hello 3"
   $WRITE SYS$OUTPUT "Hello 5"
   
   The alternative to the |REPEAT |END pair is the |FILE |END pair. 
   The effect of this is similar except that variables are read from
   a file and the construct is repeated until the file ends. Before 
   using |FILE |END, the |FORMAT command must be given to describe the
   format of a line in the file and associate values with variables
   e.g.
   |FORMAT |STRING |A |NUMBER |B |NUMBER |C
   would expect each line of the file to contain a string followed by two
   numbers. These would be assigned to the variables |A, |B and |C
   resepctively.
   Suppose a file exists called "batch.dat" as follows:
   A 53 2
   B 47 5
   C 20 15
   The following commands would be used to create a batch file:
   |FORMAT |STRING |A |NUMBER |B |NUMBER |C
   |FILE batch.dat
   $WRITE sys$output "String|A  Values |B , |C "
   |END
   |EXIT
   
   This would result in the following batch file:
   $WRITE sys$output "StringA Values 53, 2"
   $WRITE sys$output "StringB Values 47, 5"
   $WRITE sys$output "StringC Values 20, 15"
   
   The |SARRAY command is used to create a string array:
   |SARRAY |MyArray
   Hello
   Goodbye World!
   |END
   
   Items in the array are numbered from 1 and accessed with a number or 
   numeric variable:
   |MyArray |a
   |MyArray 2
   
   Below is the contents of batch.inp, a demonstration input file.
   =============================Cut Here==================================
   =======================================================================
                     Demonstration input file for batch 
   =======================================================================
   
   Any lines which have no vertical line will be copied to the output file
   exactly as they are.
   
   Run this demo by typing:
   batch -ibatch.inp batch.out
   
   You also need to have the file batch.dat available.
   
   We can create an array of strings thus:
   |sarray |a
   Hello
   Goodbye
   |end
   Those lines will not go to the output file.
   
   Next we can create a group of lines to be repeated.
   The first of this set of lines will echo the number of times round the 
   loop. The second will echo the appropriate string from the array 
   created above.
   |repeat |b 1 2
   This is loop cycle |b
   I'm going to say: |a |b
   |end
   
   We can also create and manipulate variables directly:
   |set |c 1
   |c
   |inc |c
   |c
   |dec |c
   |c
   |set |c 10
   |c
   
   Let's echo the variables from the string array again:
   |a 1  |a 2
   Note that to get a space between these two words in the output, we 
   had to place *two* spaces in the input file. This is always the case 
   as the first space is eaten by the command interpreter in defining 
   the variable name. This, of course, lets us follow something 
   immediately without a space. For example:
   |step 2
   |repeat |d 1 10
   file|d .dat
   |end
   This example also illustrates the command to set the step size.
   
   Finally the file parsing option.
   First we need to create a template:
   |format |string |e |number |f |number |g
   Now specify a file loop with something to write:
   |file batch.dat
   String|e  Values |f ,|g
   |end
   |exit
   
   Notes:
   1. The file must end with a |exit command---anything after that will 
   be ignored.
   2. Commands are (currently) not allowed within |FILE or |REPEAT loops,
   so you can't put in |INC's or |DEC's.
   3. Consequently, loops cannot be nested.
   (2. and 3. will be fixed by making the commandline stuff recursive...)
   4. Variable names may be upto 19 characters long
   5. Each variable *MUST* be followed by a space. This space will be 
   removed in the resulting output string. Thus, if you wish a space to 
   appear, you must place 2 spaces in the command line.
   
   Below is the contents of batch.dat
   ==============================Cut Here=================================
   A 53 2
   B 47 5
   C 20 15
   ==============================Cut Here=================================
*************************************************************************/
/* Includes
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bioplib/MathType.h"
#include "bioplib/SysDefs.h"
#include "bioplib/parse.h"
#include "bioplib/macros.h"
#include "bioplib/general.h"
/************************************************************************/
/* Defines
*/
#define NCOMM         12
#define MAXSTRPARAM   3
#define MAXFLOATPARAM 1
#define MAXSTRLEN     104
#define MAXNAME       24
#define MAXINT        24
#define BUFFLEN       160
#define LF            10
#define CR            13

/************************************************************************/
/* Structures
*/
struct Variable
{
   struct Variable   *next;
   REAL             value;
   int               type;
   char              string[BUFFLEN];
   char              name[24];
}  ;
typedef struct Variable VarItem;

struct Line
{
   struct Line *next;
   char string[BUFFLEN];
}  ;
typedef struct Line LineItem;

struct StrArray
{
   struct Line       *string;
   struct StrArray   *next;
   int               nstring;
   char              name[20];
}  ;
typedef struct StrArray StrArrayItem;

/************************************************************************/
/* Prototypes
*/
int main(int argc, char **argv);
void CreateVar(VarItem *VarList,int type,char *name);
void SetNumVar(VarItem *VarList,char *name,REAL value);
void GetNumVar(VarItem *VarList,char *name,REAL *value);
void GetStringVar(VarItem *VarList,char *name,char *string);
void ReadLines(FILE *in,LineItem *LineList);
void WriteLines(FILE *out,LineItem *LineList,VarItem *VarList,
                StrArrayItem *SArray);
void EndString(char *string);
void MakeSArray(FILE *in,StrArrayItem *SArray,char *name);
int FindVar(VarItem *VarList,char *name);
int FindArray(StrArrayItem *SArray,char *name);
void WriteString(FILE *out,char *buffer,VarItem *VarList,
                 StrArrayItem *SArray);
void GetStrArray(StrArrayItem *SArray,char *name,int arrayidx,
                 char *string);
void ProcessFile(FILE *in,FILE *out,char *filename,char *interpret,
                 char name[MAXINT][MAXNAME],int icount,VarItem *VarList,
                 StrArrayItem *SArray);
void SetStrVar(VarItem *VarList,char *name,char *value);

/************************************************************************/
main(int argc, char **argv)
{
   char           outfile[160],
                  infile[160],
                  *buffpoin,
                  buffer[BUFFLEN],
                  buff2[BUFFLEN],
                  LoopVar[MAXNAME],
                  interpret[MAXINT],
                  intnames[MAXINT][MAXNAME],
                  *strparam[MAXSTRPARAM];
   REAL          floatparam[MAXFLOATPARAM],
                  fvalue;
   FILE           *in,
                  *out;
   KeyWd          keywords[NCOMM];
   int            i,
                  key,
                  step=1,
                  icount,
                  start,
                  stop,
                  nletters;
   VarItem        *VarList;
   LineItem       *LineList;
   StrArrayItem   *SArray;


   VarList  = (VarItem  *)malloc(sizeof(VarItem));
   VarList->next  = NULL;

   LineList = (LineItem *)malloc(sizeof(LineItem));
   LineList->next = NULL;

   SArray   = (StrArrayItem *)malloc(sizeof(StrArrayItem));
   SArray->next   = NULL;

   in  = stdin;
   out = NULL;

   /* Parse the command line                                            */
   argc--; argv++;
   while(argc)
   {
      if(!strncmp(argv[0],"-i",2))
      {
          buffpoin = argv[0]+2;
          strcpy(infile,buffpoin);
          if((in=fopen(infile,"r"))==NULL)
          {
             printf("Unable to open input file: %s\n",infile);
             exit(1);
          }
      }
      else if(!strncmp(argv[0],"-",1))
      {
          printf("Unknown switch %s\n",argv[0]);
          exit(1);
      }
      else
      {
         strcpy(outfile,argv[0]);
         if((out=fopen(outfile,"w"))==NULL)
         {
            printf("Unable to open output file %s\n",outfile);
            exit(1);
         }
      }
      argv++; argc--;
   }
   if(!out)
   {
      printf("Output file: ");
      scanf("%s",outfile);
      if((out=fopen(outfile,"w"))==NULL)
      {
         printf("Unable to open output file %s\n",outfile);
         exit(1);
      }
   }

   /* Initialise stuff for the command parser                           */
   for(i=0;i<NCOMM;i++)
      strparam[i] = (char *)malloc(MAXSTRLEN * sizeof(char));   
   MAKEKEY(keywords[0], "|EXIT",     NUMBER,0);
   MAKEKEY(keywords[1], "|STEP",     NUMBER,1);
   MAKEKEY(keywords[2], "|REPEAT",   STRING,3);
   MAKEKEY(keywords[3], "|END",      NUMBER,0);
   MAKEKEY(keywords[4], "|FORMAT",   NUMBER,0);
   MAKEKEY(keywords[5], "|STRING",   STRING,1);
   MAKEKEY(keywords[6], "|NUMBER",   STRING,1);
   MAKEKEY(keywords[7], "|FILE",     STRING,1);
   MAKEKEY(keywords[8], "|SARRAY",   STRING,1);
   MAKEKEY(keywords[9], "|SET",      STRING,1);
   MAKEKEY(keywords[10],"|INC",      STRING,1);
   MAKEKEY(keywords[11],"|DEC",      STRING,1);

   /* Repeat until the end of the input file                            */
   while(!feof(in))
   {
      /* Get a line from the input file                                 */
      memset(buffer,0,BUFFLEN);
      fgets(buffer,BUFFLEN,in);
      buffpoin = KillLeadSpaces(buffer); 

      /* Check if it's a command or variable                            */
      if(buffpoin[0] == '|')
      {
         GetString(buffpoin,buff2);
         /* See if it's a variable or an array                          */
         if(FindVar(VarList,buff2) || FindArray(SArray,buff2))
         {
            WriteString(out,buffer,VarList,SArray);
         }
         else
         {   /* Assume it's a command                                   */
            EndString(buffpoin);
            if((key=parse(buffpoin,NCOMM,keywords,floatparam,
                          strparam))==NULL)
               exit(0);

            switch(key)
            {
            case 1:   /* |STEP                                          */
               step = (int)floatparam[0];
               break;
            case 2:   /* |REPEAT                                        */
               /* Strip off the |REPEAT command from the command line   */
               buffpoin += GetString(buffpoin,buff2);
               buffpoin = KillLeadSpaces(buffpoin);
               /* Now get the variable name                             */
               buffpoin += GetString(buffpoin,LoopVar);
               /* and create the variable                               */
               CreateVar(VarList,NUMBER,LoopVar);
               /* Now get the start and stop points                     */
               buffpoin = KillLeadSpaces(buffpoin);
               GetParam(buffpoin,&fvalue,&nletters);
               start = (int)fvalue;
               buffpoin += nletters;
               buffpoin = KillLeadSpaces(buffpoin);
               GetParam(buffpoin,&fvalue,&nletters);
               stop  = (int)fvalue;
               /* Now read in the following lines until we get an
                  |END command                                          */
               ReadLines(in,LineList);
               /* Write them out, substituting variables                */
               if(start<stop)
               {
                  for(i=start; i<=stop; i+=step)
                  {
                     SetNumVar(VarList,LoopVar,(REAL)i);
                     WriteLines(out,LineList,VarList,SArray);
                  }
               }
               else
               {
                  for(i=start; i>=stop; i-=step)
                  {
                    SetNumVar(VarList,LoopVar,(REAL)i);
                    WriteLines(out,LineList,VarList,SArray);
                  }
               }
               step = 1;
               break;
            case 3:   /* |END --- this is actually read by ReadLines()
                         of MakeSArray()
                         so if seen here is out of place 
                      */
               printf("Command |END is out of context\n");
               break;
            case 4:   /* |FORMAT                                        */
               /* Strip off the |FORMAT command from the command line   */
               buffpoin += GetString(buffpoin,buff2);
               buffpoin = KillLeadSpaces(buffpoin);
               icount=0;
               while(*buffpoin)
               {
                  key=parse(buffpoin,NCOMM,keywords,floatparam,strparam);
                  switch(key)
                  {
                  case 0:
                  case 1:
                  case 2:
                  case 3:
                  case 4:
                  case 7:
                     printf("Command out of context ignored\n");
                     break;
                  case 5:   /* |STRING                                  */
                     /* Strip off the |STRING command from the command 
                        line 
                     */
                     buffpoin += GetString(buffpoin,buff2);
                     buffpoin = KillLeadSpaces(buffpoin);
                     /* Get the variable name                           */
                     buffpoin += GetString(buffpoin,buff2);
                     buffpoin = KillLeadSpaces(buffpoin);
                     /* Create the variable                             */
                     CreateVar(VarList,STRING,buff2);
                     /* Note this in the interpret array                */
                     strcpy(intnames[icount],buff2);
                     interpret[icount++] = 'S';
                     break;
                  case 6:   /* |NUMBER                                  */
                     /* Strip off the |NUMBER command from the command 
                        line 
                     */
                     buffpoin += GetString(buffpoin,buff2);
                     buffpoin = KillLeadSpaces(buffpoin);
                     /* Get the variable name                           */
                     buffpoin += GetString(buffpoin,buff2);
                     buffpoin = KillLeadSpaces(buffpoin);
                     /* Create the variable                             */
                     CreateVar(VarList,NUMBER,buff2);
                     /* Note this in the interpret array                */
                     strcpy(intnames[icount],buff2);
                     interpret[icount++] = 'D';
                     break;
                  default:
                     printf("Error in command\n");
                  }                  
               }
               break;
            case 5:   /* |STRING                                        */
            case 6:   /* |NUMBER                                        */
               printf("Command out of context ignored\n");
               break;
            case 7:   /* |FILE                                          */
               ProcessFile(in,out,strparam[0],interpret,intnames,
                           icount,VarList,SArray);
               break;
            case 8:   /* |SARRAY                                        */
               MakeSArray(in,SArray,strparam[0]);
               break;
            case 9:   /* |SET                                           */
               /* Create the variable if necessary                      */
               if(!FindVar(VarList,strparam[0]))
                  CreateVar(VarList,NUMBER,strparam[0]);
               /* Strip off the |SET command                            */
               buffpoin += GetString(buffpoin,buff2);
               buffpoin = KillLeadSpaces(buffpoin);
               /* Strip off the variable name                           */
               buffpoin += GetString(buffpoin,buff2);
               buffpoin = KillLeadSpaces(buffpoin);
               /* Now get the value                                     */
               GetParam(buffpoin,&fvalue,&nletters);
               /* And set it                                            */
               SetNumVar(VarList,strparam[0],fvalue);
               break;
            case 10:   /* |INC                                          */
               GetNumVar(VarList,strparam[0],&fvalue);
               fvalue += 1.0;
               SetNumVar(VarList,strparam[0],fvalue);
               break;
            case 11:   /* |DEC                                          */
               GetNumVar(VarList,strparam[0],&fvalue);
               fvalue -= 1.0;
               SetNumVar(VarList,strparam[0],fvalue);
               break;
            default:
               printf("Error in command\n");
            }
         }  /* End of starts with |, but not Variable                   */
      }
      else /* Not a command or var (no |), so just copy it to out       */
      {
         WriteString(out,buffer,VarList,SArray);
      }
   }
}

/************************************************************************/
void CreateVar(VarItem *VarList,int type,char *name)
{
   VarItem *p,*q;
   static int first=TRUE;

   /* Go to the end of the list                                         */
   for(p=VarList; p; NEXT(p))q=p;
   p=q;

   /* Allocate space for a new variable                                 */
   if(!first)
   {
      ALLOCNEXT(p,VarItem);
   }

   /* And set its values                                                */
   strcpy(p->name,name);
   p->type = type;
   p->value = 0.0;
   strcpy(p->string," ");
   p->next = NULL;
   first=FALSE;
}
/************************************************************************/
void SetNumVar(VarItem *VarList,char *name,REAL value)
{
   VarItem *p;
   int     found=FALSE;

   /* Search the list                                                   */
   for(p=VarList; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   if(!found)
   {
      printf("Unknown variable: %s\n",name);
      exit(1);
   }
   if(p->type != NUMBER)
   {
      printf("Variable %s is a string not a number\n",name);
      exit(1);
   }
   p->value = value;
}
/************************************************************************/
void GetNumVar(VarItem *VarList,char *name,REAL *value)
{
   VarItem *p;
   int     found=FALSE;

   /* Search the list                                                   */
   for(p=VarList; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   if(!found)
   {
      printf("Unknown variable: %s\n",name);
      exit(1);
   }
   if(p->type != NUMBER)
   {
      printf("Variable %s is a string not a number\n",name);
      exit(1);
   }
   *value = p->value;
}
/************************************************************************/
void GetStringVar(VarItem *VarList,char *name,char *string)
{
   VarItem *p;
   int     found=FALSE;

   /* Search the list                                                   */
   for(p=VarList; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   if(!found)
   {
      printf("Unknown variable: %s\n",name);
      exit(1);
   }
   switch(p->type)
   {
   case NUMBER:
      sprintf(string,"%d",(int)p->value);
      break;
   case STRING:
      strcpy(string,p->string);
   }
}
/************************************************************************/
void ReadLines(FILE *in,LineItem *LineList)
{
   LineItem *p;
   char     buffer[BUFFLEN],
            buff2[BUFFLEN],
            *buffpoin;
   int      i= -1;

   p=LineList;
   while(!feof(in))
   {
      memset(buffer,0,BUFFLEN);
      fgets(buffer,BUFFLEN,in);
      i++;
      buffpoin = KillLeadSpaces(buffer);
      StringToUpper(buffpoin,buff2);
      if(!strncmp(buff2,"|EN",3))
         break;
      /* Allocate space for a new variable                              */
      if(i)
      {
         ALLOCNEXT(p,LineItem);  /* V1.1 Was VarItem                    */
      }
      else
      {
         p=LineList;
      }
      /* And set its values                                             */
      strcpy(p->string,buffer);
      p->next = NULL;
   }
}
/************************************************************************/
void WriteLines(FILE *out,LineItem *LineList,VarItem *VarList,
           StrArrayItem *SArray)
{
   LineItem *p;
   char     *buffpoin;

   for(p=LineList;p;NEXT(p))
   {
      buffpoin=p->string;
      WriteString(out,buffpoin,VarList,SArray);
   }
}
/************************************************************************/
void EndString(char *string)
{
   char *p;

   for(p=string;(*p!=LF && *p!=CR && *p); p++);
   if(*p==LF || *p==CR) *p='\0';
}
/************************************************************************/
void MakeSArray(FILE *in,StrArrayItem *SArray,char *name)
{
   char         buffer[BUFFLEN],
                buff2[BUFFLEN],
                *buffpoin;
   StrArrayItem *p,*p1;
   LineItem     *q;
   int          j= -1;
   static int   i= -1;

   /* Go to the end of the SArray list                                  */
   for(p=SArray;p;NEXT(p))p1=p;
   /* Allocate space for a new variable                                 */
   if(++i)
   {
      ALLOCNEXT(p1,StrArrayItem);
      p=p1;
   }
   else
   {
      p=SArray;
   }
   /* Set the variable name                                             */
   strcpy(p->name,name);
   /* Set q as a pointer to the string list                             */
   q=p->string;

   /* Fill in the strings                                               */
   while(!feof(in))
   {
      fgets(buffer,BUFFLEN,in);
      EndString(buffer);
      buffpoin = KillLeadSpaces(buffer);
      StringToUpper(buffpoin,buff2);
      if(!strncmp(buff2,"|EN",3))     /* Find the |END                  */
         break;
      /* Allocate space for the string                                  */
      if(++j)
      {
         ALLOCNEXT(q,LineItem);
      }
      else
      {
         q=(LineItem *)malloc(sizeof(LineItem));
         p->string = q;
      }

      /* And set its values                                             */
      strcpy(q->string,buffer);
      q->next = NULL;
   }
   p->nstring = j+1;
}
/************************************************************************/
int FindVar(VarItem *VarList,char *name)
{
   VarItem *p;
   int     found=FALSE;

   /* Search the list                                                   */
   for(p=VarList; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   return(found);
}
/************************************************************************/
int FindArray(StrArrayItem *SArray,char *name)
{
   StrArrayItem *p;
   int          found=FALSE;

   /* Search the list                                                   */
   for(p=SArray; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   return(found);
}
/************************************************************************/
void WriteString(FILE *out,char *buffer,VarItem *VarList,
                 StrArrayItem *SArray)
{
   char     *buffpoin,
            string[BUFFLEN],
            *strpoin,
            varname[20], *varpoin,
            param[20],   *parampoin;
   int      arrayidx;
   REAL    x;

   buffpoin=buffer;
   while(*buffpoin)
   {
      if(*buffpoin == '|')
      {  /* It's a variable or an array                                 */
         varpoin=varname;
         while((*buffpoin != ' ')&&(*buffpoin != LF)&&(*buffpoin != CR))
            *(varpoin++) = *(buffpoin++);
         if((*buffpoin == LF)||(*buffpoin == CR)) buffpoin--;
         *varpoin = '\0';
         /* varname now has the name of the variable                    */
         /* See if it's an array                                        */
         if(FindArray(SArray,varname))
         {
             /* Get the parameter out of the buffer                     */
             parampoin=param;
             buffpoin = KillLeadSpaces(buffpoin);
             while((*buffpoin != ' ') && (*buffpoin != LF) &&
                   (*buffpoin != CR))
                *(parampoin++) = *(buffpoin++);
             if((*buffpoin == LF)||(*buffpoin == CR)) buffpoin--;
             *parampoin = '\0';
             if(param[0]=='|')
             {
                GetNumVar(VarList,param,&x);
             }
             else
             {
                sscanf(param,"%f",&x);
             }
             arrayidx = (int)x;
             GetStrArray(SArray,varname,arrayidx,string);
         }
         else
         {
            GetStringVar(VarList,varname,string);
         }
         for(strpoin=string;*strpoin;strpoin++)
            putc(*strpoin,out);
      }
      else
      {
         putc(*buffpoin,out);
      }
      buffpoin++;
   }
}


/************************************************************************/
void GetStrArray(StrArrayItem *SArray,char *name,int arrayidx,
                 char *string)
{
   StrArrayItem *p;
   LineItem     *q;
   int          found=FALSE,
                j;

   /* Search the list                                                   */
   for(p=SArray; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   if(!found)
   {
      printf("Unknown array variable: %s\n",name);
      exit(1);
   }
   arrayidx--;
   if(arrayidx >= p->nstring)
   {
      printf("Array index out of bounds. Maximum for array %s is %d\n",
              name,p->nstring);
      exit(1);
   }
   q=p->string;
   for(j=0;j<arrayidx;j++) NEXT(q);
   strcpy(string,q->string);
}

/************************************************************************/
void ProcessFile(FILE *in,FILE *out,char *filename,char *interpret,
                 char name[MAXINT][MAXNAME],int icount,VarItem *VarList,
                 StrArrayItem *SArray)
{
   FILE     *datafile;
   char     buffer[BUFFLEN],
            buff2[BUFFLEN],
            *buffpoin;
   LineItem *LineList;
   int      i,
            nletters,
            ch;
   REAL     x;

   LineList = (LineItem *)malloc(sizeof(LineItem));
   LineList->next = NULL;

   /* Open the file                                                     */
   if((datafile=fopen(filename,"r"))==NULL)
   {
      printf("Unable to open datafile %s\n",datafile);
      exit(1);
   }

   /* Read in the lines to repeat                                       */
   ReadLines(in,LineList);

   /* Write them out, substituting variables                            */
   /* Step through the data file                                        */
   ch=getc(datafile);
   while(ch!=EOF)
   {
      ungetc(ch,datafile);
      buffpoin = buffer;
      /* Read in a line                                                 */
      i=0;
      while((ch!=LF)&&(ch!=CR))
      {
         ch=getc(datafile);
         buffpoin[i++]=(char)ch;
      }
      /* Get the values from the buffer and set the variables           */
      buffpoin=buffer;
      for(i=0;i<icount;i++)
      {
         if(interpret[i]=='S')
         {
            buffpoin += GetString(buffpoin,buff2);
            buffpoin=KillLeadSpaces(buffpoin);
            SetStrVar(VarList,name[i],buff2);
         }
         else
         {
            GetParam(buffpoin,&x,&nletters);
            buffpoin += nletters;
            buffpoin=KillLeadSpaces(buffpoin);
            SetNumVar(VarList,name[i],x);
         }
      }
      WriteLines(out,LineList,VarList,SArray);
      ch=getc(datafile);
   }
}
/************************************************************************/
void SetStrVar(VarItem *VarList,char *name,char *value)
{
   VarItem *p;
   int     found=FALSE;

   /* Search the list                                                   */
   for(p=VarList; p; NEXT(p))
   {
      if(!strcmp(p->name,name))
      {
         found = TRUE;
         break;
      }
   }
   if(!found)
   {
      printf("Unknown variable: %s\n",name);
      exit(1);
   }
   if(p->type != STRING)
   {
      printf("Variable %s is a number not a string\n",name);
      exit(1);
   }
   strcpy(p->string,value);
}
