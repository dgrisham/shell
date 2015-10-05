/*********************************/
/*           FORK EXAMPLE             */
/********************************/

#include <iostream>
#include <errno.h>      /* include file for perror */ 
#include <unistd.h>     /* include file for fork/exec */ 
#include <sys/types.h>  /* include file for wait */ 
#include <sys/wait.h>   /* include file for wait */ 
#include <stdio.h>
using namespace std; 

main()
{
  int cpid;
  int cpid2;

  if ((cpid = fork()) == -1) {
    perror("fork failed");
    return(-1);
  }

  cout << "The child is started." << endl << endl;

  if (cpid == 0) {       /* This is the child. */ 

    cout << endl << "I'm the child and my pid is " << getpid() << endl; 
    cout << "I'm waiting for the execute sacrifice." << endl << endl;

    execl("/bin/ls", "ls", "-l", (char *)0);   /* execute a process */ 

    perror("exec failed");
    return(-1);
  } 

  else {                 /* This is the parent. */ 

    cout << endl << "I'm the parent. I'll wait on my child." << endl;

    cpid2 = wait( (int *) 0);    /* wait for child to finish */ 

    cout << endl << "The child with pid = " << cpid2 << " is done." << endl;
    return(0); 
  }
}

