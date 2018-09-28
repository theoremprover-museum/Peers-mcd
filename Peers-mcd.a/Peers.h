/* MODIFIED CLAUSE DIFFUSION theorem prover */

#include "Clause.h"  /* For CLAUSE_TAB_SIZE and hash tables of clauses. */

#define MAX_SYMBOL_ARRAY   200
#define MAX_DAC_STRING    10000
#define MAX_PROCESSES      100

/* types of work---messages and local work---and their priorities */
 
#define HALT_MSG		 1

#define OPTIONS_MSG		 2
#define SYM_TAB_MSG		 3
#define INPUT_PASSIVE_MSG	 4
#define INPUT_SOS_MSG		 5
#define INPUT_USABLE_MSG	 6
#define INPUT_DEMOD_MSG		 7
#define INFERENCE_MSG		 8
#define TOKEN_MSG		 9
#define INFERENCE_WORK		10

#define NUMBER_OF_WORK_TYPES	10

/* values for the Parameter DECIDE_OWNER_STRAT, which controls the choice */
/* of the strategy to allocate clauses to processes			  */

#define ROTATE			 1	/* default */
/* rotate among processes */
#define SYNTAX			 2
/* based on the clause syntax */
#define SELECT_MIN		 3
/* based on work-load of processes */
#define PARENTS			 4
/* based on ids of from_parent and into_parent so that all clauses children */
/* of same parents are assigned to the same process */
#define HISTORY			 5
/* based on ids of from_parent, into_parent, bd_parent and demod_parents */
/* taken as representing the history of the clause */
#define R_PARENTS		 6
/* ROTATE on input clauses, PARENTS on all other clauses */
#define R_HISTORY		 7
/* ROTATE on input clauses, HISTORY on all other clauses */
#define ALL_PARENTS		 8
/* based on ids of from_parent and into_parent or bd_parent, so that all */
/* clauses children of same parents are assigned to the same process */
#define R_ALL_PARENTS		 9
/* ROTATE on input clauses, ALL_PARENTS on all other clauses */
#define R_AGOP			10
/* ROTATE for as many choices as defined by the value of the parameter 	*/
/* SWITCH_OWNER_STRAT choices, AGOP (allocation strategy based on 	*/
/* Ancestor-Graph Overlap measured on parents = from_parent and into_parent */
/* owned and born) afterwards. */
#define P_AGOA			11
/* PARENTS for as many choices as defined by the value of the parameter 	*/
/* SWITCH_OWNER_STRAT choices, AGOA (allocation strategy based on 	*/
/* Ancestor-Graph Overlap measured on all_parents = from_parent, into_parent */
/* and bd_parent owned) afterwards. */

/****************** Global variables for peer code ************/

#ifdef OWNER_OF_PEER_GLOBALS
#define PEER_GLOBAL         /* empty string if included by owner file */
#else
#define PEER_GLOBAL extern  /* extern if included by any other file */
#endif

PEER_GLOBAL int Pid;              /* Peer ID */
PEER_GLOBAL int Number_of_peers;

/* fast symbol table */

PEER_GLOBAL struct sym_ent Symbol_array[MAX_SYMBOL_ARRAY];

/* clause lists */

PEER_GLOBAL struct list *Usable;
PEER_GLOBAL struct list *Sos;
PEER_GLOBAL struct list *Demodulators;
PEER_GLOBAL struct list *Passive;
PEER_GLOBAL struct list *Deleted_clauses;

/* indexes */

PEER_GLOBAL struct discrim *Demod_index;

PEER_GLOBAL int Peers_assigned_clauses[MAX_PROCESSES];
/* Peers_assigned_clauses[i] is the number of clauses that Pid assigned */
/* to i so far. It is used by the naming scheme to determine the third  */
/* component (num) of the id of a clause. 				*/

PEER_GLOBAL int Peers_work_load[MAX_PROCESSES];
/* Peers_work_load[i] for i =/= Pid is the number of inference messages */
/* that Pid received from i so far. Peers_work_load[Pid] is the number  */
/* of inference messages that Pid sent so far. Peers_work_load[i] is	*/
/* used as an estimate of the work-load of peer i by decide_owner()	*/
/* with the SELECT_MIN criterion.					*/

PEER_GLOBAL int Msgs_diff;
/* Used by the termination detection algorithm. It is incremented whenever */
/* a message is sent and decremented whenever a message is received. The   */
/* sum of the Msgs_diff at all the peers will be 0 when all sent messages  */
/* have been received.							   */

PEER_GLOBAL int Have_token;
/* Used by the termination detection algorithm: 1 if Pid has the token,    */
/* 0 otherwise.								   */

PEER_GLOBAL int How_many_times_token_received;
/* Used by the termination detection algorithm, so that Peer 0 can detect  */
/* to have received the token twice.					   */

/* The following array is used for building long messages. */

PEER_GLOBAL char Work_str[MAX_DAC_STRING];



