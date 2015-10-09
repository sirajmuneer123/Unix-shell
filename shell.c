/*simple unix shell programming....*/
#include"header.h"
int readL(char *wptr[],char *script[],char *p,char rchar[]);
void exe(char *wptr[],char *script[],int nwords,char dir[],char rchar[]);
void redirect(char *wptr[],char *script[],char dir[],char rchar[],int nwords);
void piping(char *wptr[]);
void create2Darray(char *multy[][20],char *wptr[],int *c_index,int *a_index);
main()
{
	char rchar[3],line[100];
	int i,pid,nwords;
	char *script[20];
	char *wptr[20];
	char dir[50];
	char *p;
	while(1){
		printf("[siraj@shell]$ ");
		fgets(line,sizeof(line),stdin); /* read the input file*/
		if(line[0]=='\n')   /* if input is newline */
			continue;
	        p=strtok(line,"\n");

		nwords=readL(wptr,script,p,rchar);
		strcpy(dir,"/bin/");
		strcat(dir,wptr[0]);
	
		exe(wptr,script,nwords,dir,rchar);/* ececute the commands*/
	}	
}/* execute function */
void exe(char *wptr[],char *script[],int nwords,char dir[],char rchar[])
{
	int pid,re,in,out;
	pid=fork();
	switch(pid){
		case -1:
			fprintf(stderr,"fork killed....\n");
			break;
		case 0:
			if(rchar[0]=='>'|rchar[0]=='<'|rchar[0]=='@'){ /* checking for redirection */
				redirect(wptr,script,dir,rchar,nwords);
				break;
			}else if(rchar[0]=='|'){ /* checking for piping */
				piping(wptr);
				break;
			}else
				execvp(dir,script); /* exec other simple commands */
				printf("sh: %s :no command found\n",wptr[0]);
			break;
		default:
			wait(&pid);
			break;
	}
}/* piping function */
void piping(char *wptr[])
{	
	int i,j;
	char *multy[20][20];/* 2D array for pipe command */
	int pfd1[2],pfd2[2]; 
	int pid;
	int c_index=0;  /*  number of commands...*/
	int a_index=0;  /* number of arguments...*/
	
	create2Darray(multy,wptr,&c_index,&a_index); /* create a 2dimensional array... */
	
	for (i=0; i<c_index; i++) {
		if (i % 2 != 0)   /* calling different file dicreptor for odd or even*/
			pipe(pfd1); 
		else
			pipe(pfd2); 
	pid = fork();
	switch (pid) {
		case -1: /* fork error checking..*/
			if (i != c_index - 1) {
				if (i % 2 != 0)
					close(pfd1[1]);
				else
					close(pfd2[1]);
			}
			fprintf(stderr, "fork failed!\n");
			break;
		case 0:     /* piping...*/ 
			if (i == 0)
				dup2(pfd2[1], 1);
			else if (i == c_index - 1) {
				if (c_index % 2 != 0)
					dup2(pfd1[0], 0);
				else
					dup2(pfd2[0], 0);
			}else {
				if (i % 2 != 0) {
					dup2(pfd2[0], 0);
					dup2(pfd1[1], 1);
				}else {
					dup2(pfd1[0], 0);
					dup2(pfd2[1], 1);
				}	
			}
			int ex;
			ex=execvp(multy[i][0], multy[i]);
			if(ex == -1) { /* errror checking and exec the commands */	
			fprintf(stderr, "exec function failed!\n");
				exit(0);
			}
			break;	
			
		default:
			break;
		}
	/* close all pfd....*/
	if (i == 0) 
		close(pfd2[1]);
	else if (i == c_index - 1) {
		if (c_index % 2 != 0)
			close(pfd1[0]);
		else
			close(pfd2[0]);
	}else{
		if (i % 2 != 0) {
			close(pfd2[0]);
			close(pfd1[1]);
		}else {
			close(pfd1[0]);
			close(pfd2[1]);
		}
	}
	wait(&pid);
     }
}
void create2Darray(char *multy[][20],char *wptr[],int *c_index,int *a_index)
{
	int i,j;
       // char *multy[15][15];/* 2D array for pipe command */
        int m,n;
        int pfd1[2],pfd2[2];
        int pid;
        for(m=0;m<15;m++)               /* null the araay*/
                for(n=0;n<15;n++)
                        multy[m][n]=0;

        for(j=0;wptr[j]!=NULL;j++){             /* creating 2D arry for pipe*/
                if (strcmp(wptr[j], "|") == 0) {
                        multy[*c_index][*a_index] = 0;
                       *a_index = 0;
                        (*c_index)++;
                }else {
                        multy[*c_index][*a_index] = wptr[j];
                        (*a_index)++;
                }
        }
        multy[*c_index][*a_index] = 0;
        (*c_index)++;
}
/*  redirection function definition*/
void redirect(char *wptr[],char *script[],char dir[],char rchar[],int nwords)
{
	int fd,in,i,j=0,fd2;
	char *temp[20];

	for(i=0;strcmp(wptr[i],"<")!=0 && strcmp(wptr[i],">")!=0 && strcmp(wptr[i],">>")!=0;i++)
		temp[j++]=wptr[i];
	temp[j]=NULL;

	if(rchar[0]=='>'){  /* cheking for '>' redirection */
        	fd=open(wptr[nwords-1],O_RDWR|O_CREAT|O_TRUNC,0666);
                close(STDOUT_FILENO);
                dup(fd);
		execvp(dir,temp);

        }else if(rchar[0]=='<'){ /*checking for '<' redirection */
		 execvp(dir,script);

	}else if(rchar[0]=='@'){ /* checking for '>>' redirection ('@' representing '>>')*/
		fd2=open(wptr[nwords-1],O_APPEND|O_WRONLY,0666);
		close(STDOUT_FILENO);
		dup(fd2);
		execvp(dir,temp);
	}
}
/* read the input commands */
int readL(char *arg[],char *script[],char *p,char rchar[])
{

	int i=0,j,in=0,ri=0;
	char *temp;
	rchar[0]='\0';
	temp=strtok(p," "); /*tokanising with space*/
	while(temp!=NULL){	
				
		arg[i++]=temp;
		temp=strtok(NULL," ");
	}
	arg[i]=NULL;
	
	for(j=0;arg[j]!=NULL;j++)  /* checking for redirection and piping*/
		if(strcmp(arg[j],"<")!=0 && strcmp(arg[j],">")!=0 && strcmp(arg[j],"|")!=0 && strcmp(arg[j],">>")!=0)
			script[in++]=arg[j];
		else
			if(strcmp(arg[j],">>")==0)
				rchar[0]='@';
			else
				rchar[0]=arg[j][0];
	script[in]=NULL;
	rchar[1]='\0';
	return i;
}

