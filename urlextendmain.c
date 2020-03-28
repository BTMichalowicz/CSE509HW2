#include "urlextend.h"

int main(int argc, char** argv){

    /*Basic premise:
      wget to get a webpage
      place into temprary file with I/O redirection
      use "word count" to get the webpage
      Fork a few times and wait for the children processes to take care of things
      */
    char filename[8] =".urltemp";

    /*Sanity check for the number of arguments*/
    if(argc<=1){
        fprintf(stderr, "usage: urlextend <function> <url>\n");
        FATAL("Too few arguments: %d", argc);
    }
    char* arg2[5];
    int i = 0;
    for(;i<5;i++){
        arg2[i]=NULL;
    }char* arg3[3];
    for(i=0; i<3;i++){
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
            arg3[0] = argv[1];
            arg3[1] = filename;
            execvp(argv[1], arg3);
            FATAL("%s", strerror(errno));
            break;
        default:
            while((waitpid(pid,0,0)!=-1));
            break;
    }

    unlink(filename);


    


    return 0;
}
