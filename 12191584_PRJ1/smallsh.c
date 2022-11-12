#include "smallsh.h"

static char inpbuf[MAXBUF];
static char tokbuf[2*MAXBUF];
// inpbuf에서 token 단위로 분리

static char *ptr = inpbuf;
static char *tok = tokbuf;

														 
static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

int userin(char* p){
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	char buf[100];

	getcwd(buf,100);
	printf("%s\n",buf);

	printf("%s", p);
	count = 0;

	while(1){
		if((c = getchar()) == EOF)
			return EOF;
		if(count < MAXBUF)
			inpbuf[count++] = c;
		if(c == '\n' && count < MAXBUF){
			inpbuf[count] = '\0';
			return count;
		}
		if(c == '\n' || count >= MAXBUF){
			printf("smallsh: input line is too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}


int gettok(char** outptr){
	int type;
	*outptr = tok;
	while(*ptr == ' ' || *ptr == '\t')
		ptr++;

	*tok++ = *ptr;
	switch(*ptr++){
		case '\n':
			type = EOL;
			break;
		case '&':
			type = AMPERSAND;
			break;
		case ';':
			type = SEMICOLON;
			break;
		default:
			type = ARG;
			while(inarg(*ptr))
				*tok++ = *ptr++;
	}
	*tok++ = '\0';
	return type;
}

// 그냥 문자면 1, special ' ' '\t' '\n' '&' ';' '\0' 이면 0
int inarg(char c){
	char *wrk;
	for(wrk = special; *wrk; wrk++){
		if(c == *wrk)
			return 0;
	}
	return 1;
}

void procline(){
	char *arg[MAXARG + 1];
	int toktype, type;
	int narg = 0;
	for(;;){
		switch(toktype = gettok(&arg[narg])){
			case ARG:
				if (narg < MAXARG)
					narg++;
				break;
			case EOL:
			case SEMICOLON:
			case AMPERSAND:
				if (toktype == AMPERSAND) type = BACKGROUND;
				else type = FOREGROUND;

				if (narg != 0){
						arg[narg] = NULL;
						/* modified */
						if(runcommand(arg, type) == -1)
							exit(0);
				}
				if (toktype == EOL) return;
				narg = 0;
				break;
		}
	}
}

int runcommand(char** cline, int where){
	pid_t pid;
	int status;
	int redirect = 0;
	int fd_new;

	/**/
	// exit
	if(strcmp(*cline, "exit") == 0){
		printf("logout\n");
		exit(0);
	}

	// cd
	if(strcmp(*cline, "cd") == 0){
		if(cline[2] != NULL){
			printf("Usage: cd <dir>\n");
			return 0;
		}
		if(chdir(cline[1]) == -1)
			perror(cline[1]);
		return 0;
	}
	
	// >
	for(int i = 0; cline[i]; i++){
		// find >
		if(strcmp(cline[i], ">") == 0){
			if((fd_new = open(cline[i + 1], O_WRONLY | O_CREAT, 0755)) == -1){
				perror(cline[i + 1]);
				return -1;
			}
			
			// overwrite > to NULL
			redirect = 1;
			cline[i] = NULL;
			break;
		}
	}
	
	// fork
	switch(pid = fork()){
		case -1:
			perror("smallsh");
			return -1;
		case 0:
			if(redirect == 1){
				if(dup2(fd_new, 1) == -1)
					perror("output redirection failed");
			}
			execvp(*cline, cline);
			perror(*cline);
			exit(1);
	}
	
	/**/

	if (where == BACKGROUND){
		printf("[Process id] %d\n", pid);
		return 0;
	}
	if (waitpid(pid, &status, 0) == -1)
		return -1;
	else
		return status;
}

