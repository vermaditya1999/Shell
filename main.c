/* A shell program.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


const int INP_BUF_SIZE = 1024;


void input(char* argv[], char buf[]);
void sigint_handler(int SIGNAL);


int
main(void)
{
    
    // Set signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    while (1)
    {
        pid_t pid;
        char *tokens[100];
        char buf[INP_BUF_SIZE];

        // Display shell prompt
        write(1, "(ash) $ ", 8);

        // Take user input
        input(tokens, buf);

        if (tokens[0] != NULL)
        {
            // Exit if exit command is entered
            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }

            // Create child process
            if ((pid = fork()) > 0)
            {
                wait(NULL);
            }
            else if (pid == 0)
            {
                execvp(tokens[0], tokens);
                printf("%s: Command not found\n", tokens[0]);
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
input(char* tokens[], char buf[])
{    
    buf[0] = '\0';
    fgets(buf, INP_BUF_SIZE, stdin);

    tokens[0] = strtok(buf, "  \n\0");
    for (int i = 1; tokens[i] != NULL; ++i)
    {
        tokens[i] = strtok(NULL, "  \n\0");
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
