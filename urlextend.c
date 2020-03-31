#include"urlextend.h"
#include<sys/user.h>

int capture(int argc, char** argv){
  /*More of a ptrace-based approach??*/  

  if(argc<=2){ //Not even a website or application is given
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
          case SYS_statfs:
          case SYS_stat:
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
          case SYS_fstatfs:
          case SYS_fstat:
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
          case SYS_set_thread_area:
            if(insyscall==0){
              insyscall=1;
            params[0]=ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
            printf("set_thread_area called with %ld\n", params[0]);
            }else{
              insyscall = 0;
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("set_thread_area returned with %ld\n", eax);
            }
            break;
          case SYS_mprotect:
            if(insyscall==0){
              insyscall=1;
              params[0] = ptrace(PTRACE_PEEKUSER, pid, 4*EBX, NULL);
              params[1] = ptrace(PTRACE_PEEKUSER, pid, 4*ECX, NULL);
              params[2] = ptrace(PTRACE_PEEKUSER, pid, 4*EDX, NULL);
              params[3] = 0l;
              printf("Mprotect called with %ld, %ld, %ld\n", params[0], params[1], params[2]);
            }else{
              eax = ptrace(PTRACE_PEEKUSER, pid, 4*EAX, NULL);
              printf("Mprotect returned with %ld\n", eax);
              insyscall=0;
            }

            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            break;
        /*End switch statement*/
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

      } 

      /*TODO: Finish this part*/
  }




  return 0;
}


