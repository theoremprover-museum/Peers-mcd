/* File for the MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#include "mpi.h"

/*************
 *
 *    void print_banner(argc, argv)
 *
 *************/

static void print_banner(int argc, char **argv)
{
    int i;
    char host[64];

    if (gethostname(host, 64) != 0)
	strcpy(host, "???");

    printf("----- Peers_mcd including EQP 0.9d, April 1999 -----\nThe job began on %s, %s", host, get_time());

    printf("The command was \"");
    for(i = 0; i < argc; i++)
        printf("%s%s", argv[i], (i < argc-1 ? " " : ""));
    printf("\".\n\n");
    
}  /* print_banner */

/*************
 *
 *    main
 *
 *************/

main(argc,argv)
int argc;
char *argv[];
{
int myrank;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if (myrank == 0) {
	print_banner(argc, argv);
        peer_init_and_work();
	}
    else
        peer_work();
    MPI_Finalize();
}  /* main */

