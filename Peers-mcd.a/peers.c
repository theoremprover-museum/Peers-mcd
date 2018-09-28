/* MODIFIED CLAUSE DIFFUSION theorem prover */

#include "Header.h"
#include "Io.h"
#include "Unify.h"
#include "Index.h"
#include "Paramod.h"
#define OWNER_OF_PEER_GLOBALS
#include "Peers.h"
#include "p4.h"

/*************
 *
 *    peer_init_and_work
 *
 *************/

void peer_init_and_work()
{
    int i;
    struct list_pos *p, *q;
    struct literal *c;
    int i0, i1;
    struct list *usable, *sos, *passive, *demodulators;
	/* temp lists for the input phase */

    init();  /* Init stats, clocks, sym_table, options and special_ops */
    
    for (i = 0; i < MAX_PROCESSES; i++)
    	Peers_assigned_clauses[i] = Peers_work_load[i] = 0;
    	
    Msgs_diff = 0;
    /* Difference between number of sent messages (globally) and number */
    /* of received messages (globally).					*/
    Have_token = 1;
    /* Peer 0 has the token at the beginning. */
    How_many_times_token_received = 0;

    Deleted_clauses = get_list();
    Demod_index = get_discrim();
    init_clause_table_id(Id_table);
    init_clause_table_id(Already_broadcast_table);

    Pid = 0;		/* this code is executed only by peer0 */

    read_preamble();  /* read options */
    
    /* Read input clauses into temporary lists. */

    usable = get_list();
    sos = get_list();
    demodulators = get_list();
    passive = get_list();

    read_lists(stdin, usable, sos, passive, demodulators);

    if (Stats[INPUT_ERRORS] != 0) {
	fprintf(stderr, "\nInput errors were found.\007\n\n");
	printf("Input errors were found.\n");
	return;
	}

    /* Create all other peers. */

    i0 = p4_clock();
    p4_create_procgroup();
    i1 = p4_clock();
    Number_of_peers = p4_num_total_slaves() + 1;

    printf("\nPeer0 created %d additional peers in %.2f seconds.\n",
	   Number_of_peers-1, (i1-i0)/1000.);
    printf("run_time=%.2f, system_time=%.2f.\n\n",
	   run_time()/1000., system_time()/1000.);
    fprintf(stderr, "Procgroup created.\007\n");

    /* Process input clauses and decide their owners. */

    set_clauses(usable, sos, passive, demodulators);
    
    /* Broadcast a new set of options to the peers. */

    options_to_string(Work_str, MAX_DAC_STRING);
    m4_broadcast(OPTIONS_MSG, Work_str, strlen(Work_str)+1);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
/* Message broadcast: it is like sending Number_of_peers - 1 messages. */
    
    /* Broadcast a new symbol table to the peers. */

    sym_tab_to_string(Work_str, MAX_DAC_STRING);
    string_to_sym_array(Work_str, Symbol_array, MAX_SYMBOL_ARRAY);
    m4_broadcast(SYM_TAB_MSG, Work_str, strlen(Work_str)+1);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
    
    /* Broadcast the input clauses to the peers. */

    for (p = Sos->first; p; p = p->next) {
	c = p->lit;
	clause_to_string(c, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_SOS_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
	store_clause_by_id(c, Already_broadcast_table);
	}
    for (p = Usable->first; p; p = p->next) {
	c = p->lit;
	clause_to_string(c, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_USABLE_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
	store_clause_by_id(c, Already_broadcast_table);
	}

    /* Must broadcast demods after usable and after sos.	*/
    /* This is to handle the case where an equation c in	*/
    /* Sos or Usable has been turned into a demod.		*/
    /* By broadcasting Sos and Usable first, we make sure	*/
    /* that c will be received first and thus stored by the	*/
    /* other peers with the id set for its occurrence		*/
    /* in Sos or Usable.					*/ 

    for (p = Demodulators->first; p; p = p->next) {
	clause_to_string(p->lit, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_DEMOD_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
	}
    for (p = Passive->first; p; p = p->next) {
	clause_to_string(p->lit, Work_str, MAX_DAC_STRING);
	m4_broadcast(INPUT_PASSIVE_MSG, Work_str, strlen(Work_str)+1);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
	}
    
    if (Flags[PARA_PAIRS].val) {
        for (p = Sos->first; p; p = p->next) {
            list_append(p->lit, Usable);
            for (q = Usable->first; q; q = q->next) {
		/* make sure that at least one of the two equations in the pair is a resident */
		if (mine(p->lit) || mine(q->lit))
		    store_in_pp(p->lit, q->lit);
		}
            }
        list_zap(Sos);
	}
	
/* In Peers, Peer0 did this job, i.e. moving all Sos clauses to Usable and */
/* generating the pairs in the para_pairs structure at the end of set_clauses,*/
/* before broadcasting the input clauses. This creates trouble if either */
/* OWN_IN_USABLE or OWN_IN_SOS but not both are on, because the other peers */
/* receives as input Usable clauses what were originally input Sos clauses. */

    peer_work();

 /*   printf("peer_init_and_work exiting normally\n"); */

}  /* peer_init_and_work */

/*************
 *
 *    peer_work()
 *
 *************/

void peer_work()
{
    int i;
    char s1[MAX_NAME], hostname[64];

    /* Peer initialization. */

    Pid = p4_get_my_id();
    Number_of_peers = p4_num_total_slaves() + 1;

    if (gethostname(hostname, 64) != 0)
	str_copy("???", hostname);

    sprintf(s1,"peer%d", Pid);
    Peer_fp = fopen(s1, "w");
    fprintf(Peer_fp, "Peer %d (of %d), host %s, %s\n\n",
	    Pid, Number_of_peers, hostname, get_time());
    fflush(Peer_fp);

    if (Pid != 0) {
    
	init();
      
	for (i = 0; i < MAX_PROCESSES; i++)
    	Peers_assigned_clauses[i] = Peers_work_load[i] = 0;
 
	Msgs_diff = 0;
	Have_token = 0;
	How_many_times_token_received = 0;

	Usable = get_list();
	Sos = get_list();
	Passive = get_list();
	Demodulators = get_list();
	Deleted_clauses = get_list();
	Demod_index = get_discrim();
	init_clause_table_id(Id_table);
	init_clause_table_id(Already_broadcast_table);
	}
    
    work_loop();

#if 0
    print_clause_table_id(Peer_fp, Id_table);
#endif

    if (Flags[PRINT_LISTS_AT_END].val) {
	fprintf(Peer_fp, "\nUsable:\n");
	print_list(Peer_fp, Usable);
	fprintf(Peer_fp, "\nSos:\n");
	print_list(Peer_fp, Sos);
	fprintf(Peer_fp, "\nDemodulators:\n");
	print_list(Peer_fp, Demodulators);
	}
	
    print_stats(Peer_fp);
    print_times(Peer_fp);
    fprintf(Peer_fp, "\nPeer %d finishes work_loop.\n", Pid);
    fprintf(Peer_fp, "THE END\n");
    fflush(Peer_fp);

}  /* peer_work */

/*************
 *
 *   work_available(type)
 *
 *   This routine determines if there is work of a given type available.
 *   `Work' is either an incoming message to process or something the
 *   process decides to do for itself.
 *
 *************/

int work_available(type)
int type;
{
    switch (type) {
      case HALT_MSG:
      case OPTIONS_MSG: 
      case SYM_TAB_MSG:    
      case INPUT_SOS_MSG:
      case INPUT_PASSIVE_MSG:
      case INPUT_DEMOD_MSG:
      case INPUT_USABLE_MSG:
      case INFERENCE_MSG:
      case TOKEN_MSG:
      	return(m4_messages_available(type));
      case INFERENCE_WORK:
	return(expansion_work_available());
      default:
	fprintf(Peer_fp, "work_available, bad type=%d.\n", type);
	abend("work_available, bad work type");
	return(0);  /* to quiet lint */
	}
}  /* work_available */

/*************
 *
 *   find_high_priority_work()
 *
 *   This process finds the type of the highest-priority work that is
 *   available.  If no work is available, it returns -1 (which usually
 *   means that the process should wait for a message).
 *
 *   The priorities are specified with #define statements, with the n types
 *   assigned 1,2,...,n  (1 is highest priority).  n is NUMBER_OF_WORK_TYPES.
 *
 *************/

int find_high_priority_work()
{
    int i;
    for (i=1; i<=NUMBER_OF_WORK_TYPES; i++)
	if (work_available(i))
	    break;
    return(i>NUMBER_OF_WORK_TYPES ? -1 : i);
}  /* find_high_priority_work */

/*************
 *
 *   work_loop()
 *
 *************/

void work_loop()
{
    int go, work_type, from, size, to;
    char *incoming, *s;
    struct literal *lit;

    go = 1;
    while (go) {
	work_type = find_high_priority_work();
	switch (work_type) {

	    /* cases for `message' work */

	  case -1:  /* No messages available.  Wait and receive any message. */

	    if (Number_of_peers == 1) {
		fprintf(Peer_fp, "Peer 0 stops, because it's idle, and there are no other peers.\n");
		fflush(Peer_fp);
		go = 0;
		break;
		}
	    else if (Have_token) {
	             if (Pid == 0 && Msgs_diff == 0 && How_many_times_token_received >= 2) {
		     fprintf(Peer_fp, "\nPeer %d stops, because no one is busy.\n", Pid);
		     fflush(Peer_fp);
		     m4_broadcast(HALT_MSG, "", 0);
		     go = 0;
		     break;
		     }
		     else {
	    		to = (Pid + 1) % Number_of_peers;
    			sprintf(Work_str, "%d ", Msgs_diff);
	    		p4_send(TOKEN_MSG, to, Work_str, strlen(Work_str)+1);
			Msgs_diff = 0;
/* It resets to 0 the local counter, having sent its value in the token. */
			Have_token = 0;
		        fprintf(Peer_fp, "\nPeer %d sends token message to Peer %d.\n", Pid, to);
		        fflush(Peer_fp);
			  }
	           }
	  case HALT_MSG:        
	  case OPTIONS_MSG: 
	  case SYM_TAB_MSG:    
	  case INPUT_SOS_MSG:
	  case INPUT_PASSIVE_MSG:
	  case INPUT_DEMOD_MSG:
	  case INPUT_USABLE_MSG:	  
	  case INFERENCE_MSG:
	  case TOKEN_MSG:

	 /* This part handles all message types, including wait-for-messages */

	    from = -1;
	    incoming = NULL;
	    p4_recv(&work_type, &from, &incoming, &size);

	    switch (work_type) {

	      case HALT_MSG:
		go = 0;
		fprintf(Peer_fp, "\nPeer %d receives halt message in work_loop from Peer %d.\n", Pid, from);
		fflush(Peer_fp);
		break;

	      case TOKEN_MSG:
		Have_token = 1;
		How_many_times_token_received++;
		s = incoming;
		Msgs_diff = Msgs_diff + next_int(&s);
		fprintf(Peer_fp, "Peer %d gets token message from Peer %d.\n", Pid, from);
		fflush(Peer_fp);
		break;

	      case SYM_TAB_MSG:
	        Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
		string_to_sym_array(incoming, Symbol_array, MAX_SYMBOL_ARRAY);
		init_symbol_table();
		sym_array_to_normal_sym_tab(Symbol_array, MAX_SYMBOL_ARRAY);
		fprintf(Peer_fp, "Peer %d gets new symbols.\n", Pid);
		fflush(Peer_fp);
		break;

	      case OPTIONS_MSG:
	        Msgs_diff--;
		string_to_options(incoming);
		fprintf(Peer_fp, "Peer %d gets new options.\n", Pid);
		fflush(Peer_fp);
		break;

	      case INPUT_PASSIVE_MSG:
	        Msgs_diff--;
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input passive: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		list_append(lit, Passive);
		store_clause_by_id(lit, Id_table);
		Stats[VISITORS]++;
		break;

	      case INPUT_DEMOD_MSG:
	        Msgs_diff--;
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input demod: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		list_append(lit, Demodulators);
		discrim_wild_insert(lit->atom->farg->argval, Demod_index, (void *) lit);
		if (Flags[TWO_WAY_DEMOD].val)
		    discrim_wild_insert(lit->atom->farg->narg->argval, Demod_index, (void *) lit);
		if (!find_clause_by_id(lit->id, Id_table))
		    store_clause_by_id(lit, Id_table);
		break;

	      case INPUT_USABLE_MSG:
	        Msgs_diff--;
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input usable: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		if (Flags[OWN_IN_USABLE].val)
		    lit->id = encode_id(Pid, Pid, ++(Peers_assigned_clauses[Pid]));
		/* If OWN_IN_USABLE is on, it is like each peer has read   */
		/* the input Usable clauses itself, so that both owner and */
		/* and birth_place of the clause are set to Pid.           */
		list_append(lit, Usable);
		store_clause_by_id(lit, Id_table);
		store_clause_by_id(lit, Already_broadcast_table);
		if (Flags[PARA_PAIRS].val) {
		    struct list_pos *p;
		    for (p = Usable->first; p; p = p->next) {
			/* make sure that at least one of the two clauses (equations) in the pair is a resident */
			if (mine(lit) || mine(p->lit))
			    store_in_pp(lit, p->lit);
			}
		    }
		if (mine(lit))
		    Stats[RESIDENTS]++;
		else Stats[VISITORS]++;
		break;

	      case INPUT_SOS_MSG:
	        Msgs_diff--;
		lit = string_to_clause(incoming);
		fprintf(Peer_fp, "Peer %d gets input sos: ", Pid);
		print_clause(Peer_fp, lit);
		fflush(Peer_fp);
		if (Flags[OWN_IN_SOS].val || (!lit->sign && Flags[OWN_GOALS].val))
		    lit->id = encode_id(Pid, Pid, ++(Peers_assigned_clauses[Pid]));
/* If OWN_IN_SOS is on, or OWN_GOALS is on and the clause is negated,         */
/* the receiving peer Pid assigns the clause to itself by setting its owner   */
/* field to Pid. The birth_place field is also set to Pid and the num field   */
/* is set accordingly. Remark that we could not keep the birth_place 0 to say */
/* that the clause was read by Peer0, because Pid does not know the value of  */
/* Peers_assigned_clauses[Pid] at Peer0.				      */
		store_new_clause(lit);
		store_clause_by_id(lit, Id_table);
		store_clause_by_id(lit, Already_broadcast_table);
		if (mine(lit))
		    Stats[RESIDENTS]++;
		else Stats[VISITORS]++; 
		break;

	      case INFERENCE_MSG:
	        Msgs_diff--;
		Stats[IM_RECEIVED]++;
		CLOCK_START(INF_MSG_TIME);
		handle_inference_msg(incoming,from);
		CLOCK_STOP(INF_MSG_TIME);
		break;
	
		}
	    p4_msg_free(incoming);
	    break;

            /* end of message processing */
	       
	    /* case for local work */

	  case INFERENCE_WORK:
		CLOCK_START(INFERENCE_TIME);
		if (Flags[PARA_PAIRS].val)
		    expansion_work_pairs();
		else
		    expansion_work_given();
		CLOCK_STOP(INFERENCE_TIME);
	    break;

            /* end of local work */

	    }
	}
}  /* work_loop */

/*************
 *
 *    forward_subsume(lit)
 *
 *************/

int forward_subsume(lit)
struct literal *lit;
{
    struct term *t1, *t2;
    struct literal *subsumer;

    if (lit->sign && is_symbol(lit->atom, "=", 2)) {
	t1 = lit->atom->farg->argval;
	t2 = lit->atom->farg->narg->argval;
	if (term_ident(t1,t2))
	    return(1);	/* subsumed by x=x */
	}

    subsumer = subsume_list(lit, Passive);
    if (!subsumer)
	subsumer = subsume_list(lit, Usable);
    if (!subsumer)
	subsumer = subsume_list(lit, Sos);

    return(subsumer != NULL);

}  /* forward_subsume */

/*************
 *
 *    check_for_proof(lit)
 *
 *************/

void check_for_proof(lit)
struct literal *lit;
{
    struct term *t1, *t2;
    struct context *c1, *c2;
    struct bt_node *bt_position;
    struct literal *conflictor;
    int xx_proof;

    xx_proof = 0;

    if (!lit->sign && is_symbol(lit->atom, "=", 2)) {
        c1 = get_context();
        c2 = get_context();
	t1 = lit->atom->farg->argval;
	t2 = lit->atom->farg->narg->argval;
	bt_position = unify_bt_first(t1, c1, t2, c1);
	if (bt_position) {
	    xx_proof = 1;
	    unify_bt_cancel(bt_position);
	    }
	free_context(c1);
	free_context(c2);
	}

    if (xx_proof)
	conflictor = NULL;
    else {
	conflictor = conflict_list(lit, Passive);
	if (!conflictor)
	    conflictor = conflict_list(lit, Usable);
	if (!conflictor)
	    conflictor = conflict_list(lit, Sos);
	}

    if (xx_proof || conflictor) {
	
	fprintf(stderr, "---------------- PROOF ----------------");
	fprintf(stderr, "\007\n\n");
	if (conflictor)
	    fprintf(Peer_fp, "\nUNIT CONFLICT from <%d,%d,%d> and <%d,%d,%d>",
		   owner(lit->id), birth_place(lit->id), num(lit->id),
		   owner(conflictor->id), birth_place(conflictor->id), num(conflictor->id));
	else
	    fprintf(Peer_fp, "\nUNIT CONFLICT from <%d,%d,%d> and x=x",
		   owner(lit->id), birth_place(lit->id), num(lit->id));

	fprintf(Peer_fp, " at %6.2f seconds.\n", run_time() / 1000.);

	Stats[PROOFS]++;

        print_proof(Peer_fp, lit, conflictor);
	fflush(Peer_fp);

	if (Stats[PROOFS] >= Parms[MAX_PROOFS].val) {
#if 0
	    print_clause_table_id(Peer_fp, Id_table);
#endif	    
	    m4_broadcast(HALT_MSG, "", 0);
	    if (Flags[PRINT_LISTS_AT_END].val) {
		fprintf(Peer_fp, "\nUsable:\n");
		print_list(Peer_fp, Usable);
		fprintf(Peer_fp, "\nSos:\n");
		print_list(Peer_fp, Sos);
		fprintf(Peer_fp, "\nDemodulators:\n");
		print_list(Peer_fp, Demodulators);
		}
	    print_stats(Peer_fp);
	    print_times(Peer_fp);
	    fprintf(Peer_fp, "\nPeer %d found a proof.\n", Pid);
	    fprintf(Peer_fp, "THE END\n");
	    fflush(Peer_fp);
	    p4_wait_for_end();
	    exit(PROOF_EXIT);
	    }
	}
}  /* check_for_proof */

/*************
 *
 *    term_list_to_literal_list(p, l)
 *
 *************/

void term_list_to_literal_list(p, l)
struct gen_ptr *p;
struct list *l;
{
    struct gen_ptr *p1;
    struct term *t;
    struct literal *lit;

    while (p) {
	t = p->u.t;
	lit = get_literal();
	lit->sign = !is_symbol(t, "-", 1);
	if (!lit->sign) {
	    lit->atom = t->farg->argval;
	    free_rel(t->farg);
	    free_term(t);
	    }
	else
	    lit->atom = t;
	lit->weight = -1;
	list_append(lit, l);
	p1 = p;
	p = p->next;
	free_gen_ptr(p1);
	}
}  /* term_list_to_literal_list */

/*************
 *
 *     read_lists(fp, usable, sos, passive, demodulators)
 *
 *************/

void read_lists(fp, usable, sos, passive, demodulators)
FILE *fp;
struct list *usable, *sos, *passive, *demodulators;
{
    struct term *t;
    int rc;
    struct gen_ptr *p;

    t = read_term(stdin, &rc);

    while (t || rc == 0) {
        if (rc == 0)
	    Stats[INPUT_ERRORS]++;
        else if (!is_symbol(t, "list", 1)) {
	    printf("bad list command: ");
	    p_term(t);
	    Stats[INPUT_ERRORS]++;
	    }
	else {
	    p = read_list(fp, &rc);
	    Stats[INPUT_ERRORS] += rc;
	    if (str_ident(sn_to_str(t->farg->argval->sym_num), "usable"))
		term_list_to_literal_list(p, usable);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "sos"))
		term_list_to_literal_list(p, sos);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "passive"))
		term_list_to_literal_list(p, passive);
	    else if (str_ident(sn_to_str(t->farg->argval->sym_num), "demodulators"))
		term_list_to_literal_list(p, demodulators);
	    else {
		printf("bad list command: ");
		p_term(t);
		Stats[INPUT_ERRORS]++;
		}
	    }
	t = read_term(stdin, &rc);
	}
	
    if (Stats[INPUT_ERRORS] == 0) {
	printf("\nUsable:\n");
	p_list(usable);
	printf("\nSos:\n");
	p_list(sos);
	printf("\nDemodulators:\n");
	p_list(demodulators);
	printf("\nPassive:\n");
	p_list(passive);
	}
}  /* read_lists */

/*************
 *
 *     set_clauses()
 *
 *************/

void set_clauses(usable, sos, passive, demodulators)
struct list *usable, *sos, *passive, *demodulators;
{
    struct list_pos *p, *q;
    struct literal *lit;
    
    Usable = get_list();
    Sos = get_list();
    Demodulators = get_list();
    Passive = passive;
    /* Passive list doesn't change, so just move the pointer. */

    for (p = Passive->first; p; p = p->next) {
	lit = p->lit;
	ac_canonical(lit->atom);
	lit->id = encode_id(0, 0, ++(Peers_assigned_clauses[0]));
        store_clause_by_id(lit, Id_table);
        Stats[RESIDENTS]++;
	}

	/* Input clauses in Passive belong to Peer0, by default.        */
	/* Since they are used only for forward subsumption and         */
	/* unit_conflict, ownership does not affect their usage.        */
	/* Same remark applies to input Demodulators below.             */
 
    p = usable->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	list_remove(lit, usable);
	process_input_clause(lit, Usable);
	}
    free_list(usable);

    p = sos->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	list_remove(lit, sos);
	process_input_clause(lit, Sos);
	}
    free_list(sos);

    p = demodulators->first;
    while(p) {
	q = p;
	p = p->next;
	lit = q->lit;
	ac_canonical(lit->atom);
	lit->id = encode_id(0, 0, ++(Peers_assigned_clauses[0]));
        store_clause_by_id(lit, Id_table);
        discrim_wild_insert(lit->atom->farg->argval, Demod_index, (void *) lit);
        if (Flags[TWO_WAY_DEMOD].val)
            discrim_wild_insert(lit->atom->farg->narg->argval, Demod_index, (void *) lit);
	list_remove(lit, demodulators);
	list_append(lit, Demodulators);
	}
    free_list(demodulators);

    printf("\nAfter processing input:\n");
    printf("\nUsable:\n");
    p_list(Usable);
    printf("\nSos:\n");
    p_list(Sos);
    printf("\nDemodulators:\n");
    p_list(Demodulators);
    printf("\nPassive:\n");
    p_list(Passive);
    
}  /* set_clauses */

/*************
 *
 *   decide_owner(lit, lst)
 *
 *
 *   lst   Sos    -- input Sos clause
 *         Usable -- input Usable clause
 *         NULL   -- not an input clause
 *   
 *
 *************/

int decide_owner(lit, lst)
struct literal *lit;
struct list *lst;
{
    static int next_choice = -1; 	/* used only by the ROTATE strategy */
    static int number_of_choices = 0;   /* used only by the AGO strategies */
    int cand;
    	    
    if (Number_of_peers == 1)
        return(Pid);
    else if (Flags[OWN_IN_SOS].val && lst == Sos)
	return(Pid);
    else if (Flags[OWN_IN_USABLE].val && lst == Usable)
	return(Pid);
    else if (Flags[OWN_GOALS].val && !lit->sign)
        return(Pid);	
    else {

	switch (Parms[DECIDE_OWNER_STRAT].val) {
	  case ROTATE:
	    {
	    if (next_choice == -1) /* The first choice in ROTATE is oneself. */
		next_choice = Pid;
	    cand = next_choice++ % Number_of_peers;
	    return(cand);
	    }
	  case SYNTAX:
	    return(clause_to_peer_function(lit));
	  case SELECT_MIN:
	    {
	    int j, min;
	    
	    min = MAX_INT;
	    for (j = 0; j < Number_of_peers; j++)
		if (Peers_work_load[j] < min) {
		    min = Peers_work_load[j];
		    cand = j;
		    }
	    return(cand);
	    }
	  case PARENTS:
	    {	    
	    cand = (lit->from_parent + lit->into_parent) % Number_of_peers;
	    return(cand);
	    }
	  case ALL_PARENTS:
	    {	    
	    cand = (lit->from_parent + lit->into_parent + lit->bd_parent) % Number_of_peers;
	    return(cand);
	    }
	  case HISTORY:
	    {
	    int j = 0;
	    struct gen_ptr *q;
	    
	    for (q = lit->demod_parents; q; q=q->next)
	         j = j + num(q->u.i);
	    cand = (num(lit->from_parent) + num(lit->into_parent) + num(lit->bd_parent) + j) % Number_of_peers;
	    return(cand);
	    }
	  case R_PARENTS:
	    {
	    if (lst == Sos || lst == Usable) /* input clause: ROTATE */
	       {	    
	       if (next_choice == -1)
		next_choice = Pid;
	       cand = next_choice++ % Number_of_peers;
	       }
	    else   /* not input clause: PARENTS */  
	    cand = (lit->from_parent + lit->into_parent) % Number_of_peers;
	    return(cand);
	    }
	  case R_ALL_PARENTS:
	    {
	    if (lst == Sos || lst == Usable) /* input clause: ROTATE */
	       {	    
	       if (next_choice == -1)
		next_choice = Pid;
	       cand = next_choice++ % Number_of_peers;
	       }
	    else   /* not input clause: ALL_PARENTS */  
	    cand = (lit->from_parent + lit->into_parent + lit->bd_parent) % Number_of_peers;
	    return(cand);
	    }
	  case R_HISTORY:
	    {
	    int j = 0;
	    struct gen_ptr *q;
	    	    
	    if (lst == Sos || lst == Usable) /* input clause: ROTATE */
	       {	    
	       if (next_choice == -1)
		next_choice = Pid;
	       cand = next_choice++ % Number_of_peers;
	       }
	    else {  /* not input clause: HISTORY */  
	    for (q = lit->demod_parents; q; q=q->next)
	         j = j + num(q->u.i);
	    cand = (num(lit->from_parent) + num(lit->into_parent) + num(lit->bd_parent) + j) % Number_of_peers;
	         }
	    return(cand);
	    }
	   case R_AGOP:
	    {
	    int j, max, votes[MAX_PROCESSES];
	    	    
	    if (Parms[SWITCH_OWNER_STRAT].val < MAX_INT && 
	        number_of_choices > Parms[SWITCH_OWNER_STRAT].val)
	       {	/* AGOP */	    
	       for (j = 0; j < MAX_PROCESSES; j++)
    		       votes[j] = 0;
    		  votes[owner(lit->from_parent)]++;
    		  votes[owner(lit->into_parent)]++;
    		  votes[birth_place(lit->from_parent)]++;
    		  votes[birth_place(lit->into_parent)]++;
    		  max = 0;
	          for (j = 0; j < Number_of_peers; j++)
		       if (votes[j] > max) {
		           max = votes[j];
		           cand = j;
		           }
	       }
	    else {  	/* ROTATE */
		 if (next_choice == -1)
		     next_choice = Pid;
	         cand = next_choice++ % Number_of_peers; 
	         }
	    number_of_choices++;
	    return(cand);
	    }
	    case P_AGOA:
	    {
	    int j, max, votes[MAX_PROCESSES];
	    	    
	    if (Parms[SWITCH_OWNER_STRAT].val < MAX_INT && 
	        number_of_choices > Parms[SWITCH_OWNER_STRAT].val)
	       {	/* AGOA */	    
	       for (j = 0; j < MAX_PROCESSES; j++)
    		       votes[j] = 0;
    		  votes[owner(lit->from_parent)]++;
    		  votes[owner(lit->into_parent)]++;
    		  votes[owner(lit->bd_parent)]++;
    		  max = 0;
	          for (j = 0; j < Number_of_peers; j++)
		       if (votes[j] > max) {
		           max = votes[j];
		           cand = j;
		           }
	       }
	    else  	/* PARENTS */
	    	 cand = (lit->from_parent + lit->into_parent) % Number_of_peers;
	    number_of_choices++;
	    return(cand);
	    }
	  default:
	    abend("decide_owner, bad decide_owner_strategy");
	    return(0);  /* to quiet lint */
	    }
	}
}  /* decide_owner */

/*************
 *
 *    process_input_clause(lit, lst)
 *
 *************/

void process_input_clause(lit, lst)
struct literal *lit;
struct list *lst;
{
    int oriented, owner;
    
    printf("Begin process input clause: "); p_clause(lit);
    ac_canonical(lit->atom);
    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (forward_subsume(lit)) {
        printf("Subsumed on input: "); p_clause(lit);
        Stats[CLAUSES_FORWARD_SUBSUMED]++;
        zap_lit(lit);
        }
    else {
        renumber_vars(lit);  /* This can make it non-ac-canonical. */
        ac_canonical(lit->atom);

        owner = decide_owner(lit, lst);

        lit->id = encode_id(owner, 0, ++(Peers_assigned_clauses[owner]));
        /* birth place of input clause is Peer0 */
        store_clause_by_id(lit, Id_table);

	/* If PARA_PAIRS, handle para_pair storing after broadcasting input clauses. */

	if (mine(lit))
	    Stats[RESIDENTS]++;
	else
	    Stats[VISITORS]++;
		
	if (lst == Sos) {
	    insert_literal_by_weight(lit, Sos);
	    if (mine(lit))
		Stats[RESIDENTS_IN_SOS]++;
	    else
		Stats[VISITORS_IN_SOS]++;
	    }
	else
	    list_append(lit, Usable);

        check_for_proof(lit);

	printf("End process input clause: "); p_clause(lit);
	fflush(stdout);
	
        if ((pos_eq(lit) && !Flags[CHECK_INPUT_DEMODULATORS].val) ||
	    (oriented && oriented_eq_can_be_demod(lit)))
	    new_demodulator_input(lit);
        }
}  /* process_input_clause */

/*************
 *
 *    process_new_clause(lit)
 *
 *************/

void process_new_clause(lit)
struct literal *lit;     
{
    int oriented, owner, from, type;

    Stats[CLAUSES_GENERATED]++;
    if (Flags[PRINT_GEN].val) {
        printf("Processing #%d: ", Stats[CLAUSES_GENERATED]);
        p_clause(lit);
        }
    
    /* This should be done often, and this seems like a good place. */
    from = -1;
    type = HALT_MSG;
    if (p4_messages_available(&type,&from)) {
#if 0	
	print_clause_table_id(Peer_fp, Id_table);
#endif	
	fprintf(Peer_fp, "\nPeer %d receives halt message in process_new_clause from Peer %d.\n", Pid, from);
	if (Flags[PRINT_LISTS_AT_END].val) {
	    fprintf(Peer_fp, "\nUsable:\n");
	    print_list(Peer_fp, Usable);
	    fprintf(Peer_fp, "\nSos:\n");
	    print_list(Peer_fp, Sos);
	    fprintf(Peer_fp, "\nDemodulators:\n");
	    print_list(Peer_fp, Demodulators);
	    }
	print_stats(Peer_fp);
	print_times(Peer_fp);
	fprintf(Peer_fp, "THE END\n");
	fflush(Peer_fp);
	p4_wait_for_end();
	exit(0);
	}
    
    ac_canonical(lit->atom);
    demodulate_literal(lit);
    oriented = orient_equality(lit);

    lit->weight = term_weight(lit->atom);

    if (lit->weight > Parms[MAX_WEIGHT].val) {
        zap_lit(lit);
        Stats[CLAUSES_WT_DELETED]++;
        }
    else if (forward_subsume(lit)) {
	if (Flags[PRINT_FORWARD_SUBSUMED].val) {
	    fprintf(Peer_fp, "New clause forward subsumed: ");
	    print_clause(Peer_fp, lit); fflush(Peer_fp);
	    }
        Stats[CLAUSES_FORWARD_SUBSUMED]++;
	zap_lit(lit);
	}
    else {
	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);

	owner = decide_owner(lit, (struct list *) NULL);
	lit->id = encode_id(owner, Pid, ++(Peers_assigned_clauses[owner]));
	store_clause_by_id(lit, Id_table);
	Stats[CLAUSES_KEPT]++;
	if (owner == Pid)
	    Stats[RESIDENTS]++;
	else Stats[VISITORS]++;
	fprintf(Peer_fp, "\n** KEPT by Peer%d: ", Pid);
	print_clause(Peer_fp, lit);
	fflush(Peer_fp);

	store_new_clause(lit);
	    
	if (Number_of_peers > 1 && (lit->sign || !Flags[OWN_GOALS].val) &&
	    !find_clause_by_id(lit->id, Already_broadcast_table)) {
	/* Trying to broadcast messages when Number_of_peers == 1 has no     */
	/* effect, but this check is added here to avoid having in the final */
	/* statistics Stats[IM_BROADCAST] != 0 when Number_of_peers == 1.    */
	    clause_to_string(lit, Work_str, MAX_DAC_STRING);
	    Stats[IM_BROADCAST]++;
	    m4_broadcast(INFERENCE_MSG, Work_str, strlen(Work_str)+1);
	    Msgs_diff = Msgs_diff + Number_of_peers - 1;
	    store_clause_by_id(lit, Already_broadcast_table);
	    Peers_work_load[Pid]++;
	    fprintf(Peer_fp, "\n** BROADCAST AS INFERENCE MESSAGE by Peer%d: ", Pid);
	    print_clause(Peer_fp, lit);
	    fflush(Peer_fp);
	} 
	    
	if (oriented && oriented_eq_can_be_demod(lit))
		new_demodulator(lit);
	check_for_proof(lit);

	}
}  /* process_new_clause */

/*************
 *
 *    process_inference_msg(lit)
 *
 *************/

void process_inference_msg(lit)
struct literal *lit;     
{
    int oriented;
    
    	ac_canonical(lit->atom);
    	oriented = orient_equality(lit);
	lit->weight = term_weight(lit->atom);
	
        if (lit->weight > Parms[MAX_WEIGHT].val) {
		list_append(lit, Deleted_clauses);
		store_clause_by_id(lit, Id_table);
        	Stats[CLAUSES_WT_DELETED]++;
        }
        else if (forward_subsume(lit)) {
		if (Flags[PRINT_FORWARD_SUBSUMED].val) {
	    	fprintf(Peer_fp, "inference message forward subsumed: ");
	        print_clause(Peer_fp, lit); fflush(Peer_fp);
	        }
	        list_append(lit, Deleted_clauses);
		store_clause_by_id(lit, Id_table);
        	Stats[CLAUSES_FORWARD_SUBSUMED]++;
	}
        else {

	renumber_vars(lit);  /* This can make it non-ac-canonical. */
	ac_canonical(lit->atom);
	    
	store_clause_by_id(lit, Id_table);

	store_new_clause(lit); /* append in either Usable or Sos */
	
	if (mine(lit))  
		Stats[RESIDENTS]++;
	else Stats[VISITORS]++;
	
	if (oriented && oriented_eq_can_be_demod(lit))
		new_demodulator(lit);
		
	check_for_proof(lit);
	}
}  /* process_inference_msg */

/*************
 *
 *   encode_id()
 *
 *************/

int encode_id(owner, birth_place, num)
int owner, birth_place, num;
{
    return((owner*100 + birth_place)*1000000 + num);
}  /* encode_id */


/*************
 *
 *   handle_inference_msg(msg,from)
 *
 *************/

void handle_inference_msg(msg,from)
char *msg;
int from;
{
    struct literal *lit;

    lit = string_to_clause(msg);
    fprintf(Peer_fp, "Inference_message received from Peer %d: ", from);
    print_clause(Peer_fp, lit);
    Peers_work_load[from]++;
    process_inference_msg(lit);
    
}  /* handle_inference_msg */

/*************
 *
 *    extract_given_clause()
 *
 *	Since Sos is kept sorted by weight, extracting the first one amounts
 *	to extracting the lightest first. If PICK_GIVEN_RATIO is used, then
 *	we find the oldest clause (for breadth-first search) by taking the
 *	clause with the smallest id. Since we take the num(id), there may be
 *	more than one clause with the same value of num(id) (and different
 *	owner(id) and/or birth_place(id)). By taking the first one we find,
 *	we take the lightest among those of smallest num(id).
 *
 *************/

struct literal *extract_given_clause()
{
    struct literal *lit;

    if (!Sos->first)
        return(NULL);
    else {
        int r;
	r = Parms[PICK_GIVEN_RATIO].val;
	if (r == -1)
	    /* Default: smallest weight (should be the first). */
            lit = Sos->first->lit;
        else /* Ratio strategy! */
	     if (Stats[CLAUSES_GIVEN] % (r+1) == 0) {
		struct list_pos *p;
		lit = NULL;
		for (p = Sos->first; p; p = p->next)
		    if (!lit || num(lit->id) > num(p->lit->id))
			lit = p->lit;
		}
	     else lit = Sos->first->lit;
        list_remove(lit, Sos);
        if (mine(lit))
	    Stats[RESIDENTS_IN_SOS]--;
        else
	    Stats[VISITORS_IN_SOS]--;
	return(lit);
        }
}  /* extract_given_clause */

/*************
 *
 *   expansion_work_available()
 *
 *************/

int expansion_work_available()
{
    if (Flags[PARA_PAIRS].val)
	return(Para_pairs_first != NULL);
    else
	return(Sos->first != NULL);
}  /* expansion_work_available */

/*************
 *
 *   expansion_work_given()
 *
 *************/

void expansion_work_given()
{
    struct literal *given_clause, *u;
    struct list_pos *p;

    given_clause = extract_given_clause();
    list_append(given_clause, Usable);

    Stats[CLAUSES_GIVEN]++;

    if (Flags[PRINT_GIVEN].val) {
	fprintf(Peer_fp, "given clause #%d: ", Stats[CLAUSES_GIVEN]);
	print_clause(Peer_fp, given_clause);
	fflush(Peer_fp);
	}

    /* para into residents only, para from anything */

    if (mine(given_clause)) {
	for (p = Usable->first; p; p = p->next) {
	    u = p->lit;
	    para_from_into(u, given_clause, ALL);
	    if (mine(u))
		para_from_into(given_clause, u, ALL_BUT_TOP);
		/* It uses ALL_BUT_TOP, because the topmost position has been already */
		/* covered by para_from_into(u, given_clause, ALL). */
	    }
	}
    else {
	for (p = Usable->first; p; p = p->next) {
	    u = p->lit;
	    if (mine(u))
		para_from_into(given_clause, u, ALL);
	    }
	}
}  /* expansion_work_given */

/*************
 *
 *    expansion_work_pairs()
 *
 *************/

void expansion_work_pairs()
{
    struct literal *c1, *c2;
    struct para_pair *pp;

    pp = get_from_pp();
    c1 = pp->c1;
    c2 = pp->c2;
    free_para_pair(pp);

    if (list_member(c1, Usable) && list_member(c2, Usable)) {

	Stats[CLAUSES_GIVEN]++;

	if (Flags[PRINT_GIVEN].val) {
	    fprintf(Peer_fp, "paramodulate pair #%d:\n", Stats[CLAUSES_GIVEN]);
	    fprintf(Peer_fp, "    "); print_clause(Peer_fp, c1);
	    fprintf(Peer_fp, "    "); print_clause(Peer_fp, c2);
	    fflush(Peer_fp);
	    }
	
	if (mine(c1))
	    para_from_into(c2, c1, ALL);
	
	if (mine(c2)) {
	    if (mine(c1))
		para_from_into(c1,c2, ALL_BUT_TOP);
	    else
		para_from_into(c1,c2, ALL);
	    }
	}
}  /* expansion_work_pairs */

/*************
 *
 *   store_new_clause(lit)
 *
 *************/

void store_new_clause(lit)
struct literal *lit;
{
    if (Flags[PARA_PAIRS].val) {
	struct list_pos *p;

        list_append(lit, Usable);
        for (p = Usable->first; p; p = p->next) {
	    /* make sure that at least one of the two equations in the pair is a resident */
	    if (mine(lit) || mine(p->lit))
            store_in_pp(lit, p->lit);
	    }
	}
    else {
	insert_literal_by_weight(lit, Sos);
	if (mine(lit))
	    Stats[RESIDENTS_IN_SOS]++;
	else
	    Stats[VISITORS_IN_SOS]++;
	}
    
}  /* store_new_clause */

/*************
 *
 *   birth_place(id)
 *
 *************/

int birth_place(id)
int id;
{
    return((id / 1000000) % 100);
}  /* birth_place */

/*************
 *
 *   num(id)
 *
 *************/

int num(id)
int id;
{
    return(id % 1000000);
}  /* num */

/*************
 *
 *   owner(id)
 *
 *************/

int owner(id)
int id;
{
    return(id / 100000000);
}  /* owner */

/*************
 *
 *   mine(lit)
 *
 *************/

int mine(lit)
struct literal *lit;     
{
    return(owner(lit->id) == Pid);
}  /* mine */

/*************
 *
 *   term_to_int()
 *
 *************/

#if 0

static int term_to_int(t)
struct term *t;
{
    int i;

    if (t->type == VARIABLE)
        i = t->varnum;
    else if (t->type == NAME)
        i = t->sym_num;
    else {
        struct rel *r;
        int pos;

        i = t->sym_num;
        pos = 1;
        for (r = t->farg; r; r = r->narg)
            if (sn_to_node(t->sym_num)->lrpo_status == LRPO_MULTISET_STATUS)
                i += term_to_int(r->argval);
            else if (sn_to_node(t->sym_num)->lrpo_status == LRPO_LR_STATUS)
		i += (pos++) * term_to_int(r->argval);
            else        /* no LRPO_RL_STATUS ? */
		i += term_to_int(r->argval) / (pos++);
        }
    return (i);
}  /* term_to_int */

#else

static int term_to_int(t)
struct term *t;
{
    int i;

    if (t->type == VARIABLE)
        /* i = t->varnum; */
	   i = 0;
	   /* So that not only copies but also variants go to the same peer. */
    else if (t->type == NAME)
        i = t->sym_num;
    else {
        struct rel *r;

        i = t->sym_num;
        for (r = t->farg; r; r = r->narg)
            i += term_to_int(r->argval);
        }
    return (i);
}  /* term_to_int */

#endif

/*************
 *
 *   clause_to_peer_function()
 *
 *************/

int clause_to_peer_function(lit)
struct literal *lit;
{
    return(term_to_int(lit->atom) % Number_of_peers);
}  /* clause_to_peer_function */
