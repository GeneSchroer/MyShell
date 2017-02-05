#include "sfish.h"

void sigchld_handler(int sig);
extern char** environ;
struct prompt_check{
	int user;	
	char user_color;
	int user_bold;
	int machine;
	char machine_color;
	int machine_bold;
	char* relpath;
};
struct job_struct{
	unsigned int jid;
	pid_t grpid;
        time_t timestamp;
	int exit_status; 
	struct job_struct* next;
	struct process_struct* process;
};
struct process_struct{
  struct process_struct* next;
  char** args;
  pid_t pid;
  int status;

};
void signals(int i);
void printerrormsg(int* retval, char* name, char* msg);
int run_command(char** args, size_t n/*, int* fread, int* fd_write*/);
int run_exec2(char** args, size_t n);
void run_prog(char* file, char** args, char* env[], int* fread);
void read_commandline(char** args);
void redirection(char** rhs, char* symbol, int* fd_read, int* fd_write);
char** getsubarray(char** ar, int* n);
char* parse2(char* cmd);


void Prt(int n);
void prt();
void Help(int n);
void help();
char* pwd();
void Pwd(int n);

void Jobs(int n);
void jobs();
struct job_struct* createjob();
void addjob(struct job_struct* job);
void printjob(struct job_struct* job);
int deletejobfromlist(pid_t pgid, int jid);
void deletejob(struct job_struct* job);
void deleteprocess(struct process_struct* process);

int add_process_to_job(struct job_struct* job, pid_t pid, char** args, int status);
int update_job(pid_t pid, int status);

/* Part 4 Functions */
void Fg(char** args, int n);
void fg(int pid,int jid);
void Bg(char** args ,int n);
void bg(int pid, int jid);
void Kill(char** args, int n);
void kill2(int signal, int grpid, int jid);
void Disown(char** args, int n);
void disown(int pid, int jid);
void delete_all_jobs(struct job_struct* job);


void place_in_background(struct job_struct* job);
void place_in_foreground(struct job_struct* job);
int foreground_wait(struct job_struct* job);


char** parse(char* input, size_t n, char* delim);
int gettokencount(char* input, char* delim);
void Chclr(char** args, int n);
void chclr(struct prompt_check* pl, char* setting, char* color, int bold);
void Cd(char** args, size_t n);
void Chpmt(char** args, size_t n);
void chpmt(struct prompt_check* pc, char* option, int number);
char* getrelativepath();
char* getcommandline(struct prompt_check pc, const char* c);

/* Part 5 Functions */

int ctrl_help(int count, int key);
int storespid(int count, int key);
int getspid(int count, int key);
int printinfo(int count, int key);

char* cmd_prompt;
struct prompt_check prompt_c;
int my_return = 0;

int wait_option;
int fd_read, fd_write, fd_error;


int jobid;	/* used to set the lowest unused job id*/
pid_t spid;
pid_t shell_pgid;
pid_t fore_pgid;



struct job_struct* job_list;
int pipe_flag, bg_flag;
int commands;


int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.
rl_command_func_t ctrl_help;
rl_command_func_t storespid;
rl_command_func_t getspid;
rl_command_func_t printinfo;

rl_bind_keyseq("\\C-h", ctrl_help);   
rl_bind_keyseq("\\C-b", storespid);
rl_bind_keyseq("\\C-g", getspid);
rl_bind_keyseq("\\C-p", printinfo);
    /* Initialize the prompt on the command line */
    prompt_c.user = 1;
    prompt_c.user_color = 7;
    prompt_c.machine = 1;
    prompt_c.machine_color = 7;
    prompt_c.relpath = getrelativepath();
    cmd_prompt = getcommandline(prompt_c, "sfish");
    
    //int b;
    jobid = 0;
    spid = -1;
    commands = 0;
    char *cmd=NULL;
    char *parsed_cmd = NULL;
    char **av = NULL;
    int tokens /*, i*/;
    job_list = malloc(sizeof(struct job_struct));
    job_list->next = NULL;  
    job_list->process = NULL;
    job_list->jid = jobid++;
    shell_pgid = getpid(); 
    if(setpgid(0, 0)<0)
	fprintf(stderr,"Could not change group\n");                    
    signals(0);
    fore_pgid = shell_pgid;
    while((cmd = readline(cmd_prompt)) != NULL) {
        
	
	parsed_cmd = parse2(cmd);
	tokens = gettokencount(parsed_cmd, " ");
	av = parse(parsed_cmd, (size_t)tokens, " ");
	/* To Do: */	

	/* 1) Parse cmd line into string tokens */
	/* 2) Call get_builtin, which returns a number */
	/* 3) Use that number, along with some tokens, */
	/*      to execute a builtin function */
	/* 4) */


        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        //You WILL lose points if your shell prints out garbage values.
	/* Part 1: */	
	/* This will include a list of...stuff */
	
	if (strcmp(cmd,"quit")==0)
            break;	
	read_commandline(av);
	
	free(*av); 	
	free(av);
	free(cmd);
        free(parsed_cmd);
    }
    free(*av);
    free(av);
    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    free(parsed_cmd);
    free(prompt_c.relpath); 
    close(0);
    close(1);
    close(2);
    free(cmd_prompt);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}

/* 0: program has no input 1: program has input */
int run_builtin(char** args, size_t n){
	if(n == 0)
		return -1;
	else if(strcmp(args[0], "exit")==0){
		exit(0);
	}		
	else if(strcmp(args[0], "cd")==0){
		Cd(args, n);
		return 0;
	}
	else if(strcmp(args[0], "chpmt")==0){
		Chpmt(args, n);	
		return 0;
	}
	else if(strcmp(args[0], "chclr")==0){
		Chclr(args, n);
		return 0;
	}
	else if(strcmp(args[0], "fg")==0){
		Fg(args, n);
		return 0;
	}
	else if(strcmp(args[0], "bg") ==0){
		Bg(args, n);
		return 0;
	}
	else if(strcmp(args[0], "kill")==0){
		Kill(args, n);
		return 0;
	}
	else if(strcmp(args[0], "disown")==0){
		Disown(args, n);
		return 0;
	}
        else if(strcmp(args[0], "")==0){
		return 0;
	}
	else
 	  return -1;

}

/* Cases: */
/* builtin function is exit: return 2 */
/* builtin function is anything else: return 0 */
/* no builtin function: return 1 */
/* no tokens */
int run_command(char** args, size_t n){
	if(n == 0)
		return -1;
	if(strcmp(args[0], "help") == 0){
	  Help(n);
	  return 0;
	}
	else if(strcmp(args[0], "exit")==0){
		exit(0);
	}		
	else if(strcmp(args[0], "prt") == 0){	
		Prt(n);
		return 0;
	}
	else if(strcmp(args[0], "cd")==0){
		Cd(args, n);
		return 0;
	}
	else if(strcmp(args[0], "pwd") == 0){
		Pwd(n);
		return 0;
	}
	else if(strcmp(args[0], "chpmt")==0){
		Chpmt(args, n);	
		return 0;
	}
	else if(strcmp(args[0], "chclr")==0){
		Chclr(args, n);
		return 0;
	}
        else if(strcmp(args[0], "jobs")==0){
		Jobs(n);
		return 0;
	}

        else if(strcmp(args[0], "")==0){
		return 0;
	}
	else{
	  run_exec2(args, n);
        }
 return -1;
}
int run_exec2(char** args, size_t k){
  struct stat buf; 
  char* path;
  char** pathtoks = NULL;
  char* temp;
  int n, i, size;
 
   //int retval;
  /* case 1, first argument has a '/' */
  if(strchr(args[0], '/') != NULL){
   /* check if file exists */
    if (stat(args[0], &buf) < 0){
      printerrormsg(&my_return, args[0], "Program not found"); 
      return -1;
    } 
    else if( (buf.st_mode & S_IXUSR) != S_IXUSR){
      printerrormsg(&my_return, args[0], "Command Not Found");
    }
    else{
     //run_prog(args[0], args, environ, &fd_read);  
      if( execve(args[0], args,environ)<0){
        printerrormsg(&my_return, args[0],"Child: Command Not Found.");
        exit(0);
      }
    }
  }
  /* case 2, first argument does not have a '/' */
  else{
    /* get PATH string */
    path = getenv("PATH");
    /* break string into tokens */
    n = gettokencount(path, ":");
    pathtoks = parse(path, n, ":");
    
    /* for each token, concatenate pathname to token and check if it exists*/
    for(i = 0; i < n ; ++i){
      size = (strlen( *(pathtoks+i) ) + strlen(args[0]) +1 + 1 ) ;
      temp = malloc(sizeof(char) * size);
      memset(temp, 0, sizeof(char) * size);
      strcat(temp, *(pathtoks + i) );
      strcat(temp, "/");
      strcat(temp, args[0]);
      
      if(stat(temp, &buf)== 0){
        if( (buf.st_mode & S_IXUSR) != S_IXUSR){
           printerrormsg(&my_return, args[0], "Command Not Found");
        }
        else{
	if( execve(temp, args,environ)<0){
          printerrormsg(&my_return, args[0],"Child: Command Not Found.");
          exit(0);
        } 
	// run_prog(temp, args, environ, &fd_read);
        }
        free(temp);
	break;
      }
      free(temp);
     
    }
    if(i == n)
      printerrormsg(&my_return, args[0],"Command Not Found.");
    
  }
 free(pathtoks);
  return 0;
}
int add_process_to_job(struct job_struct* job, pid_t pid, char** args, int status){
int i;
if( (job->grpid) == 0)
job->grpid = pid;

setpgid(pid, job->grpid);
struct process_struct *trav = job->process; 


  while(trav->next!=NULL){
    trav = trav->next;  
  }

  trav->next = malloc(sizeof(struct process_struct));
  trav = trav->next;

trav->pid = pid;

i = 0;
while( args[i++] != NULL){
  
}
++i;

trav->args = malloc(sizeof(char*) * (i));
memset(trav->args, 0, sizeof(char*) * (i));
i = 0;
while( args[i] != NULL){
  *( (trav->args) + (2*i)) = strdup( *(args+i));
  ++i;
}


trav->status = status;
trav->next = NULL;
return 0;
}

/* attempt to create foreground process*/
 void pipeline4(char*** list, int n, int* tokens){  
/* if '&', set jobid*/
int status = 0;
struct job_struct* current_job = NULL;


int first_pid=0;
char ***trav = list;
int *pipefd;
pid_t pid;
pipefd = malloc(sizeof(int) * 2 * (n-1));
int i, j, pipe_r, pipe_w;

if(run_builtin(trav[0], tokens[0])<0){
    i = 0;
    j = 0;
    pipe_r = fd_read;
    while(trav[i] != NULL ){
      pipe((pipefd+j)); /* create pipe for this fork */
      if(trav[i+1] !=NULL )
        pipe_w = *(pipefd+j+1);
      else{
	pipe_w = fd_write;
      }
      switch( pid = fork() ){
        case 0:
     		signals(1);	
	        
	    
	          if(!first_pid){
	            setpgid(0, 0);
                  }
	        else
	          setpgid(0, current_job->grpid);
	        

	        if (pipe_r != 0){
	          dup2(pipe_r, 0);	/* dup over read and write ends*/	
	          close(pipe_r);/* child does not need reading end */
	        }
	        if( pipe_w != 1){
                  dup2(pipe_w, 1);
	          close(pipe_w);   
	        } 
	
	        if(fd_error != 2){
	          dup2(fd_error, 2);
                }	

	        run_command(trav[i], (size_t)tokens[i]);
      	        exit(0);
	  break;
  	  case -1:
	    break;
  	  default:
            /* close pipes */
	    	 	
            		
	          if(first_pid){
	         
	          } 
	          else{
	            first_pid = 1;
			current_job = createjob();
	          }

	          add_process_to_job(current_job, pid, trav[i], status);
		#ifdef DEBUG	
                  printf("Pid %ld has signals %d\n", (long int)pid, status);
	        #endif
            if(pipe_w != fd_write || pipe_w != 1)
              close(pipe_w); /* parent does not need writing end */
	    if(pipe_r != fd_read)
	      close(pipe_r);
	    pipe_r = *(pipefd+j);
		++i;
		j += 2;
	    	break;
	}
	

    }
      //printjob(current_job);
 	place_in_foreground(current_job);
  }


bg_flag = 0;

} 


 
void pipeline3(char*** list, int n, int* tokens){  
/* if '&', set jobid*/
int status = 0;
struct job_struct* current_job;

current_job = NULL;

int first_pid=0;
char ***trav = list;
int *pipefd;
pid_t pid;
pipefd = malloc(sizeof(int) * 2 * (n-1));
int i, j, pipe_r, pipe_w;
//sigset_t mask, prev_mask;
//sigemptyset(&mask);
//sigaddset(&mask, SIGCHLD);
//sigprocmask(SIG_BLOCK, &mask, &prev_mask);



  if(run_builtin(trav[0], tokens[0])<0){
    i = 0;
    j = 0;
    pipe_r = fd_read;
    while(trav[i] != NULL ){
      pipe((pipefd+j)); /* create pipe for this fork */
      if(trav[i+1] !=NULL )
        pipe_w = *(pipefd+j+1);
      else{
	pipe_w = fd_write;
      }
      switch( (pid = fork()) ){
        case 0:
     		signals(1);	
//	        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
	          if(!first_pid){
	            setpgid(0, 0);
                  }
	        else{
	          setpgid(0, current_job->grpid);
	        }

	        if (pipe_r != 0){
	          dup2(pipe_r, 0);	/* dup over read and write ends*/	
	          close(pipe_r);/* child does not need reading end */
	        }
	        if( pipe_w != 1){
                  dup2(pipe_w, 1);
	          close(pipe_w);   
	        } 
	
	        if(fd_error != 2){
	          dup2(fd_error, 2);
                }	

	        #ifdef DEBUG
	           fprintf(stderr, "Running child process %s %d %d\n", 
                       trav[i][0], (int)getpid(), (int)getpgrp());
      	        #endif

	        run_command(trav[i], (size_t)tokens[i]);
      	        exit(0);
	  break;
  	  case -1:
	    break;
  	  default:
	          
	          if(first_pid){
	         
	          } 
	          else{
	            first_pid = 1;
			current_job = createjob();
	          }

	          add_process_to_job(current_job, pid, trav[i], status);
		#ifdef DEBUG
		  printf("Pid %ld has signals %d\n", (long int)pid, status);
       		#endif
            if(pipe_w != fd_write)
              close(pipe_w); /* parent does not need writing end */
	    if(pipe_r != fd_read)
	      close(pipe_r);
	    pipe_r = *(pipefd+j);
		++i;
		j += 2;
	    	break;
	}
     }
	#ifdef DEBUG
	printf("Placing job into background\n");
        #endif
	place_in_background(current_job);

      printjob(current_job);
		 
  

}
 bg_flag = 0;

//sigprocmask(SIG_SETMASK, &prev_mask, NULL);


}

void read_commandline(char** args){
  char  **lhs, **rhs, **trav;
  
  int s = 0;
  while(*(args + s) != NULL){
    ++s;
  }
  char*** list = malloc(sizeof(char**) * (s+1));
  int token_count[s+1];
  
  int list_size = 0;
  lhs = NULL;
  rhs = NULL;
  int l, r, end_reached, error_occured;
  char *lhs_symbol, *rhs_symbol;
  trav = args; /* trav starts at beginning of array and traverses through it*/
  error_occured = 0;
  end_reached = 0;
  fd_read = 0;
  fd_write = 1;
  fd_error = 2;
  wait_option = 0;
  pipe_flag = 0;
  bg_flag = 0;
  while(1){ 

    /* get lhs subarray and its length */
    lhs = getsubarray(trav, &l); 
    if(lhs == NULL)
      break; 
    /* look at the symbol directly after lhs */
    lhs_symbol = *(trav + l);
  
  
    /* if symbol after lhs is NULL, break from loop */
    if( lhs_symbol == NULL )
      break;
  
    /* if symbol is '&', set bg flag */
    else if(!strcmp(lhs_symbol, "&")){
          /* set global variable wait_option to WNOHANG */
	  #ifdef DEBUG
		fprintf(stderr, "Do Background\n");
	  #endif
	  bg_flag = 1;
        break;  
	trav += l + 1;
          /* reset list */
          bg_flag = 0;
          list_size = 0;
	  
    }
    else{
      r = l;
      /* we now start a sub loop that breaks when we pipe */
      while(1){
        trav += r  + 1; 
        rhs = getsubarray(trav, &r);
        /* error occured, can't complete command */
        rhs_symbol = *(trav + r);
        if (rhs == NULL){
          fprintf(stderr, "Error occured\n");
          error_occured = 1;
        }
	
        else if (!strcmp(lhs_symbol, "|")){
            pipe_flag = 1;
            #ifdef DEBUG
            printf("Do pipeline\n");
            #endif
            list[list_size] = lhs;
	    token_count[list_size] = l;
	    #ifdef DEBUG
		fprintf(stderr, "Command number %d: %s\n",list_size, list[list_size][0]);
	    #endif
	    ++list_size;
            //pipeline(lhs, &fd_read)
            break;

        }
	else if(rhs_symbol != NULL && !strcmp(rhs_symbol, "&")){
	/* you should run commandline if you encounter ampersand */	
        }        
        else{
	  #ifdef DEBUG
	  printf("Do redirection\n");
          #endif
	  redirection(rhs, lhs_symbol, &fd_read, &fd_write);
	  rhs_symbol = *(trav + r);
          if(rhs_symbol == NULL){
            end_reached = 1; 
	    break;
          }
          else
            lhs_symbol = rhs_symbol;
        }
    //    free(rhs);
      }
    }
    if(end_reached || error_occured)
      break;
    //free(lhs);
  }
  list[list_size] = lhs;
  token_count[list_size] = l;
    #ifdef DEBUG
  printf("fd_read: %d\n", fd_read);
  printf("fd_write: %d\n", fd_write);
 #endif
  
  if(list[list_size][0] != '\0'){
   ++list_size;
   list[list_size] = NULL;
   token_count[list_size] = 0;
   if(bg_flag)
    pipeline3(list, list_size, token_count/*, &fd_read, &fd_write, &fd_error*/);
   else{
	 pipeline4(list, list_size, token_count);
	}
   ++commands;  
}
bg_flag = 0;
free(lhs);  
  free(rhs);

  //if(fd_read != 0)
  //  close(fd_read);
  //if(fd_write != 1)
   // close(fd_write);
  //if(fd_error != 2)
    //close(fd_error);
}
void redirection(char** rhs, char* symbol, int* fd_read, int* fd_write){
  int fd;
  #ifdef DEBUG
  printf("Attempt to open fd\n");
  #endif
  if(!strcmp(symbol, "<")){
    
   
    if( (fd = open(rhs[0], O_RDONLY))< 0){
       fprintf(stderr, "Error: Cannot open file\n");
    }
    #ifdef DEBUG
    else
      printf("fd opened\n");
    #endif
    if(*fd_read != 0)
      close(*fd_read);
    
    *fd_read = fd;
  }
  else if(!strcmp(symbol, ">")){
    
  
    if( (fd = open(rhs[0], O_TRUNC | O_WRONLY | O_CREAT, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH))< 0){
      fprintf(stderr,"Error: cannot open file\n");
    }
    #ifdef DEBUG
    printf("fd opened\n");
    #endif
    if(*fd_write != 1 && *fd_write != 2)
      close(*fd_write);
    *fd_write = fd;
  }
}

char** getsubarray(char** ar, int* n){
  int i;
  *n = 0;
  char** ret;
  char* temp;
  temp = *ar;
  while(temp != NULL && strcmp(temp, ">") && strcmp(temp, "<") && strcmp(temp,"2>") && strcmp(temp, "|") && strcmp(temp, "&")){
    ++(*n);
    temp = *(ar + *n);

  }

  ret = malloc(sizeof(char*) * ( (*n) + 1));
  for(i = 0; i < *n ; i++){
    *(ret + i) = *(ar + i);
  }
  *(ret + i) = NULL;
  return ret;

}
void Help(int n){
  if(n == 1){
    help();
  }

  else
    printerrormsg(&my_return, "help", "Too many arguments"); 
 
}

/* error messages covered: N/A */
void help(){
 
    printf("SBU sfish, version ???\n");
    printf("These shell commands are defined arbitrarilly. Type 'help' to see this list.\n");
    printf("exit\n");
    printf("cd [DIR]\n");
    printf("pwd\n");
    printf("prt\n");
    printf("chpmt chpmt SETTING TOGGLE\n");
    printf("chclr SETTING COLOR BOLD\n");
    printf("jobs\n");
    printf("fd PID|JID\n");
    printf("bg PID|JID\n");
    printf("kill [signal] PID|JID\n");
    printf("disown [PID|JID]\n");
    my_return = EXIT_SUCCESS;
    exit(0);
}

int gettokencount(char* input, char* delim){
	int token_count = 0;
	char* temp;
	char* copy = strdup(input);
	temp = strtok(copy, delim);
	if(temp == NULL)
		return 0;
	++token_count;
	while( (temp = strtok(NULL, delim)) != NULL)
		++token_count;

	free(copy);
	return token_count;
}

char** parse(char* input, size_t n, char* delim){
	char **output/*=NULL*/;
	output = malloc(sizeof(char*) * (n + 1));
	char* temp = NULL;
	temp = strdup(input);
	int i = 1;
	temp = strtok(temp, delim);
	*output = temp;
	//free(temp);
	while( (temp = strtok(NULL, delim)) != NULL){
		*(output + i) = temp;
		++i;
	//	free(temp);
	}
	*(output + n) = 0x00;
	return output;

}




/* All error messages: Yes */
/* Message for single argument: Not completed */
void Chclr(char** args, int n){

int valid_setting;
int valid_color;
int valid_bold;

  switch(n){
    default:
	  if(n<4)
            printerrormsg(&my_return, "chclr", "Too few arguments");
	  else
	    printerrormsg(&my_return, "chclr,","Too many arguments");
	  break;
    case 4:
 
	valid_setting = ( strcmp(args[1], "user") == 0) 
				|| ( strcmp(args[1], "machine") == 0);
	valid_color = (strcmp(args[2], "red") == 0)
			|| (strcmp(args[2], "blue") == 0)
			|| (strcmp(args[2], "green") == 0)
			|| (strcmp(args[2], "yellow") == 0)
			|| (strcmp(args[2], "cyan") == 0)
			|| (strcmp(args[2], "magenta") == 0)
			|| (strcmp(args[2], "black") == 0)
			|| (strcmp(args[2], "white") == 0);
	valid_bold = (strcmp(args[3], "0") == 0) 
			|| (strcmp(args[3], "1") == 0);

	if(valid_setting && valid_color && valid_bold){
	  #ifdef DEBUG
          printf("Change successful\n");
	  #endif
          chclr(&prompt_c, args[1], args[2], (int)(args[3][0]-'0'));
	  free(cmd_prompt);
	  cmd_prompt = getcommandline(prompt_c, "sfish");

	  my_return = EXIT_SUCCESS;
	} else
	  printerrormsg(&my_return, "chclr", "One or more arguments are invalid");
	break;
  }
}
void chclr(struct prompt_check* pl, char* setting, char* color, int bold){
  char c;
  if(!strcmp(color, "red"))
    c = 1;
  else if(!strcmp(color, "blue"))
    c = 4;
  else if(!strcmp(color, "green"))
    c = 2;
  else if(!strcmp(color, "yellow"))
    c = 3;
  else if(!strcmp(color, "cyan"))
    c = 6;
  else if(!strcmp(color, "magenta"))
    c = 5;
  else if(!strcmp(color, "black"))
    c = 0;
  else if(!strcmp(color, "white"))
    c = 7;
  if(strcmp(setting, "user") == 0){
    pl->user_color = c;
    pl->user_bold = bold;
  }
  else if(strcmp(setting, "machine") == 0){
    pl->machine_color = c;
    pl->machine_bold = bold;
  }

}

/* All error message: Not complete */
/* Message for single argument: Not complete */
void Prt(int n){
	switch(n){
		default:
		 	printerrormsg(&my_return,"prt", "Too many arguments");	
                     
			break;
		case 1: 
			prt();
			break;				
	}
}
void printerrormsg(int *retcode, char* name, char* msg){
    
    fprintf(stderr, "sfish: %s: %s\n", name, msg);
    *retcode = EXIT_FAILURE;
}
void prt(){
    printf("%d\n", my_return);
    			
 
}
/* all error msgs: complete */
/* msg for single argument: N/A */
/* return code: complete*/
void Pwd(int n){
 
  char too_many[] ="Too many arguments"; 

  if(n != 1){
      printerrormsg(&my_return,"pwd", too_many);
 
  }
  else{
    char* buf;
     
      buf = pwd();
      dup2(fd_write, 1);
      printf("%s\n", buf);
      free(buf);
      my_return = EXIT_SUCCESS;
  }
}
char* pwd(){
	char* buf = malloc(sizeof(char) * 256);
	size_t n = 256;
	while( ( buf = getcwd(buf, sizeof(char) * n)) == NULL){
		buf = realloc(buf, n * 2);
		n = n * 2;
		if(errno == ENOMEM){
			free(buf);
			return NULL;
		}
	}
	return buf;
}

/* Error messages: complete*/
/* 1 arg message: N/A */
/* return code: complete */
void Cd(char** args, size_t n){
pid_t pid;
char* lastpwd;
char* currpwd;
currpwd = pwd();
switch(n){
  case 1:
	setenv("LASTPWD", currpwd, 1);
	chdir(getenv("HOME"));	
	my_return = EXIT_SUCCESS;
	break;
  case 2:
	if(!strcmp(args[1],"-")){
	  if( (lastpwd = getenv("LASTPWD"))==NULL)
	      printerrormsg(&my_return, "cd","LASTPWD is not set");	
	  else{
	    setenv("LASTPWD", currpwd, 1);
	    chdir(lastpwd);
	    if( (pid = fork()) == 0){
	      printf("%s\n", lastpwd);
	      my_return = EXIT_SUCCESS;
	      exit(0);
	    }
	    else{
		int status;
	      waitpid(-1, &status, 0);
		}
	    }
	}
	else if(chdir(args[1]) == -1){
	    printerrormsg(&my_return, args[1], "No such file or directory");	
	} 
	else{
	  setenv("LASTPWD", currpwd, 1);
	  my_return = EXIT_SUCCESS;
	}
	break;
	default:
	  
	    printerrormsg(&my_return, "cd", "Too many arguments");
	break;
		
		
  }
	free(prompt_c.relpath);
	prompt_c.relpath = getrelativepath();
	free (cmd_prompt);
	cmd_prompt = getcommandline(prompt_c, "sfish");
	free(currpwd);
}
/* error msgs: complete */
/* 1 arg msg: incompete */
/* return code: complete*/
void Chpmt(char** args, size_t n){
  int valid_option;
  int valid_number;
  char name[] = "chpmt";
  char too_many[] = "Too many arguments";
  char too_few[] = "Too few arguments";
 // char invalid_args[] = "Invalid arguments";
  switch(n){
    default: 
	if(n>3)
	    printerrormsg(&my_return, name, too_many);
	if(n<3)
            printerrormsg(&my_return, name, too_few);
	break;
    case 1:
	break;
    case 3:
	valid_option = ( (strcmp(args[1], "user") == 0) || (strcmp(args[1], "machine") == 0) );

	valid_number = ( (strcmp(args[2], "0") == 0) || (strcmp(args[2], "1")== 0));	
	if(valid_option && valid_number){
	  chpmt(&prompt_c, args[1], (int)(args[2][0]-'0'));
	  free(cmd_prompt);
	  cmd_prompt = getcommandline(prompt_c, "sfish");
	  my_return = EXIT_SUCCESS;
	}else{
	  printerrormsg(&my_return, "chpmt", "Invalid arguments");
	}
	break;
	
	} 
}

void chpmt(struct prompt_check* p, char* option, int i){
	if(strcmp(option, "user")==0)
	  p->user = i;
        if(strcmp(option, "machine") == 0)
	  p->machine = i;
	
}

char* getrelativepath(){
  char* dir;
  char* temp;
  if ( (dir = pwd()) == NULL)
    return NULL;
  if(strcmp(dir, getenv("HOME"))==0){
    free(dir);
    dir = malloc(sizeof(char) * 2);
    strcpy(dir, "~");
    return dir;

  }else if(strstr(dir, getenv("HOME")) != NULL){
    temp = dir + strlen(getenv("HOME"));
    temp = strdup(temp);
    free(dir);
    //printf("%s\n", temp);    
    return temp;
  } else{
    return dir;  
  }
}

void Fg(char** args, int n){
  int i;
  int sum;
  int jid_flag;
  switch(n){
  case 2:
          jid_flag = 0;
	  i =0;
          if(args[1][0]=='%'){
            i++;
	    jid_flag = 1;
	  }
	  while( args[1][i] != '\0'){
	
	    if( (isdigit((unsigned char)args[1][i])) == 0 ){
		printerrormsg(&my_return, "fg", "Invalid arguments");
	    	return;
		}
	    ++i;
	  }
          if(jid_flag)
	    i = 1;
	  else
            i = 0;
	  sum = 0;
	  while(args[1][i] != '\0'){
	    sum = (sum * 10) + (args[1][i] - '0');
	    ++i;
	  }
          if(jid_flag)
            fg(0, sum); 	
	  else
	    fg(sum, 0);
	break;

  default: printerrormsg(&my_return, "fg", "Too Many Arguments");
	break;
  

  
  }
}

void fg(int pid, int jid){
  struct job_struct* trav = job_list->next;
  while(trav != NULL){
    if(trav->jid == jid || trav->grpid == pid){
      #ifdef DEBUG
      printf("Putting %d into foreground\n", (int)trav->grpid);
      #endif
      place_in_foreground(trav);
      break;
    }
    trav = trav->next;
  } 


}
void Bg(char** args, int n){
  int i;
  int sum;
  int jid_flag;
  switch(n){
  case 2:
	  jid_flag = 0;
	  i =0;
	  if(args[1][0] == '%'){
		jid_flag = 1;
		++i;
	  }
	  while( args[1][i] != '\0'){
	
	    if( isdigit((unsigned char)args[1][i]) == 0 ){
		printerrormsg(&my_return, "fg", "Invalid arguments");
		return;
	    }
	    ++i;
	  }
	  if(jid_flag)
	    i = 1;
	  else
            i = 0;
	  sum = 0;
	  while(args[1][i] != '\0'){
	    sum = (sum * 10) + (args[1][i] - '0');
	    ++i;
	  }
          if(jid_flag)
          bg(0,sum); 	
          else
	  bg(sum, 0);
	break;

  default: printerrormsg(&my_return, "fg", "Too Many Arguments");
	break;
  

  
  }
}
void bg(int pid, int jid){
 struct job_struct* trav = job_list->next;
  while(trav != NULL){
    if(trav->jid == jid || trav->grpid == pid){
      #ifdef DEBUG
      printf("Continuing process %d\n", (int) trav->grpid);
      #endif
      place_in_background(trav);
      break;
    }
    trav = trav->next;
  }
}
void place_in_background(struct job_struct* job){
  //int status;
  //struct process_struct* process = job->process->next;
  addjob(job);
  #ifdef DEBUG
  printf("Continuing pgid in background: %d\n", (int)job->grpid);
  #endif
  if(kill(-job->grpid, SIGCONT)<0)
    fprintf(stderr,"%s\n", strerror(errno));
  //while(process != NULL){
    //waitpid(process->pid, &status , WNOHANG|WCONTINUED);
    //process->status = status;
   
 //   process = process->next;
 // }
}
void place_in_foreground(struct job_struct* job){
  #ifdef DEBUG
  printf("PGID in job: %d\n", (int)job->grpid);
  #endif
  int status;
 // pid_t pid;
  #ifdef DEBUG
 printf("Assume control\n");
  printf("Group id of job: %d\n", (int)job->grpid); 
  #endif 
 if(tcsetpgrp(0, job->grpid) < 0)
	printf("%s\n",strerror(errno));
  /* continue program if stopped */
  kill(-job->grpid, SIGCONT);
  fore_pgid = job->grpid;
  
   #ifdef DEBUG 
   printf("Foreground pgid: %d\n", (int)getpgrp());
   #endif
   status = foreground_wait(job);
  tcsetpgrp(0, shell_pgid);
  fore_pgid = shell_pgid;
  #ifdef DEBUG
  printf("Shell pgid: %d\n", (int)getpgrp()); 
  #endif
  if(WIFSTOPPED(status)){
     addjob(job);
     printjob(job);
     #ifdef DEBUG
     printf("Add job to list\n"); 
     #endif 
  }
   else
     if(deletejobfromlist(job->grpid, 0)<0)
       deletejob(job);     
 
   
 }


void Jobs(int n){
  if(n>1){
	printerrormsg(&my_return, "Jobs", "Too Many Arguments");
  }
  else
    jobs();

}
int sendthesignal(int signal){

  return 0;
}
void jobs(){
  struct job_struct* trav = job_list->next;
  //printf("Jobs called\n");
  while(trav!=NULL){
    if(trav->grpid != getpgrp())
      printjob(trav);
    trav = trav->next;
  } 

}
struct job_struct* createjob(){
  struct job_struct* trav;

  trav = malloc(sizeof(struct job_struct));
  trav->next = NULL;
  trav->jid = 0;
  trav->grpid = 0;	/* initially unset */
  trav->process = malloc(sizeof(struct process_struct));;
  trav->process->next = NULL;
  trav->process->args=NULL;
  trav->timestamp = time(NULL);
 // trav->status = 0;
  return trav;
  /* do we do status? */
}

void addjob(struct job_struct* job){
struct job_struct *trav, *temp;
  trav = job_list->next;
  temp = job_list;
  while( trav != NULL){
    if(trav->grpid == job->grpid){
	#ifdef DEBUG
	printf("Job is already in list\n");
	#endif
       return;
    }
    #ifdef DEBUG
    printf("Traversing\n");
    #endif
    trav = trav->next;
    temp = temp->next;
  }
    #ifdef DEBUG
  printf("Added job\n");
  #endif
  temp->next = job;
  job->jid = jobid++;
}

void printjob(struct job_struct* job){
  char* state;
  int status;
  struct process_struct* trav = job->process->next;
  
  while(trav!=NULL){
   
   if( waitpid(trav->pid, &status, WNOHANG|WUNTRACED|WCONTINUED) > 0){
     //printf("Status of %d updated\n", (int)trav->pid);
     trav->status = status;
   }
   else
   status = trav->status; 
   
   if(WIFEXITED(status)){
      state = "Done";
   }
   if(WIFSIGNALED(status)){
      state = "Done";
   }
   else if(WIFSTOPPED(status)){
      state = "Stopped";
   }
   else{
      state = "Running";
      break;
   }
   trav = trav->next;
  }
  printf("%u\t%s\t%ld\t", (job->jid), state, (long int)(job->grpid));

   trav = job->process->next;
  char** args;
  int i;
  while(trav!=NULL){
    i = 0;
    args=trav->args;
    while(*(args+i)!=NULL){
	printf("%s ", *(args+i));
                ++i;
    }
    if(trav->next!=NULL)
	  printf("| ");
    trav = trav->next;
  }
  printf("\n");
}

int deletejobfromlist(pid_t grpid, int jid){
  struct job_struct *trav, *temp; 
  trav = job_list;
  pid_t p;
  int j;
 
  while(trav->next != NULL){
	p = trav->next->grpid;
	j = trav->next->jid;
        if(p == grpid || j == jid){
	#ifdef DEBUG
	  printf("Deleting job\n");
	#endif
	  temp = trav->next->next;
	  deletejob(trav->next);
	  trav->next = temp;
	  return 0;	  
        }
    trav= trav->next;
  }  
  return -1;
}

void deletejob(struct job_struct* job){
  deleteprocess(job->process);
  free(job);
}

void deleteprocess(struct process_struct* process){
  if(process == NULL)
     return;
  else{
    deleteprocess(process->next);
    char** trav = process->args;
    int i = 0;
    while( trav !=NULL && trav[i] != NULL){
      free(trav[i++]);
    }
    free(trav);
    free(process);
  }

}
int update_job(pid_t pid, int status){
  pid_t grpid = getpgid(pid);
  struct job_struct* trav = job_list->next;
  struct process_struct* process = NULL;
  while(trav != NULL){
    if(trav->grpid == grpid){
      process = trav->process->next;
      break;
    }
    trav = trav->next;
  }
  while(process != NULL){
   #ifdef DEBUG 
   printf("Process pid: %d\n", (int)process->pid);
   #endif
    if(process->pid == pid){
      process->status = status;
      #ifdef DEBUG
      printf("Job updated\n");
      #endif
      return 0;
    }
    process= process->next;
  }
  return -1;
}
void Kill(char** args, int n ){
int i;
  
  int sum, sum2, jid_flag;
 // pid_t grpid;
  jid_flag = 0;
  switch(n){
  case 2:
	  i = 1;
	  sum = 0;
          if( args[1][0] == '%'){
	    jid_flag = 1;
	    ++i;
	  }
	  while( args[1][i] != '\0' ){
	    if( (isdigit((unsigned char)args[1][i])) == 0 ){
	      printerrormsg(&my_return, "kill", "Invalid arguments");
	      return;
	    }
	    ++i;
	  }
          if(jid_flag)
	    i = 1;
	  else
	    i = 0;
	  while(args[1][i] != '\0'){
	    sum = (sum * 10) + (args[1][i] - '0');
	    ++i;
	  }
	    if(jid_flag)
              kill2(15, 0,sum);
	    else
	      kill2(15, sum, 0);
          
	break;

  case 3:
	  i = 1;
	  sum = 0;
          
	  while( args[1][i] != '\0' ){
	    if( (isdigit((unsigned char)args[1][i])) == 0 ){
	      printerrormsg(&my_return, "kill", "Invalid arguments");
	      return;
	    }
	    ++i;
	  }
          if(jid_flag)
	    i = 1;
	  else
	    i = 0;
	  while(args[1][i] != '\0'){
	    sum = (sum * 10) + (args[1][i] - '0');
	    ++i;
	  }
            if(sum < 1 || sum > 31){
		printerrormsg(&my_return, "kill", "Not a valid signal");
	    } else{

              sum2 = 0;
	      i = 0;
	  if( args[2][0] == '%'){
	    jid_flag = 1;
	    ++i;
	  }
	      while( args[2][i] != '\0' ){
	        if( (isdigit((unsigned char)args[2][i])) == 0 ){
	          printerrormsg(&my_return, "kill", "Invalid arguments");
	          return;
	        }
	        ++i;
	      }
	      if(jid_flag)
	        i = 1;
	      else
                i = 0;
	      while(args[2][i] != '\0'){
	        sum2 = (sum2 * 10) + (args[2][i] - '0');
	        ++i;
	      }
	      if(jid_flag)
   	        kill2(sum, 0, sum2);
              else
		kill2(sum, sum2, 0); 
	    }
	  
	break;
  case 1:printerrormsg(&my_return, "kill", "Too Few Arguments");
	break;
  default: printerrormsg(&my_return, "kill", "Too Many Arguments");
	break;
  

  
  }

}

void kill2(int signal, int pid, int jid){
  pid_t grpid = (pid_t)pid;
  struct job_struct* trav=job_list->next;
  #ifdef DEBUG
	printf("Signal: %d, PGID: %d, JobId: %d\n", signal, pid, jid);
  #endif
  while(trav!=NULL){
    if((trav->jid == jid)|| (trav->grpid == grpid)) {
#ifdef DEBUG   
   printf("[%d] %d stopped by signal %d", trav->jid, trav->grpid, signal);
#endif
      kill(-trav->grpid, signal);
      break;
    }
	trav = trav->next;
  }
   #ifdef DEBUG
   printf("Signal %d sent\n", signal);
   printf("Job ID: %d and Group PID:%d\n ", trav->jid, (int)trav->grpid);
   #endif
}
void Disown(char** args,int n){
int i;
  
  int sum, jid_flag;
  switch(n){
  case 1:
  case 2:
	  i = 1;
	  sum = 0;
	  jid_flag = 0;
	  if(args[1] != NULL){
	    if(args[1][0] == '%'){
	     	jid_flag = 1;
	    	++i;
	    }
	    while( args[1][i] != '\0' ){
	      if( (isdigit((unsigned char)args[1][i])) == 0 ){
		printerrormsg(&my_return, "fg", "Invalid arguments");
	    	return;
	      }
	      ++i;
	    }
            if(jid_flag)
              i = 1;
	    else 
	      i = 0;
            while(args[1][i] != '\0'){
	      sum = (sum * 10) + (args[1][i] - '0');
	      ++i;
	    }
          }
	  if(jid_flag)
            disown(0, sum); 	
          else
            disown(sum, 0);
	break;
  
  default: printerrormsg(&my_return, "disown", "Too Many Arguments");
	break;
  

  
  }
}

void disown(int pid, int jid){
  if(pid == 0 && jid == 0){
    #ifdef DEBUG
    printf("disown all jobs\n");
    #endif
    delete_all_jobs(job_list->next);}
  else 
    deletejobfromlist(pid, jid);
}

void delete_all_jobs(struct job_struct* job){
  if(job==NULL)
	return;
  else{
	delete_all_jobs(job->next);
        deleteprocess(job->process);
	free(job);
  }

}

char* getcommandline(struct prompt_check pc, const char* c){
  char machine[256];
  char* user;
  size_t size = 0;
  char* pr;
  size += strlen(c); /* the size of the initial command line ex. "sfish" */
  if(pc.user == 1){
    user = getenv("USER");
    size += strlen(user) + 1 +/* 10 + 8*/14; /* add a user and either a "-" or "@" */
  }
  if(pc.machine == 1){
    gethostname(machine, 256);
    size += strlen(machine) + 1 + /*10 + 8*/14; /* add a machine and either a "-" or  "@" */
  }
  size += 5;	/* add a ":", a "[]", and a "> " */
 
  size += strlen(pc.relpath);
  
  //size += 5; /* to reposition cursor */

  size += 1;   /* add space for a null byte */

  pr = malloc(sizeof(char) * size);
  
  /* concatentate time */

  memset(pr, 0, size);
  strcat(pr, c);
  if(pc.user || pc.machine)
    strcat(pr, "-");
  if(pc.user){
    switch(pc.user_bold){
	case 0: strcat(pr, "\x1B[0");
		break;
	case 1: strcat(pr, "\x1B[1");
		break;
    }    
  switch(pc.user_color){
 	case 0: strcat(pr, ";30m");
		break;    
   	case 1: strcat(pr, ";31m");
		break;
	case 2: strcat(pr, ";32m");
		break;
	case 3: strcat(pr, ";33m");
		break;
	case 4: strcat(pr, ";34m");
		break;
	case 5: strcat(pr, ";35m");
		break;
	case 6: strcat(pr, ";36m");
		break;
	case 7: strcat(pr, ";37m");
		break;
    }

  
    strcat(pr, user);
    strcat(pr, "\x1B[37;0m");
  //  strcat(pr, "\x1B[7C");
  }
  if(pc.user && pc.machine)
    strcat(pr, "@");
  if(pc.machine){ 
     
    switch(pc.machine_bold){
	case 0: strcat(pr, "\x1B[0");
		break;
	case 1: strcat(pr, "\x1B[1");
		break;
    }switch(pc.machine_color){
 	case 0: strcat(pr, ";30m");
		break;    
   	case 1: strcat(pr, ";31m");
		break;
	case 2: strcat(pr, ";32m");
		break;
	case 3: strcat(pr, ";33m");
		break;
	case 4: strcat(pr, ";34m");
		break;
	case 5: strcat(pr, ";35m");
		break;
	case 6: strcat(pr, ";36m");
		break;
	case 7: strcat(pr, ";37m");
		break;
    }
     
    strcat(pr, machine);
    strcat(pr, "\x1B[37;0m");
  }
  strcat(pr, ":[");
  strcat(pr, pc.relpath);
  strcat(pr, "]> ");
  //printf("%s\n", pr);
  //strcat(pr, "\x1B[17C");
  return pr;
}








char* parse2(char* cmd){
  //char** args; 
  int n = strlen(cmd);
  //int tokens; 

  char* temp = malloc(sizeof(char) * (n * 3 + 1 ) );
  
  int i, k;
  
  for(i = 0, k = 0; i<n ; i++){
    if( *(cmd+i) == '<' || *(cmd+i) == '>' || *(cmd+i) == '|' || *(cmd+i)=='&'){
      *(temp + k) = ' ';
      ++k;
      *(temp + k) = *(cmd + i);
      ++k;
      *(temp + k) = ' ';
      ++k;
    }
    else if( *(cmd + i) == '2')
      if( *(cmd + i + 1) == '>' && *(cmd+i-1)==' '){
        *(temp + k) = ' ';
        ++k;
        *(temp + k) = '2';
        ++k;
        *(temp + k) = '>';
        ++k;
        *(temp + k) = ' ';
        ++k;
      }
      else{
        *(temp + k) = '2';
        ++k;
      }
    else{
      *(temp + k) = *(cmd + i);
      ++k;
    }
}
  *(temp+k) = '\0';  
  #ifdef DEBUG
  printf("Parsed command line%s\n", temp);
  #endif
  return temp;
}
void sigchld_handler(int sig){
  int status;
  pid_t pid;
  #ifdef DEBUG 
  char* caught = "SIGCHLD caught\n";
  write(1, caught, strlen(caught)+1);
  #endif
  while( (pid = waitpid(-1, &status, WNOHANG | WUNTRACED|WCONTINUED)) >0 ){
    if(WIFEXITED(status)){
      #ifdef DEBUG
      char* child_reap = "Background child reaped\n";
      write(1, child_reap, strlen(child_reap)+1);
    #endif
    }
    else if(WIFSTOPPED(status)){
      #ifdef DEBUG
      char* child_stop = "Child stopped\n";
      write(1, child_stop, strlen(child_stop)+1);
      #endif
    }
    else if(WIFSIGNALED(status)){
      #ifdef DEBUG
      char* child_missed = "Missed signal\n";
      write(1, child_missed, strlen(child_missed)+1);
      #endif
      deletejobfromlist(pid, 0);
    }
    else{
     #ifdef DEBUG
      char* huh = "Child reaped anyway\n";
      write(1, huh, strlen(huh));
     #endif
     }
    update_job(pid, status); 
    
  }
  

}




void sigterm_handler(int sig){
  char* child_int = "SIGTERM signal caught\n";
  write(1, child_int, strlen(child_int) + 1);



}

void sigint_handler(int sig){
  char* term = "SIGINT signal caught\n";
  //char* finished = "SIGINT handler finished\n";
  char* huh = "...but you're in the shell.\n";

  write(1, term, strlen(term) +1);

  if(getpgrp() == shell_pgid){
    write(1, huh, strlen(huh) +1 );
    tcsetpgrp(0, shell_pgid);      

  }  
}


void sigttou_handler(int sig){
char *string = "SIGTTOU Caught\n";

  write(1, string, strlen(string)+1);

}
void sigttin_handler(int sig){
char *string = "SIGTTIN Caught\n";

  write(1, string, strlen(string)+1);
exit(0);
}
void sigcont_handler(int sig){
char *string = "SIGCONT caught\n";

  write(1, string, strlen(string)+1);

}void sigstop_handler(int sig){
char *string = "SIGTSTP caught\n";
char* huh = "But you're already in the shell...\n";  
sigset_t mask, prev_mask;


 
 write(1, string, strlen(string)+1);
  if(fore_pgid == shell_pgid)
    write(1, huh, strlen(huh) +1 );
  else{
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);   
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    tcsetpgrp(0, shell_pgid);
    kill(-shell_pgid, SIGCONT);
    kill(-fore_pgid, SIGSTOP);
    fore_pgid = shell_pgid;


   sigprocmask(SIG_SETMASK, &prev_mask, NULL);
  }
 
}
void sigtstp_handler(int sig){
char *string = "SIGTSTP caught\n";
//char* huh = "But you're already in the shell...\n";  
//sigset_t mask, prev_mask;



 
 write(2, string, strlen(string)+1);
 
}
void signals(int i){

if(!i){
  signal(SIGCHLD, sigchld_handler);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGINT,  SIG_IGN);
}
else{
  signal(SIGCHLD, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGTTIN, SIG_DFL);
  signal(SIGTTOU, SIG_DFL);
  signal(SIGINT, SIG_DFL);
}


 // }

 // signal(SIGTSTP, sigtstp_handler);
 // signal(SIGINT, sigint_handler);
}
int foreground_wait(struct job_struct* job){
 // int i;
  struct process_struct* trav = job->process->next;
  int status; 
  while(trav!=NULL){
    waitpid(trav->pid, &status, WUNTRACED);
    trav->status = status;
    trav = trav->next;
  }
  my_return = status;

  return status;
}

int storespid(int count, int key){
  struct job_struct* trav = job_list->next;
  pid_t pid;
  int status;
  if(trav != NULL)
    spid = trav->grpid;
  else
    spid = -1;
  
  if( (pid = fork()) == 0 ){
    printf("SPID: %d\n", (int)spid);
    exit(0);
  }
  else{
    waitpid(pid, &status, 0); 
    my_return = status;
  }
  
  rl_on_new_line();
  return status;
}

int getspid(int count, int key){
  pid_t pid; 
  int status;
  if(spid == -1){
    if( (pid = fork())==0){
      fprintf(stderr,"SPID has not been set\n");
      exit(-1);
    }
    else{
	waitpid(pid, &status, 0);
        my_return = status;
        return -1;
    }
  }
  else{

    if(spid != fore_pgid){
      printf("Terminating %d\n", (int)spid);
      kill(-spid, SIGTERM);
      deletejobfromlist(spid, 0);
    }
    else{
      printf("Stopping %d\n", (int)spid);
      kill(-spid, SIGSTOP);
    }
  }
  return 0;
}

int ctrl_help(int count, int key){
  pid_t pid;
  int status;
  if( (pid = fork() ) == 0 ){

  help();
  exit(0);
  }
  else{
    waitpid(pid, &status, 0); 
  }
  my_return = status;
  rl_on_new_line();
return 0;
}

int printinfo(int count, int key){
  pid_t pid;
  int status;
  //struct job_struct* trav = job_list->next;
  if( (pid = fork())==0){
    printf("----Info----\n");
    printf("help\n");
    printf("prt\n");
    printf("pwd\n");
    printf("----CTRL----\n");
    printf("cd\n");
    printf("chclr\n");
    printf("chpmt\n");
    printf("pwd\n");
    printf("exit\n");
    printf("----Job Control----\n");
    printf("bg\n");
    printf("fg\n");
    printf("disown\n");
    printf("jobs\n");
    printf("---Number of Commands Run----\n");
    printf("%d\n", commands);
    printf("----Process Table----\n");
    struct job_struct* trav = job_list->next;
    struct process_struct* proc = NULL;
    char** ar = NULL;
    int seconds, minutes, hours;
    seconds = 0;
    minutes = 0;
    hours = 0;
    printf("PGID\tPID\tTIME\tCMD\n");
    while(trav !=NULL){
      printf("%d\t%d\t", (int)trav->grpid, (int)trav->grpid);
      seconds = time(NULL) - trav->timestamp;
      while( (seconds - 60) >= 0){
	seconds -= 60;
	++minutes;
      }
      while( (minutes -60)>= 0){
        minutes -=60;
	++hours;

      }
      printf("%d:%d:%d\t", hours, minutes, seconds);
      proc = trav->process->next;
      while(proc != NULL){
        ar = proc->args;
        while(*(ar)!=NULL){
          printf("%s ", *ar);
	  ++ar;
        }
  
        proc = proc->next;
      }
      printf("\n");
      trav = trav->next;
    }
    /* while there are jobs to print, print them */
    exit(0);

  }
  else{
    waitpid(pid, &status, 0);
    my_return = status;
  }

  rl_on_new_line();
  return 0;
}
