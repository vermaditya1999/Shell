/* A shell program.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


void input(char* argv[]);
void sigint_handler(int SIGNAL);


int
main(void)
{
    
    // Set signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    int pressed = 0;

    while (1)
    {
        pid_t pid;
        char *argv[100];

        // Display shell prompt
        write(1, "(ash) $ ", 8);

        // Take user input
        input(argv);

        if (argv[0] != NULL)
        {
            // Exit if exit command is entered
            if (strcmp(argv[0], "exit") == 0)
            {
                exit(0);
            }

            // Create child process
            if ((pid = fork()) > 0)
            {
                wait(0);
            }
            else if (pid == 0)
            {
                execvp(argv[0], argv);
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
            else
            {
                printf("Fork Error!\n");
            }
        }       
    }
}


/* Takes input from user and splits it in
   tokens into argv. The last element in
   argv will always be NULL. */
void
input(char* argv[])
{
    const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    int i;
    
    buf[0] = '\0';
    fgets((void*) buf, BUF_SIZE, stdin);

    i = 0;
    argv[i] = strtok(buf, "  \n\0");
    while (argv[i] != NULL)
    {
        argv[++i] = strtok(NULL, "  \n\0");
    }
}


/* The child processes created by execvp will
   have their default signal handlers and
   will close on SIGINT. However, we need to
   catch SIGINT from the parent shell process
   for which we will set an empty handler for
   SIGINT */
void
sigint_handler(int SIGNAL)
{
    // Do nothing
}