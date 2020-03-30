#include"urlextend.h"
#include<sys/user.h>

int capture(int argc, char** argv){
  /*More of a ptrace-based approach??*/  

  if(argc<=1){
    fprintf(stderr, "usage: urlextend <function> <url>\n");
    FATAL("Too few arguments: %d", argc);
  }

  /*Declarations*/
  char filename[11] = "urltemp.txt";
  pid_t pid;
  long orig_eax, eax;
  long params[4];
  int status;
  int insyscall = 0;
  char* wgetArgs[5];
  char* funcArgs[argc];
  int counter = 0;

  /*Assignments*/
  /*Set up arguments to NULLIFY to avoid any awkward security garbage memory*/
  for(;counter<5;counter++){ wgetArgs[counter] = NULL; }
  for(counter = 0; counter<argc; counter++){ funcArgs[counter] = NULL;}

  for(counter = 0; counter<argc; counter++){
    if(strstr(argv[counter], "http:")!=NULL || strstr(argv[counter], "https:")!=NULL){
      wgetArgs[1] = argv[counter];
      break;
    }
  }

  if(counter == argc){
    fprintf(stderr, "Website not formatted properly. it MUST start with http: or https.\n");
    return EINVAL;
  }


  /*Begin forking*/
  pid = fork();
  switch(pid){
    case -1:
      FATAL("%s", strerror(errno));
      break;
    case 0:
      wgetArgs[0] = "wget";
      wgetArgs[2] = "-O";
      wgetArgs[3] = filename;
      wgetArgs[4] = NULL; /*Just in case NULLIFYING goes awry*/
      execvp("wget", wgetArgs);
      FATAL("%s", strerror(errno));
    default:
      /*Here we go, with all of the ptracing items*/
      /*Taken from the first example from linuxjournal.com/article/6100 */
      while((waitpid(pid, &status, 0)!=-1)){} 
      //ptrace(PTRACE_SYSCALL, pid, NULL, NULL); //placeholder
      //TODO: Setup for stat, open, open, read, mmap, munmap, etc     
  }


  /*Setup for the second function call*/
  pid = fork();
  switch(pid){
    case -1:
      FATAL("%s", strerror(errno));
      break;
    case 0:
      for(counter=1; counter<argc; counter++){
        if(strstr(argv[counter], "http:")!=NULL || strstr(argv[counter], "https:")!=NULL){
          funcArgs[counter-1]=filename;
          continue;
        }
        funcArgs[counter-1] = argv[counter];
      }
      ptrace(PTRACE_TRACEME, 0, NULL, NULL); //TODO: UNCOMMENT WHEN FINISHED
      execvp(argv[1], funcArgs);
      FATAL("%s", strerror(errno));
      break;
    default:
      while(/*waitpid(pid,0,0)!=-1*/1){
        wait(&status);
        if(WIFEXITED(status)) break;
        //if(pid2>0) break;

        orig_eax = ptrace(PTRACE_PEEKUSER, pid, 4*ORIG_EAX, NULL);
        switch(orig_eax){
          case SYS_write:
            if(insyscall==0){
              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = ptrace(PTRACE_PEEKUSER, pid, 4*EDX, NULL);
              params[3] = 0l;
              printf("Write called with %ld, %ld, %ld\n", params[0], params[1], params[2]);
            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Write returned with %ld\n", eax);
              insyscall=0;
            }

            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

            break;
          case SYS_read:
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = ptrace(PTRACE_PEEKUSER, pid, 4*EDX, NULL);
              params[3] = 0l;
              printf("Read called with %ld, %ld, %ld\n", params[0], params[1], params[2]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Read returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
          case SYS_access:
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = 0l;
              printf("Access called with %ld, %ld\n", params[0], params[1]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Access returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
          case SYS_open:
            /*TODO: Consider both 2- and 3-arg variants?*/
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = 0l;
              printf("Open called with %ld, %ld\n", params[0], params[1]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Open returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

            break;
          case SYS_mmap:
            break;
          case SYS_munmap:
            break;
          case SYS_socket: /*For making a connection*/
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = ptrace(PTRACE_PEEKUSER, pid, 4*EDX, NULL);
              params[3] = 0l;
              printf("Socket(2) called with %ld, %ld, %ld\n", params[0], params[1], params[2]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Socket(2) returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
          case SYS_connect:

            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = ptrace(PTRACE_PEEKUSER, pid, 4*EDX, NULL);
              params[3] = 0l;
              printf("Connect(2) called with %ld, %ld, %ld\n", params[0], params[1], params[2]);


            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Connect(2) returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
          case SYS_statfs64:
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = 0l;
              printf("statfs/statfs64 called with %ld, %ld\n", params[0], params[1]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("statfs/statfs64 returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;

          case SYS_fstatfs64:
            if(insyscall==0){

              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = 0l;
              printf("fstatfs64 called with %ld, %ld\n", params[0], params[1]);

            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("ftatfs64 returned with %ld\n", eax);
              insyscall=0;
            }
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;

        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

      } 

      /*TODO: Finish this part*/
  }




  return 0;
}

int bad_approach(int argc, char** argv){


  /*Basic premise:
    wget to get a webpage
    place into temprary file with I/O redirection
    use "word count" to get the webpage
    Fork a few times and wait for the children processes to take care of things
    */
  char filename[11] ="urltemp.txt";

  /*Sanity check for the number of arguments*/
  if(argc<=1){
    fprintf(stderr, "usage: urlextend <function> <url>\n");
    FATAL("Too few arguments: %d", argc);
  }
  char* arg2[5];
  int i = 0;
  for(;i<5;i++){
    arg2[i]=NULL;
  }char* arg3[argc];
  for(i=0; i<argc;i++){
    arg3[i]=NULL;
  }

  uint pid = fork();
  //Setup of a basic wget
  switch(pid){
    case -1:
      FATAL("%s", strerror(errno));
      break;
    case 0:
      /*We set up the child process of wget into an output hidden file*/

      arg2[0]="wget";
      arg2[1]=argv[2];
      printf("%s\n", argv[2]);
      arg2[2]="-O";
      arg2[3]=filename;
      execvp("wget", arg2);
      FATAL("%s", strerror(errno));
      break;
    default:
      while((waitpid(pid,0,0))!=-1); //Reaping any and all child processes
      break;
  }

  /*Primitive Wordcount setup: Do we even need to fork for this?*/
  //Actually, do whatever's in the last set of arguments, may not just be wc, but test here
  //
  pid = fork();

  switch(pid){
    case -1:
      FATAL("%s", strerror(errno));
      break;
    case 0:
      //char* arg3[2] = {"wc", filename};
      for(i = 1; i<argc;i++){
        if(strstr(argv[i], ".")!=NULL){
          arg3[i-1]=filename;
          continue;
        }
        arg3[i-1] = argv[i];
      }

      //arg3[0] = argv[1];
      //arg3[1] = filename;
      execvp(argv[1], arg3);
      FATAL("%s", strerror(errno));
      break;
    default:
      while((waitpid(pid,0,0)!=-1));
      break;
  }




  return 0;
}
