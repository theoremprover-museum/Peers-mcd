/* File inherited from EQP0.9d and adapted for the */
/* MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#ifndef TP_OPTIONS_H
#define TP_OPTIONS_H

/* This file does not depend on header.h */

#include <stdio.h>
#include <limits.h>  /* for INT_MAX */
#include <string.h>  /* for strcmp() */

/*************
 *
 *    Flags are boolean valued options.  To install a new flag, append
 *    a new name and index to the end of this list, then insert code to
 *    initialize it in the routine `init_options'.
 *    Example access:  if (Flags[PARA_FROM_LEFT].val) { ... }
 *    See routine `init_options' for defaults.
 *
 *************/

#define MAX_FLAGS              100  /* increase if necessary */

#define DISPLAY_TERMS        0  /* print terms in internal format */
#define CHECK_ARITY          1  /* check if equal symbols have equal arities */
#define PROLOG_STYLE_VARIABLES   2
#define PRINT_GEN                4
#define DEMOD_HISTORY            6
#define PRINT_GIVEN             10
#define PRINT_PAIRS             11
#define LRPO                    15
#define PARA_PAIRS              16
#define PRINT_FORWARD_SUBSUMED  17
#define PRINT_BACK_DEMOD  	18
#define PRINT_LISTS_AT_END  	19
#define NO_DEMODULATION         20

#define INDEX_BT_DEMOD          21
#define INDEX_AC_ARGS           22
#define INDEX_PARAMOD           23
#define INDEX_BD                24
#define INDEX_BS                25
#define INDEX_FS                26
#define INDEX_CONFLICT          27

#define PRINT_KEPT              28
#define PRINT_NEW_DEMOD         29
#define DELAY_BACK_DEMOD        30
#define DELAY_NEW_DEMOD         31
#define BACK_DEMOD_SOS          32

#define ORDERED_PARAMOD         33
#define AC_EXTEND               34
#define FUNCTIONAL_SUBSUME      35
#define BASIC_PARAMOD           36
#define PRIME_PARAMOD           37
/* blocked paramodulation, i.e. paramodulation with blocking */
#define FPA_DELETE              38

/* Flags for Peers_mcd prover - October 1996 */

/* The default value is 0 (off) for all following flags.		   */
#define EAGER_BD_DEMOD		50
/* If off, only owner of a clause generates its reduced form by backward   */
/* demodulation, others only delete the reducible clause according to	   */
/* Modified Clause-Diffusion; if on, all processes generate reduced forms  */
/* by backward demodulation regardless of ownership as in Clause-Diffusion.*/
#define OWN_GOALS		51
/* This flag assumes the problem is purely equational, and all negated     */
/* equations in Sos are goals. If it is on, goals in Sos are treated       */
/* in a purely local manner: each process owns its copy of input goals     */
/* in Sos, each process owns and does not broadcast the goals it generates.*/

/* Flags for Peers_mcd prover - March 1999 */

/* These flags provide Peers_mcd with some multi-search ability.        */
#define DIVERSE_SEL		52
/* DIVERSE_SEL controls the premise-selection function of the search    */
/* plan: if it is off (default), every process uses the pairs algorithm */
/* if PARA_PAIRS is on, the given clause algorithm otherwise; if it is  */
/* on, processes of even number (including 0) set PARA_PAIRS on, and    */
/* processes of odd number set PARA_PAIRS off, so that half use the     */
/* pairs algorithm and the other half use the given clause algorithm.   */
/* If it is on, DIVERSE_SEL overwrites PARA_PAIRS.                                 */
#define DIVERSE_PICK		53
/* DIVERSE_PICK diversifies the premise-selection function by           */
/* diversifying PICK_GIVEN_RATIO: if it is off (default) all processes  */
/* use the same value of PICK_GIVEN_RATIO; if it is on, every process   */
/* sets its value of PICK_GIVEN_RATIO to the input value + Pid, so      */
/* that every process has a different value of PICK_GIVEN_RATIO.        */
/* it is used only if PICK_GIVEN_RATIO is set, that is                  */
/* PICK_GIVEN_RATIO!= -1 (-1 is the default value when not used)        */

/* Flags for Peers-mcd prover - April 2000 */

#define HEURISTIC_SEARCH         54
/* Enables the computation and assignment of heuristic values to each   */
/* clause; list sos is then kept sorted by heuristic value; also        */
/* clauses are broadcast only if their HEURISTIC_VALUE is less than a   */
/* threshold, MAX_HV_SEND.						*/

/* Flags for Peers-mcd prover - September 2000 */

#define PRINT_MSGS               55

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
#define MAX_PROOFS        5
#define REPORT_GIVEN      6
#define AC_SUPERSET_LIMIT 8
#define MAX_SECONDS       9
#define PICK_GIVEN_RATIO 10
#define FPA_DEPTH        11
#define PAIR_INDEX_SIZE  12
#define MAX_VARIABLES    13
#define PARA_KEY         14  /* in eqp0.9d, not in eqp0.9 and eqp0.9c */

/* Parameters for Peers_mcd prover - October 1996 */

#define DECIDE_OWNER_STRAT  20
/* determines the allocation strategy; possible values defined in Peers.h; default is ROTATE */
#define SWITCH_OWNER_STRAT  21
/* it establishes after how many choices to switch allocation strategy */

/* Parameters for Peers_mcd prover - March 1999 */

#define MAX_WEIGHT_SEND	    22
/* clauses of smaller or equal weight are regarded as good and broadcast */
/* when Peers-mcd does pure multi-search (DECIDE_OWNER_STRAT=NO_SUBDIVIDE)    */


/* Parameters for Peers_mcd prover - April 2000 */

#define HEURISTIC_MEASURE 23
/* It establishes which heuristic measure should be used for clause       */
/* selection. If HEURISTIC_SEARCH is on, then a peer assigns to           */
/* HEURISTIC_MEASURE its Pid % 3. Possible values defined in Heuristic.h */

#define MAX_HV_SEND 24
/* This is the threshold value that the heuristic value of a clause must  */
/* be less than in order for that clause to be selected for communication.*/
/* Used only if HEURISTIC_SEARCH is on					  */

/* end of Parms */

struct flag {  /* Flags are boolean valued options */
    char *name;
    int val;
    };

struct parm {  /* Parms are integer valued options */
    char *name;
    int val;
    int min, max;  /* minimum and maximum permissible values */
    };

extern struct flag Flags[MAX_FLAGS];
extern struct parm Parms[MAX_PARMS];

/* function prototypes from options.c */

void init_options(void);

void print_options(FILE *fp);

void p_options(void);

void auto_change_flag(FILE *fp, int index, int val);

void dependent_flags(FILE *fp, int index);

void auto_change_parm(FILE *fp, int index, int val);

void dependent_parms(FILE *fp, int index);

int change_flag(FILE *fp, char *flag_name, int set);

int change_parm(FILE *fp, char *parm_name, int val);

void check_options(FILE *fp);

#endif  /* ! TP_OPTIONS_H */
