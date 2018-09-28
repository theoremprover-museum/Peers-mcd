/* File inherited from EQP0.9 and adapted for the         */
/* MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#ifndef TP_CLAUSE_H
#define TP_CLAUSE_H

#define UNIT_CONFLICT_RULE  -1
#define PARA_RULE           -2
#define PARA_FX_RULE        -3
#define PARA_IX_RULE        -4
#define PARA_FX_IX_RULE     -5
#define BACK_DEMOD_RULE     -6
#define DEMOD_RULE          -7
#define COPY_RULE           -8
#define FLIP_RULE           -9
#define ORIENT_RULE         -10      /* Peers-mcd, September 2000 */
/* FLIP_RULE is the rule that infers q=p from an unorientable equation p=q */
/* and keeps both p=q and q=p; this is done by flip_eq_unit() called by    */
/* process_clause().                                                       */
/* The justification of q=p includes flip(x) if x is the id of p=q.        */
/* ORIENT_RULE is the rule that orients an orientable equation q=p into a  */
/* rewrite rule p->q (written p=q) and keeps the oriented version; this is */
/* done by orient_eq_literals() called by process_clause() and             */
/* process_inference_msg().                                                */
/* The justification of p=q includes oriented with no argument, because:   */
/* orient(n) where n is the number of the literal oriented (always 1 if    */
/* there are only unit clauses!) is not correct, because positive numbers  */
/* in a justification are interpreted as identifiers of clauses;           */
/* orient(x) where x is the id of q=p does not work, if orient_eq_literals */
/* is called before forward contracting p=q, hence before giving it an id, */
/* as an id is given only to a clause that survives forward contraction;   */
/* also this solution is dangerous because it would make the justification */
/* of a clause point to the clause itself, possibly causing a loop.        */

struct literal {
    struct literal *next;
    struct term *atom;
    char sign;
    char type;
    };

/* Note that atom is a pointer to struct term, that is, it is of type Term_ptr */
/* if we use the typedef struct term *Term_ptr given in Term.h                    */

typedef struct literal *Literal_ptr;

struct clause {
    int id;
    /* For Peers_mcd prover - October 1996:					    */
    /* <owner, birth_place, num> : id = ((owner*10^2)+birth_place)*10^6+num */
    /* where MAX_PROCESSES <= 10^2					    */
    int weight;
    int interpreted_value;
    struct literal *literals;
    struct gen_ptr *justification;
    struct list_pos *containers;
    int heuristic_value;
    /* For Peers_mcd prover - April 2000:              */
    /* if HEURISTIC_SEARCH is on, then heuristic_value is used in place of  */
    /* weight to: 							    */
    /*   (1) sort list sos					            */
    /* 	 (2) possibly broadcast clause depending on threshold MAX_HV_SEND   */
    };

typedef struct clause *Clause_ptr;

struct list {
    struct list_pos *first, *last;
    struct list *next;  /* for avail list */
    };

typedef struct list *List_ptr;

struct list_pos {
    struct list_pos *prev, *next, *nocc;
    struct list *container;
    struct clause *c;
    };

typedef struct list_pos *List_pos_ptr;

#define CLAUSE_TAB_SIZE 1000

/* Types of literal */

#define ORDINARY_LIT   1
#define EQ_LIT         2
#define ORD_EQ_LIT     3
#define ANS_LIT        4
#define CONSTRAINT_LIT 5


/* function prototypes from clause.c */

Literal_ptr get_literal(void);

void free_literal(Literal_ptr p);

Clause_ptr get_clause(void);

void free_clause(Clause_ptr p);

List_ptr get_list(void);

void free_list(List_ptr p);

void free_list_pos(List_pos_ptr p);

void print_clause_mem(FILE *fp, int heading);

void p_clause_mem();

Gen_ptr_ptr *init_clause_table_id(void);

void store_clause_by_id(Clause_ptr c, Gen_ptr_ptr *tab);

void delete_clause_by_id(Clause_ptr c, Gen_ptr_ptr *tab);

Clause_ptr find_clause_by_id(int id, Gen_ptr_ptr *tab);

void print_clause_table_id(FILE *fp, Gen_ptr_ptr *tab);

void p_clause_table_id(Gen_ptr_ptr *tab);

void zap_clause(Clause_ptr c);

int pos_clause(Clause_ptr c);

int neg_clause(Clause_ptr c);

int literal_count(Clause_ptr c);

Literal_ptr literal_i(Clause_ptr c, int i);

int unit_clause(Clause_ptr c);

Clause_ptr copy_clause(Clause_ptr c);

void copy_clause_nonbasic_marks(Clause_ptr c1, Clause_ptr c2);

Clause_ptr term_to_clause(Term_ptr t);

Term_ptr clause_to_term(Clause_ptr c);

int clause_ident(Clause_ptr c1, Clause_ptr c2);

void list_append(Clause_ptr c, List_ptr l);

void list_prepend(Clause_ptr c, List_ptr l);

void list_insert_before(Clause_ptr c, List_pos_ptr pos);

void list_insert_after(Clause_ptr c, List_pos_ptr pos);

int list_remove(Clause_ptr c, List_ptr l);

int list_remove_all(Clause_ptr c);

int list_member(Clause_ptr c, List_ptr l);

void print_list(FILE *fp, List_ptr l);

void p_list(List_ptr l);

void list_zap(List_ptr l);

void list_check(List_ptr l);

int clause_in_list(Clause_ptr c, List_ptr l);

void clause_up_pointers(Clause_ptr c);

void print_clause(FILE *fp, Clause_ptr c);

void p_clause(Clause_ptr c);

void get_ancestors(Clause_ptr c, Gen_ptr_ptr *id_table, Gen_ptr_ptr *pp);

void get_para_parents(Clause_ptr c, Gen_ptr_ptr *id_table,
		      Clause_ptr *p1, Clause_ptr *p2);
void get_bd_parent(Clause_ptr c, Gen_ptr_ptr *id_table, Clause_ptr *p);
void print_proof(FILE *fp, Clause_ptr c1, Clause_ptr c2, Gen_ptr_ptr *id_table);

int biggest_variable_in_clause(Clause_ptr c);

int distinct_vars(Clause_ptr c);

#endif  /* ! TP_CLAUSE_H */
