/* MODIFIED CLAUSE DIFFUSION theorem prover */

/* Flags, parameters and statistics and clocks */

/*************
 *
 *    Flags are boolean valued options.  To install a new flag, append
 *    a new name and index to the end of this list, then insert code to
 *    initialize it in the routine `init_options'.
 *    Example access:  if (Flags[PARA_FROM_LEFT].val) { ... }
 *    See routine `init_options' for defaults.
 *
 *************/

#define MAX_FLAGS          100  /* increase if necessary */

#define DISPLAY_TERMS        0  /* print terms in internal format */
#define CHECK_ARITY          1  /* check if equal symbols have equal arities */
#define PROLOG_STYLE_VARIABLES 2
#define PRINT_GEN            4
#define DEMOD_HISTORY        6
#define INDEX_DEMOD          8
#define INDEX_AC_ARGS        9
#define PRINT_GIVEN         10
#define ALPHA_PAIR_WEIGHT   12
#define CHECK_INPUT_DEMODULATORS   13
#define INPUT_CONFLICT_CHECK       14
#define LRPO                15
#define PARA_PAIRS          16
#define PRINT_FORWARD_SUBSUMED  17
#define PRINT_BACK_DEMOD  	18
#define PRINT_LISTS_AT_END  	19

/* Flags for Peers prover */
/* The default value is 0 (off) for all of them.			      */
#define OWN_IN_USABLE		50
/* If it is on, each process owns its copy of the input Usable clauses.       */
/* Not compatible with proof reconstruction (though it is rarely a problem).  */
#define OWN_IN_SOS		51
/* If it is on, each process owns its copy of the input Sos clauses.          */
/* Not compatible with proof reconstruction (though it is rarely a problem).  */
#define TWO_WAY_DEMOD		52
/* Allows to use unoriented equations as simplifiers in both directions       */
#define EAGER_BD_DEMOD		53
/* If off, only owner of a clause generates its reduced form by backward      */
/* demodulation, others only delete the reducible clause; if on, all processes*/
/* generate reduced forms by backward demodulation regardless of ownership.   */
#define OWN_GOALS		54
/* This flag assumes that the problem is purely equational, and that all      */
/* negated equations in Sos are goals. If it is on, goals in Sos are treated  */
/* in a purely local manner: each process owns its copy of input goals in Sos */
/* (same as if OWN_IN_SOS were on and applied only to negated equations);     */
/* each process owns and does not broadcast the goals it generates.           */

/* end of Flags */

/*************
 *
 *    Parms are integer valued options.  To install a new parm, append
 *    a new name and index to this list, then insert code to
 *    initialize it in the routine `init_options'.
 *    Example access:  if (Parms[FPA_LITERALS].val == 4) { ... }
 *    See routine `init_options' for defaults.
 *
 *************/

#define MAX_PARMS        30  /* increase if necessary */

#define MAX_MEM           0  /* stop search after this many K bytes allocated */
#define MAX_WEIGHT        1
#define MAX_GIVEN         2
#define WEIGHT_FUNCTION   3
#define MAX_PAIR_WEIGHT   4  /* For paramodulation-by-pairs. */
#define MAX_PROOFS        5
#define REPORT_GIVEN      6
#define AC_SUPERSET_LIMIT 8
#define MAX_SECONDS       9
#define PICK_GIVEN_RATIO 10

/* Parameters for Peers prover */

#define DECIDE_OWNER_STRAT  20
/* determines the allocation strategy */
#define SWITCH_OWNER_STRAT  21
/* it establishes after how many choices to switch allocation strategy */

/* end of Parms */

/*************
 *
 *    Statistics.  To install a new statistic, append a new name and
 *    index to this list, then insert the code to output it in the
 *    routine `print_stats'.
 *    Example access:  Stats[INPUT_ERRORS]++;
 *
 *************/

#define MAX_STATS        40  /* increase if necessary */

#define INPUT_ERRORS      0
#define K_MALLOCED        1
#define BT_OCCUR_CHECKS   2
#define REWRITES          3
#define FPA_OVERLOADS     4
#define FPA_UNDERLOADS    5
#define AC_INITIATIONS    6
#define AC_CONTINUATIONS  7
#define REWRITE_ATTEMPTS  8

#define CLAUSES_GIVEN             9
#define CLAUSES_KEPT             10
#define CLAUSES_FORWARD_SUBSUMED 11
#define CLAUSES_WT_DELETED       12
#define CLAUSES_GENERATED        13
#define CLAUSES_BACK_DEMODULATED 14

#define PROOFS                   15
#define INIT_WALL_SECONDS        16

/* Stats for Peer prover */

#define RESIDENTS		20
#define VISITORS		21
#define RESIDENTS_IN_SOS	22
#define VISITORS_IN_SOS		23
#define EXP_GENERATED    	24
#define IM_RECEIVED      	25
#define IM_BROADCAST     	26
#define BD_RESIDENTS     	27
#define BD_VISITORS      	28
#define PROOF_LENGTH		29	/* Measured as number of clauses */

/* end of Stats */

/*************
 *
 *    Clocks.  To install a new clock, append a new name and
 *    index to this list, then insert the code to output it in the
 *    routine `print_times'.  Example of use: CLOCK_START(INPUT_TIME),
 *    CLOCK_STOP(INPUT_TIME),  micro_sec = clock_val(INPUT_TIME);.
 *    See files macros.h and clocks.c.
 *
 *************/

#define MAX_CLOCKS          40
 
#define RUN_TIME             1
#define INPUT_TIME           0
#define DEMOD_TIME           2
#define BD_FIND_TIME         3
#define LRPO_TIME            4

/* Clocks for Peers prover */

#define INF_MSG_TIME         20
#define INFERENCE_TIME       22

/* end of Clocks */

