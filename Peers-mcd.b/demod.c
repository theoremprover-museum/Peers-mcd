#include "Header.h"
#include "Symbols.h"
#include "List.h"
#include "Io.h"
#include "Unify.h"
#include "Discrim.h"
#include "Clause.h"
#include "Ac.h"
#include "Demod.h"

/*************
 *
 *     clear_demod_marks(t)
 *
 *************/

void clear_demod_marks(Term_ptr t)
{
    int i;
    clear_term_scratch(t);
    for (i = 0; i < t->arity; i++)
	clear_demod_marks(t->args[i]);
}  /* clear_demod_marks */

/*************
 *
 *    Term_ptr apply_demod(term, context) -- Apply a substitution.
 *
 *    Apply always succeeds and returns a pointer to the
 *    instantiated term.  If any subterms or terms in the context
 *    have the scratch bit marked, mark the scratch bit of the
 *    corresponding terms in the result.  (The scratch mark means
 *    that the term is fully demodulated.)  Otherwise, this is similar
 *    to ordinary apply.
 *    
 *************/

static Term_ptr apply_demod(Term_ptr t, Context_ptr c)
{
    Term_ptr t2;

    DEREFERENCE(t, c)

    /* A NULL context means that the subst was generated by match.
     * If the context is NULL, then apply_demod just copies the term.
     */
    
    t2 = get_term(t->arity);

    if (term_scratch(t))
	set_term_scratch(t2);

    if (VARIABLE(t)) {
	if (!c)
	    t2->symbol = t->symbol;
	else
	    t2->symbol = c->multiplier * MAX_VARS + t->symbol;
	}
    else {
	int i;
	t2->symbol = t->symbol;
	for (i = 0; i < t->arity; i++)
	    t2->args[i] = apply_demod(t->args[i], c);
	}
    return(t2);
}  /* apply_demod */

/*************
 *
 *    demodulate 
 *
 *    For non-AC terms.
 *
 *************/

Term_ptr demodulate(Term_ptr t, Discrim_ptr demods, Gen_ptr_ptr *head_ptr)
{
    if (term_scratch(t) || VARIABLE(t))
	;  /* Do nothing, because t cannot be rewritten. */
    else {
	Term_ptr contractum, atom;
	int i;
	Discrim_pos_ptr pos;
	Clause_ptr demodulator;
	Context_ptr c;

	for (i = 0; i < t->arity; i++)
	    t->args[i] = demodulate(t->args[i], demods, head_ptr);

	c = get_context();

	Stats[REWRITE_ATTEMPTS]++;

        demodulator = discrim_retrieve_first(t, demods, c, &pos);
	
	if (demodulator) {
	    Stats[REWRITES]++;
	    atom = demodulator->literals->atom;
	    contractum = apply_demod(atom->args[1], c);
	    discrim_cancel(pos);

	    zap_term(t);
	    if (Flags[DEMOD_HISTORY].val) {
		Gen_ptr_ptr p;
		p = get_gen_ptr();
		p->u.i = demodulator->id;
		p->next = *head_ptr;
		*head_ptr = p;
		}
	    t = demodulate(contractum, demods, head_ptr);
	    }

	free_context(c);
	}
    set_term_scratch(t);  /* Mark as fully demodulated. */
    return(t);
}  /* demodulate */

/*************
 *
 *    contract_bt
 *
 *************/

static Term_ptr contract_bt(Term_ptr t, void *demods,
			    Clause_ptr *demodulator_ptr)
{
    List_pos_ptr tp;
    Discrim_pos_ptr pos;
    struct bt_node *bt;
    Term_ptr contractum, partial, atom;
    Clause_ptr demodulator;
    Context_ptr c;

#if 0
    printf("contract_bt receives "); p_term(t); printf("\n"); fflush(stdout);
#endif    
    c = get_context();
    contractum = NULL;

    if (Flags[INDEX_BT_DEMOD].val)
	demodulator = discrim_wild_retrieve_first(t, demods, &pos);
    else {
	tp = ((List_ptr) demods)->first;
	demodulator = (tp ? tp->c : NULL);
	}
	
    Stats[REWRITE_ATTEMPTS]++;
    while (demodulator && !contractum) {
	atom = demodulator->literals->atom;
	bt = match_bt_first(atom->args[0], c, t, 1);
	if (bt) {
	    Stats[REWRITES]++;
	    contractum = apply_demod(atom->args[1], c);
	    if (c->partial_term) {
		/* Get copy, including marks that indicate normal terms. */
		partial = apply_demod(c->partial_term, (Context_ptr) NULL);
		contractum = build_binary_term(t->symbol, contractum, partial);
		}
	    match_bt_cancel(bt);
	    if (Flags[INDEX_BT_DEMOD].val)
		discrim_wild_cancel(pos);
	    *demodulator_ptr = demodulator;
	    }
	else {
	    if (Flags[INDEX_BT_DEMOD].val)
		demodulator =  discrim_wild_retrieve_next(pos);
	    else {
		tp = tp->next;
		demodulator = (tp ? tp->c : NULL);
		}
	    }
	}
    free_context(c);
#if 0
    if (contractum) {
	printf("count=%d, t: ",count); p_term(t);
	printf("demodulator: "); p_term(atom);
	printf("contractum: "); p_term(contractum);
	printf("\n");
	fflush(stdout);
	}
#endif    
    return(contractum);
}  /* contract_bt */

/*************
 *
 *    demodulate_bt
 *
 *************/

Term_ptr demodulate_bt(Term_ptr t, void *demods, int psn, Gen_ptr_ptr *head_ptr)

{
    Term_ptr contractum;
    int sn;
    Gen_ptr_ptr p;
    Clause_ptr demodulator;
    int i;

    /* If already fully demodulated, or variable, do nothing. */

    if (term_scratch(t) || VARIABLE(t))
	;
    else {
	sn = t->symbol;
	for (i = 0; i < t->arity; i++)
	    t->args[i] = demodulate_bt(t->args[i], demods, sn, head_ptr);

	if (sn != psn || !is_assoc_comm(sn)) {  /* If not part of an AC term */
	    ac_canonical(t);
  	    contractum = contract_bt(t, demods, &demodulator);
	    if (contractum) {
		zap_term(t);
		ac_canonical(contractum);
		if (Flags[DEMOD_HISTORY].val) {
		    p = get_gen_ptr();
		    p->u.i = demodulator->id;
		    p->next = *head_ptr;
		    *head_ptr = p;
		    }
		t = demodulate_bt(contractum, demods, psn, head_ptr);
		}
	    }
	}
    /* Mark as fully demodulated.  This also means ac_canonical. */
    set_term_scratch(t);
    return(t);
}  /* demodulate_bt */

/*************
 *
 *    simplifiable_top_bt
 *
 *************/

static Clause_ptr simplifiable_top_bt(Term_ptr t, void *demods)
				      
{
    List_pos_ptr tp;
    Discrim_pos_ptr pos;
    struct bt_node *bt;
    Term_ptr atom;
    Clause_ptr demodulator;
    Context_ptr c;
    int rc;

    c = get_context();
    rc = 0;

    if (Flags[INDEX_BT_DEMOD].val)
	demodulator = discrim_wild_retrieve_first(t, demods, &pos);
    else {
	tp = ((List_ptr) demods)->first;
	demodulator = (tp ? tp->c : NULL);
	}
	
    while (demodulator && !rc) {
	atom = demodulator->literals->atom;
	bt = match_bt_first(atom->args[0], c, t, 1);
	if (bt) {
	    rc = 1;
	    match_bt_cancel(bt);
	    if (Flags[INDEX_BT_DEMOD].val)
		discrim_wild_cancel(pos);
	    }
	else {
	    if (Flags[INDEX_BT_DEMOD].val)
		demodulator =  discrim_wild_retrieve_next(pos);
	    else {
		tp = tp->next;
		demodulator = (tp ? tp->c : NULL);
		}
	    }
	}
    free_context(c);
    return(rc ? demodulator : NULL);
}  /* simplifiable_top_bt */

/*************
 *
 *    simplifiable_bt
 *
 *************/

int simplifiable_bt(Term_ptr t, int psn, void *demods)
{
    int i, sn;
    Clause_ptr demodulator;

    sn = t->symbol;
    for (i = 0; i < t->arity; i++) {
	if (simplifiable_bt(t->args[i], sn, demods))
	    return(1);
	else {
	    if (!(sn == psn && is_assoc_comm(sn))) {  /* not part of AC term */
		demodulator = simplifiable_top_bt(t, demods);
		if (demodulator)
		    return(1);
		}
	    }
	}
    return(0);
}  /* simplifiable_bt */

/*************
 *
 *    simplifiable -- for non-AC, discrim-indexed terms
 *
 *************/

int simplifiable(Term_ptr t, Discrim_ptr demods)
{
    if (VARIABLE(t))
	return(0);
    else {
	int i;
	Discrim_pos_ptr pos;
	Clause_ptr demodulator;
	Context_ptr c;

	for (i = 0; i < t->arity; i++) {
	    if (simplifiable(t->args[i], demods))
		return(1);
	    }

	c = get_context();
        demodulator = discrim_retrieve_first(t, demods, c, &pos);
	if (demodulator)
	    discrim_cancel(pos);
	free_context(c);

	return(demodulator != NULL);
	}
}  /* simplifiable */

