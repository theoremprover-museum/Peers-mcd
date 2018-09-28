/* File of the MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd  */
/* File added by Javeed Chida in April 2000, and modified by MPB in May 2000     */

#ifndef TP_HEURISTIC_H
#define TP_HEURISTIC_H

#include "Interp.h"

/* Values for the Parameter HEURISTIC_MEASURE, which controls the choice */
/* of heuristic measure to use for a peer				 */

#define OCCNEST 0
/* occnest measure of structural complexity - value for HEURISTIC_MEASURE */

#define GOAL_IN_CP 1
/* goal_in_CP measure of goal similarity - value for HEURISTIC_MEASURE    */

#define CP_IN_GOAL 2
/* CP_in_goal measure of goal similarity - value for HEURISTIC_MEASURE    */

#define MAXVAL(a,b) (a>b?a:b)
#define MIN_2(a,b) (a<b?a:b)

/* SINGLE_MATCH and NO_MATCH are factors used to help in distinguishing
   between the three cases that CP_in_Goal and Goal_in_CP test for        */
#define SINGLE_MATCH 3 
#define NO_MATCH 20

extern short Heuristic_symbols[MAX_SYMBOLS];
extern int Used;

/* function prototypes from heuristic.c */

int term_hv(Term_ptr t);
int occnest(Term_ptr t);
int goal_in_CP(Term_ptr t);
int CP_in_goal(Term_ptr t);
void insert_clause_by_hv(Clause_ptr c, List_ptr l);
int weight_phi(Term_ptr t);
int effective_m_f(Term_ptr t, Term_ptr goal);
int occ_fins(Term_ptr tt, Term_ptr goal, Term_ptr s);
int not_in_Heuristic_symbols(short f);
int occ(Symbol_type fsymbol, Term_ptr t);
int nest(Term_ptr s, Term_ptr t);
int hnest(Symbol_type fsymbol, Term_ptr t, int cur, int abs);
int psi(int x);
int sim_GC(Term_ptr u, Term_ptr u_at_p, Term_ptr s, int min_struct);
int sim_CG(Term_ptr u, Term_ptr s, Term_ptr s_at_p);
int find_subcase(int us, int vt);
void build_goal_list(List_ptr usable, List_ptr sos, List_ptr passive);
int min_4(int a, int b, int c, int d);

#endif  /* ! TP_HEURISTIC_H */
