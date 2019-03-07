/* A shell program.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define close_pipe(fd) close(fd[0]); close(fd[1]);


const int BUF_SIZE = 1024;
const int MAX_TOKENS = 128;
const int MAX_CMDS = 16;
const int CMD_SIZE = 16;


void input(char* argv[], char buf[]);
void sigint_handler(int SIGNAL);
int partition_tokens(char *tokens[], char *cmds[MAX_CMDS][CMD_SIZE]);
void set_fd(char *cmd[]);
void filter_argv(char *argv[], char *cmd[]);
void run_cmds(int cmd_ind, char *cmds[MAX_CMDS][CMD_SIZE]);


int
main(void)
{
    // Set signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    while (1)
    {
        int num_cmds;
        char buf[BUF_SIZE];
        char *tokens[MAX_TOKENS];
        char *cmds[MAX_CMDS][CMD_SIZE];

        // Display shell prompt
        write(1, "(ash) $ ", 8);

        // Take user input
        input(tokens, buf);

        // Partition tokens into cmds array
        num_cmds = partition_tokens(tokens, cmds);

        if (num_cmds > 0)
        {
            pid_t pid;
            
            if (!strcmp("exit", cmds[0][0]))
            {
                exit(0);
            }

            if ((pid = fork()) > 0)
            {
                wait(NULL);
            }
            else if (pid == 0) {
                run_cmds(num_cmds - 1, cmds);
                exit(0);
            }
            else if (pid == -1)
            {
                printf("Fork error!\n");
            }
        }    
    }
}


/* The child processes created by execvp will
   have their default signal handlers and
   will close on SIGINT. However, we need to
   catch SIGINT from the top most parent
   process shell for which we will set an empty
   handler for SIGINT */
void
sigint_handler(int SIGNAL)
{
    // Do nothing
}


/* Takes input from user and splits it in
   tokens into argv. The last element in
   argv will always be NULL. */
void
input(char* tokens[], char buf[])
{
    buf[0] = '\0';
    fgets(buf, BUF_SIZE, stdin);

    tokens[0] = strtok(buf, "  \n\0");
    for (int i = 0; tokens[i] != NULL; ++i)
    {
        tokens[i + 1] = strtok(NULL, "  \n\0");
    }
}


/* Parition the array of tokens into NULL terminated
   arrays of strings into the cmds array. The paritioning
   is done using the delimiter pipe "|" and NULL.
   The argument tokens must be a NULL terminated array
   of strings.
   The function returns the total number of cmds. */ 
int
partition_tokens(char *tokens[], char *cmds[MAX_CMDS][CMD_SIZE])
{
    if (tokens[0] == NULL) return 0;

    int num_cmds = 0;
    int cmds_i = 0;
    int col_i = 0;
    int tok_i = 0;

    while (1)
    {
        if (tokens[tok_i] == NULL)
        {
            cmds[cmds_i][col_i] = NULL;
            ++num_cmds;
            break;
        }
        else if (!strcmp(tokens[tok_i], "|"))
        {
            cmds[cmds_i][col_i] = NULL;
            ++cmds_i;
            ++tok_i;
            col_i = 0;
            ++num_cmds;
        }
        else {
            cmds[cmds_i][col_i] = tokens[tok_i];
            ++col_i;
            ++tok_i;
        }
    }

    return num_cmds;
}


/* Run all the commands in the cmds array and
   pipe them sequentially. */
void 
run_cmds(int cmd_ind, char *cmds[MAX_CMDS][CMD_SIZE])
{
    char *argv[CMD_SIZE];
    
    filter_argv(argv, cmds[cmd_ind]);
    set_fd(cmds[cmd_ind]);

    if (cmd_ind > 0)
    {
        pid_t pid;
        int fd[2];
        
        if (pipe(fd))
        {
            printf("Pipe error!\n");
        }
        else
        {
            if ((pid = fork()) > 0)
            {
                close(0);
                dup(fd[0]);

                close_pipe(fd);

                execvp(argv[0], argv);
                printf("%s: Command not found.\n", argv[0]);
            }
            else if (pid == 0)
            {
                close(1);
                dup(fd[1]);

                close_pipe(fd);

                run_cmds(cmd_ind - 1, cmds);
            }
            else
            {
                printf("Fork error!\n");
            }
            
        }
        
    }
    else
    {
        execvp(argv[0], argv);
        printf("%s: Command not found.\n", argv[0]);
    }
}


/* Filter command and arguments from the cmd
   array into argv array */
void
filter_argv(char *argv[], char *cmd[])
{
    int argv_i = 0;
    int cmd_i = 0;

    while (1)
    {
        if (cmd[cmd_i] == NULL)
        {
            argv[argv_i] = NULL;
            break;
        }
        else
        {
            if (!strcmp(cmd[cmd_i], ">") || !strcmp(cmd[cmd_i], ">>") || !strcmp(cmd[cmd_i], "<"))
            {
                cmd_i += 2;
            }
            else if (sizeof(cmd[cmd_i]) * sizeof(char) > 2 && *(cmd[cmd_i] + 1) == '>')
            {
                ++cmd_i;
            }
            else
            {
                argv[argv_i++] = cmd[cmd_i++];
            }
        }
    }
}


/* Set the file descriptors according to the
   IO redirections specified in the cmd array.
   The input is assumed to be valid. */
void
set_fd(char *cmd[])
{
    for (int i = 0; cmd[i] != NULL; ++i)
    {
        int fd;

        // command > filename
        if (!strcmp(cmd[i], ">"))
        {
            fd = open(cmd[i + 1], O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
            close(1);
            dup(fd);
        }
        // command >> filename
        else if (!strcmp(cmd[i], ">>"))
        {
            fd = open(cmd[i + 1], O_APPEND | O_RDWR, S_IRWXU);
            if (fd < 1) fd = open(cmd[i + 1], O_CREAT | O_RDWR, S_IRWXU);
            close(1);
            dup(fd);
        }
        // command < filename
        else if (!strcmp(cmd[i], "<"))
        {
            fd = open(cmd[i + 1], O_RDONLY | O_RDWR, S_IRWXU);
            close(0);
            dup(fd);
        }
        else if (sizeof(cmd[i]) * sizeof(char) > 2 && *(cmd[i] + 1) == '>')
        {
            // 2>&1
            if (*(cmd[i] + 2) == '&' && *(cmd[i] + 3) == '1') 
            {
                close(2);
                dup(1);
            }
            // 1>filename
            else if (*(cmd[i] + 0) == '1')
            {
                fd = open(cmd[i] + 2, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
                close(1);
                dup(fd);
            }
            // 2>filename
            else if (*(cmd[i] + 0) == '2')
            {
                fd = open(cmd[i] + 2, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
                close(2);
                dup(fd);
            }
        }
    }
}