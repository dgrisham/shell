/*********************************/
/*           PIPE EXAMPLE              */
/********************************/

#include <iostream>
#include <errno.h>  /* include file for perror */
#include <unistd.h> /* include file for pipes */ 
#include <stdio.h>
#define DATA "There are 10 types of people ... "
using namespace std; 

int main()
{
    int the_pipe[2];
    int child;

    if (pipe(the_pipe) < 0) {           /* Creates a pipe */
        perror("opening pipe");
        return(-1); 
    }

    if ((child = fork()) == -1) {
        perror("fork");
        return(-1); 
    }

    if (child == 0) {       /* This is the child. */ 
        close(the_pipe[0]);   /* It writes msg to its parent */

        cout << "I'm the child; I have something important to say." << endl << endl; 

        if (write(the_pipe[1], DATA, sizeof(DATA)) < 0) 
            perror("writing message");

        close(the_pipe[1]);

    } else {
        char buf[1024];       /* This is the parent. */
        close(the_pipe[1]);   /* It reads the child's message */

        cout << "I'm the parent; I should listen to my child." << endl << endl; 

        if (read(the_pipe[0], buf, 1024) < 0) 
            perror("reading message");

        cout << "My child says: " << buf << endl; 

        close(the_pipe[0]);
    }

        return 0;
}
