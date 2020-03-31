# urlextend tool by Benjamin Michalowicz
## CSE 509 HW 2 Project
### Purpose: System call interception to access remote URLs as if they were local files


1. How to use:
    Type `make` to compile the binary and object files.
    `./urlextend <application with flags> <webpage>`

2. Purpose: Be able to run Linux shell commands on remote URLs
    These include `less`, `wc`, `cat`, etc.

3. Urlextend intercepts system calls to analyze appplications' use and provide information in one easy step, using fork/execvp calls to `wget(1) and whichever terminal application the user decides to use.
