/* MODIFIED CLAUSE DIFFUSION theorem prover */

#include "mpi.h"	/* for MPI constants */
#include "Header.h"
/* Includes in turn Stats.h, needed for MAX_INTERNAL_FLAGS, and Options.h, */
/* needed for MAX_FLAGS and MAX_PARMS.					   */
#include "Symbols.h"	/* for MAX_NAME */
#include "List.h" 	/* for Gen_ptr_ptr */
#include "Clause.h" 	/* for Clause_ptr */
#include "Messages.h" 	/* for Symbol_array and MAX_SYMBOL_ARRAY */

/*************
 *
 *     send_string(buf, count, dest, tag)	Peers_mcd, November 1997
 *
 *************/

void send_string(char *buf, int count, int dest, int tag)
{
   MPI_Send(buf, count, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    
}  /* send_string */

/*************
 *
 *     receive_string(buf, count, from, exp_tag, act_tag)	Peers_mcd, November 1997
 *
 *************/

void receive_string(char *buf, int count, int *from, int exp_tag, int *act_tag)
{    
   MPI_Status status;
   int i;
   
   for (i=0; i < count; i++)
   	buf[i] = '\0';
   /* It makes sure the buffer is clean because MPI_Recv does not clears the */
   /* buffer at the locations past the length of the incoming message. */
   
   MPI_Recv(buf, count, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   *from = status.MPI_SOURCE;
   *act_tag = status.MPI_TAG;
#if 0
   if (exp_tag != status.MPI_TAG)
	fprintf(Peer_fp, "WARNING: received message of type %d after a message of type %d was probed.\n", status.MPI_TAG, exp_tag);
#endif
}  /* receive_string */

/*************
 *
 *     broadcast_string(buf, count, tag)	Peers_mcd, November 1997
 *
 *************/

void broadcast_string(char *buf, int count, int tag)
{
    int i;

    for (i = 0; i < Number_of_peers; i++)
	if (i != Pid)
	    MPI_Send(buf, count, MPI_CHAR, i, tag, MPI_COMM_WORLD);
    
}  /* broadcast_string */

/*************
 *
 *     messages_available(tag)	Peers_mcd, November 1997
 *
 *************/

int messages_available(int tag)
{
    int flag;
    MPI_Status status;

    MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &status);
  
    return(flag);
}  /* messages_available */

/*************
 *
 *     options_to_string(s, size)
 *
 *************/

void options_to_string(char *s, int size)
{
    int i, count;
    char s1[20];

    count = 0;
    s[0] = '\0';
    for (i=0; i<MAX_INTERNAL_FLAGS; i++) {
	sprintf(s1, "%d ", Internal_flags[i]);
	count += strlen(s1);
	if (count >= size)
	    abend("in options_to_string, too many internal flags.");
	strcat(s, s1);
	}
    for (i=0; i<MAX_FLAGS; i++) {
	sprintf(s1, "%d ", Flags[i].val);
	count += strlen(s1);
	if (count >= size)
	    abend("in options_to_string, too many flags.");
	strcat(s, s1);
	}
    for (i=0; i<MAX_PARMS; i++) {
	sprintf(s1, "%d ", Parms[i].val);
	count += strlen(s1);
	if (count >= size)
	    abend("in options_to_string, too many parms.");
	strcat(s, s1);
	}
}  /* options_to_string */

/*************
 *
 *     string_to_options(s)
 *
 *************/

void string_to_options(char *s)
{
    char *s1;
    int i;

    s1 = s;
    
    for (i=0; i<MAX_INTERNAL_FLAGS; i++)
	Internal_flags[i] = next_int(&s1);
	
    for (i=0; i<MAX_FLAGS; i++)
	Flags[i].val = next_int(&s1);

    for (i=0; i<MAX_PARMS; i++)
	Parms[i].val = next_int(&s1);
    
}  /* string_to_options */

/*************
 *
 *     more_in_string() - is there some non-space followed by a space?
 *
 *************/

int more_in_string(char *s)
{
    while (*s == ' ') s++;
    for (; *s != ' ' && *s != '\0'; s++);
    return(*s == ' ');
}  /* more_in_string */

/*************
 *
 *     next_str(sp, s2)
 *
 *     Assumes there is a string followed by a space.  Update sp.
 *
 *************/

void next_str(char **sp, char *s2)
{
    char *s;
    int i;

    s = *sp;
    while (*s == ' ') s++;
    for (i=0; *s != ' '; i++, s++)
	s2[i] = *s;
    s2[i] = '\0';
    *sp = s;
}  /* next_str */

/*************
 *
 *     next_int(sp)
 *
 *     Assumes there is an integer followed by a space.  Update sp.
 *
 *************/

int next_int(char **sp)
{
    char s1[MAX_NAME];

    next_str(sp, s1);
    return(atoi(s1));
}  /* next_int */

/*************
 *
 *     sym_array_to_normal_sym_tab()
 *
 *************/

void sym_array_to_normal_sym_tab(a, size)
struct sym_ent a[];
int size;
{
    int i, sn;
    struct sym_ent *p;

    for (i=0; i<size; i++) {
	if (a[i].sym_num < 0) {
	/* Peers_mcd, November 1997: sym_num's are negative in EQP0.9 */
	    sn = str_to_sn(a[i].name, a[i].arity);
	    if (sn != a[i].sym_num)
	/* The correct state is sn == a[i].sym_num and abs(sn) == i */
		abend("sym_array_to_normal_sym_tab, sym_num mixup.");

	    p = sn_to_node(sn);
	/* Name and arity are inserted by sn_to_node if not already there. */
	    if (a[i].assoc_comm)
		p->assoc_comm = 1;
	    else if (a[i].commutative)
		p->commutative = 1;
	    p->lrpo_status = a[i].lrpo_status;
	    p->lex_val = a[i].lex_val;
	    p->skolem = a[i].skolem;
	    }
	}
}  /* sym_array_to_normal_sym_tab */

/*************
 *
 *    string_to_sym_array(s, a, a_size)
 *     
 *    string has a sequence of tuples:
 *    
 *    <symbol-number, arity, unif-type, name, lrpo-status, lex-val, skolem >
 *
 *    unif-type and name are strings, others are integers.
 *    There should be exactly one space after each object. 
 *
 *    skolem field added for Peers_mcd, November 1997
 *
 *************/

void string_to_sym_array(s, a, a_size)
char *s;
struct sym_ent a[];
int a_size;
{
    char *s1, s2[MAX_NAME];
    int i, j, k, posk;

    for (i = 0; i < a_size; i++)
	a[i].sym_num = 0;
    /* Peers_mcd, November 1996: in older versions sym_num's are positive */
    /* and here a is initialized to all -1's. In EQP0.9 the sym_num's are */
    /* negative, so that a is initialized to all 0's.			  */
    s1 = s;
    
    while (more_in_string(s1)) {
    
	k = next_int(&s1);
	posk = abs(k);
	/* Peers_mcd, November 1996: the first int in each tuple is the */
	/* sym_num and it's negative. Thus, we take its abs as value of its */
	/* position in sym_array a.					*/
	
	if (posk >= a_size)
	    abend("string_to_sym_array, array too small.");
	a[posk].sym_num = k;

	a[posk].arity = next_int(&s1);

	next_str(&s1, s2);
	a[posk].assoc_comm = str_ident(s2, "ac");
	a[posk].commutative = str_ident(s2, "comm");

	next_str(&s1, s2);
	strcpy(a[posk].name, s2);

	a[posk].lrpo_status = next_int(&s1);
	a[posk].lex_val = next_int(&s1);
	a[posk].skolem = next_int(&s1);
	}
#if 0
    for (j=0; j<a_size; j++)
	if (a[j].sym_num != 0)
	    fprintf(Peer_fp, "Entry in sym_array:%d %d %s %d %d %d\n", a[j].sym_num, a[j].arity, a[j].name,
		   a[j].lrpo_status, a[j].lex_val, a[j].skolem);
#endif    
}  /* string_to_sym_array */

/*************
 *
 *     term_to_string_rec(t, s, size, ip)
 *
 *************/

void term_to_string_rec(Term_ptr t, char *s, int size, int *ip)
{
    char s1[MAX_NAME];
    struct rel *r;
    int i;

    if (VARIABLE(t)) {
	sprintf(s1, "v%d ", t->symbol);
	*ip += strlen(s1);
	if (*ip >= size)
	    abend("term_to_string_rec, string too small.");
	strcat(s, s1);
	}
    else {
	sprintf(s1, "%d ", t->symbol);
	*ip += strlen(s1);
	if (*ip >= size)
	    abend("term_to_string_rec, string too small.");
	strcat(s, s1);
	for (i = 0; i < t->arity; i++)
	     term_to_string_rec(t->args[i], s, size, ip);
	}
}  /* term_to_string_rec */

/*************
 *
 *     term_to_string(t, s, size)
 *
 *************/

void term_to_string(Term_ptr t, char *s, int size)
{
    int i;
    
    s[0] = '\0';
    i = 0;
    term_to_string_rec(t, s, size, &i);
}  /* term_to_string */

/*************
 *
 *     string_to_term_rec()
 *
 *************/

Term_ptr string_to_term_rec(char **sp)
{
    Term_ptr t;
    char s1[MAX_NAME];
    int arity, j, n, posn;

    next_str(sp, s1);
    if (s1[0] == 'v') {
	n = atoi(s1+1);
	t = get_term(0);
	t->symbol = n;
	t->arity = 0;
	}
    else {
	n = atoi(s1);	
	posn = abs(n) % MAX_SYMBOL_ARRAY;
	/* Peers_mcd, November 1996: sym_num's are negative in EQP0.9 and   */
	/* therefore Peers_mcd stores in Symbol_array[i] the sym_ent struct */
	/* for sym_num -i, where 0<i<MAX_SYMBOL_ARRAY.			    */
	arity = Symbol_array[posn].arity;
	t = get_term(arity);
	t->symbol = n;
	t->arity = arity;
	for (j=0; j<arity; j++)	    
	     t->args[j] = string_to_term_rec(sp);
	}
    return(t);
}  /* string_to_term_rec */

/*************
 *
 *     string_to_term()
 *
 *************/

struct term *string_to_term(char *s)
{
    char *s1;

    s1 = s;
    return(string_to_term_rec(&s1));

}  /* string_to_term */

/*************
 *
 *    clause_to_string(c, s, size)
 *
 *    The string form of a clause (all integers separated by space, with
 *    a space at the end) is
 *
 *    "id weight interpreted_value <length of justification> <justification> <literal> "
 *    where
 *    <literal> is "sign type <atom>" and
 *    <justification> is a list of integers, hence "int int ... int "
 *
 *    This is for unit clauses only.
 *
 *************/

void clause_to_string(Clause_ptr c, char *s, int size)
{
    char s1[MAX_DAC_STRING], s2[MAX_NAME];
    Gen_ptr_ptr p;
    int i;
    
    for (p=c->justification, i=0; p; p=p->next, i++);
    
    /* sprintf clears the string of possible previous contents */
    sprintf(s, "%d %d %d %d ",
	    c->id,
	    c->weight,
	    c->interpreted_value,
	    i);
     
    for (p=c->justification; p; p=p->next) {
    sprintf(s2, "%d ", p->u.i);
    strcat(s, s2);
	}
	
    sprintf(s2, "%d %d ", c->literals->sign, c->literals->type);
    strcat(s, s2);
	
    term_to_string(c->literals->atom, s1, MAX_DAC_STRING);
        
    if (strlen(s1) + strlen(s) >= size)
 	abend("clause_to_string, string too small");

    strcat(s, s1);
       
}  /* clause_to_string */

/*************
 *
 *     string_to_clause()
 *
 *    "id weight interpreted_value <length of justification> <justification> <literal> "
 *    where <justification> is a list of integers, hence "int int ... int " and
 *    <literal> is "sign type <atom>"
 *
 *************/

Clause_ptr string_to_clause(char *s)
{
    Clause_ptr c;
    Literal_ptr l;
    int n, i, j;
    Gen_ptr_ptr p1, p2;

    c = get_clause();
    
    c->id = next_int(&s);
    n = next_int(&s);
    c->weight = n;
    n = next_int(&s);
    c->interpreted_value = n;
    j = next_int(&s);	/* length of justification */
    for (i=0, p2=NULL; i<j; i++) {
	n = next_int(&s);
	p1 = get_gen_ptr();
	if (p2)
	    p2->next = p1;
	else
	    c->justification = p1;
	p1->u.i = n;
	p2 = p1;
	}

    l = get_literal();
    
    l->sign = next_int(&s);
    l->type = next_int(&s);
    l->atom = string_to_term(s);
    c->literals = l;
    return(c);
    
}  /* string_to_clause */

