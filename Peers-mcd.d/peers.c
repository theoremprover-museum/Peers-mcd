/* File of the MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#include "Header.h"
#include "List.h"
#include "Io.h"
#include "Unify.h"
#include "Ac.h"
#include "Discrim.h"
#include "Fpa.h"
#include "Clause.h"
#include "Paramod.h"
#include "Pindex.h"
#include "Interp.h"
#include "Peers.h"
#include "Messages.h"
#include "Eqp.h"
#include "Heuristic.h"
#include "mpi.h"

/***** Definition of global variables for peers_mcd (MCD prover) *****/
/* Peers_mcd -- October 1996 */

/* Global variables that are defined here and declared in Header.h.     */

/* Output files: each peer has its own output file: peer1, ..., peern.   */

FILE *Peer_fp;

/* Peer ID and number of peers. */

int Pid;
int Number_of_peers;

/* Global variables that are defined here and declared in Messages.h.   */

/* Fast symbol table used to handle messages. */

struct sym_ent Symbol_array[MAX_SYMBOL_ARRAY];

/* Global variables that are defined here and declared in Peers.h.      */

/* Clause lists. */

List_ptr Usable;
List_ptr Sos;
List_ptr Demodulators;
List_ptr Passive;
List_ptr Disabled;

/* Peers-mcd, April 2000 */
/* Goal_list contains the negated equations (i.e., goals). It is built by */
/* Peer0 in peer_init_work(), and accessed by the heuristic computation   */
/* functions to determine the heuristic_value component of an equation.   */
List_ptr Goal_list;

/* Indexes. */

Discrim_ptr Discrim_demod; /* for demodulation */
Discrim_ptr Discrim_pos;   /* positive literals for forward subsumption */
Discrim_ptr Discrim_neg;   /* negative literals for forward subsumption */

Fpa_index_ptr Fpa_terms;   /* for "from" paramodulation */
Fpa_index_ptr Fpa_alphas;  /* for "into" paramodulation */
Fpa_index_ptr Fpa_bd;      /* for back demodulation */
Fpa_index_ptr Fpa_pos;     /* pos literals: back subsump. and unit conflict */
Fpa_index_ptr Fpa_neg;     /* neg literals: back subsump. and unit conflict */

Pair_index_ptr Para_pairs; /* paramodulation-by-pairs */
Pair_index_ptr Para_pairs_breadth; /* paramodulation-by-pairs with ratio */

/* To access clauses by ID. */

Gen_ptr_ptr *Id_table;

/* Interpretation for semantic inference rules. */

Interp_ptr Interpretation;

/* Peers_assigned_clauses[i] is the number of clauses that Pid assigned */
/* to i so far. It is used by the naming scheme to determine the third  */
/* component (num) of the id of a clause.                               */
/* Therefore, it replaces the counter Id_count of the sequential EQP.   */

int Peers_assigned_clauses[MAX_PROCESSES];

/* Peers_work_load[i] for i =/= Pid is the number of inference messages */
/* that Pid received from i so far. Peers_work_load[Pid] is the number  */
/* of inference messages that Pid sent so far. Peers_work_load[i] is    */
/* used as an estimate of the work-load of peer i by decide_owner()     */
/* with the SELECT_MIN criterion.                                       */

int Peers_work_load[MAX_PROCESSES];

/* The following variable is used by the termination detection algorithm.   */
/* It is incremented whenever a message is sent and decremented whenever a  */
/* message is received. The sum of the Msgs_diff at all the peers will be 0 */
/* when all sent messages have been received.                               */

int Msgs_diff;

/* The following variable is used by the termination detection algorithm:  */
/* its value is 1 if Pid has the token, 0 otherwise.                       */

int Have_token;

/* The following variable is used by the termination detection algorithm,  */
/* so that Peer 0 can detect to have received the token twice.             */

int How_many_times_token_received;

/* The following array is used for building long messages. */

char Work_str[MAX_DAC_STRING];


/*************** End global variables for peers_mcd (MCD prover) ************/

/*************
 *
 *    flag_dep_init() -- initialize those global variables that are
 *                       structures for indexing, depending on flags
 *                       Peers-mcd, October 1996
 *
 *************/

void flag_dep_init(void)
{
    if (!Internal_flags[BT_UNIFY] || Flags[INDEX_BT_DEMOD].val)
        Discrim_demod = discrim_init();

    if (Flags[INDEX_FS].val) {
        Discrim_pos = discrim_init();
        Discrim_neg = discrim_init();
        }

    if (Flags[INDEX_PARAMOD].val) {
        Fpa_alphas = fpa_init(Parms[FPA_DEPTH].val);
        Fpa_terms = fpa_init(Parms[FPA_DEPTH].val);
        }

    if (Flags[INDEX_BD].val)
        Fpa_bd = fpa_init(Parms[FPA_DEPTH].val);

    if (Flags[INDEX_BS].val || Flags[INDEX_CONFLICT].val) {
        Fpa_pos = fpa_init(Parms[FPA_DEPTH].val);
        Fpa_neg = fpa_init(Parms[FPA_DEPTH].val);
        }

    if (Flags[PARA_PAIRS].val) {
        Para_pairs = init_pair_index(Parms[PAIR_INDEX_SIZE].val);
        Para_pairs_breadth = init_pair_index(1);
        }

}  /* flag_dep_init */

/*************
 *
 *    peer_init_and_work
 *
 *************/

void peer_init_and_work()
{
    int i;
    List_pos_ptr p, q;
    Clause_ptr c;
    int i0, i1;
    List_ptr usable, sos, passive, demodulators;
        /* temp lists for the input phase */

    /* Initialization phase at Peer0. */
    
    init();  /* Init stats, clocks, sym_table, options and special_ops */
    
    Pid = 0;            /* this code is executed only by peer0 */

    MPI_Comm_size(MPI_COMM_WORLD, &Number_of_peers);


    fprintf(stdout, "A group of %d peers is active.\n\n", Number_of_peers);
    fprintf(stderr, "A group of %d peers is active.\n\n", Number_of_peers);
    
    for (i = 0; i < MAX_PROCESSES; i++)
        Peers_assigned_clauses[i] = Peers_work_load[i] = 0;
        
    Msgs_diff = 0;      /* # of sent msgs - # of received msgs (globally). */
    
    Have_token = 1;     /* Peer 0 has the token at the beginning. */
    
    How_many_times_token_received = 0;
    
    Disabled = get_list();
    Id_table = init_clause_table_id();

    read_preamble();  /* Read options (up to "end_of_commands"). */
    
    if (Internal_flags[BT_UNIFY]) {     /* set by read_preamble */
        Flags[INDEX_PARAMOD].val = 0;
        Flags[INDEX_BD].val = 0;
        Flags[INDEX_FS].val = 0;
        Flags[INDEX_BS].val = 0;
        Flags[INDEX_CONFLICT].val = 0;
        /* (INDEX_BT_DEMOD is OK with BT_UNIFY) */
        /* (without BT_UNIFY, demodulators always indexed) */
        /* (if any ac or c symbols, BT_UNIFY will be set) */
        }
    
    if (Flags[DIVERSE_SEL].val && !Flags[PARA_PAIRS].val)
        Flags[PARA_PAIRS].val = 1;              /* Peers-mcd -- March 1999 */
        /* With DIVERSE_SEL, Peer0 uses the pair algorithm */
        
    if (Flags[HEURISTIC_SEARCH].val)    /* Peers_mcd -- April 2000 */
         Parms[HEURISTIC_MEASURE].val=OCCNEST;  /* Peer0 uses occnest */
    /* This value is broadcast to all other peers as well, but they modify it */
    /* depending on their Pid when receiving OPTIONS_MSG.                     */
   
    /* Flag-dependent initializations. */
    
    flag_dep_init();
    
    /* Read input clauses into temporary lists. */

    usable = get_list();
    sos = get_list();
    demodulators = get_list();
    passive = get_list();
    
    CLOCK_START(INPUT_TIME)
    read_lists(stdin, usable, sos, passive, demodulators);
    auto_lex_order();  /* For symbols not explicitly set with lex([]). */

    if (Stats[INPUT_ERRORS] != 0) {
        fprintf(stderr, "\nInput errors were found.\007\n\n");
        printf("Input errors were found.\n");
        return;
        }

    Goal_list = get_list();             /* Peers-mcd, September 2000 */
    build_goal_list(usable,sos,passive);
/* Must be done before process_all_input(), because process_all_input() */
/* already needs the goal(s) to compute heuristic values.               */

/* Process input clauses (including all indexing), decide their owners, */
/* and move them to ordinary lists.                                     */
    process_all_input(usable, sos, passive, demodulators);
        
    CLOCK_STOP(INPUT_TIME)
    
    if (Number_of_peers > 1) {
    
    /* Broadcast a new set of options to the peers. */
           
    options_to_string(Work_str, MAX_DAC_STRING);
    broadcast_string(Work_str, strlen(Work_str), OPTIONS_MSG);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
    /* Message broadcast: it is like sending Number_of_peers-1 messages. */
    
    /* Broadcast a new symbol table to the peers. */

    sym_tab_to_string(Work_str, MAX_DAC_STRING);
    string_to_sym_array(Work_str, Symbol_array, MAX_SYMBOL_ARRAY);
    broadcast_string(Work_str, strlen(Work_str), SYM_TAB_MSG);
    Msgs_diff = Msgs_diff + Number_of_peers - 1;
 
    /* Broadcast the input clauses to the peers. */

    /* Peers-mcd, April 2000 */
    /* Broadcast the Goal_list to the peers; it is done before sending       */
    /* any of the input clauses to make sure that the peers already have the */
    /* Goal_list when they begin computing the heuristic values of derived   */
    /* clauses.                                                              */
    /* For input clauses it is not necessary, because the peers do not       */
    /* re-compute the heuristic value of received input clauses, they keep   */
    /* the value computed by Peer0.                                          */
      for (p = Goal_list->first; p; p = p->next) {
          c = p->c;
          clause_to_string(c, Work_str, MAX_DAC_STRING);
          broadcast_string(Work_str, strlen(Work_str), GOAL_MSG);
          Msgs_diff = Msgs_diff + Number_of_peers - 1;
      }
    
    for (p = Passive->first; p; p = p->next) {
        c = p->c;
        clause_to_string(c, Work_str, MAX_DAC_STRING);
        broadcast_string(Work_str, strlen(Work_str), INPUT_PASSIVE_MSG);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
        }
        
    for (p = Usable->first; p; p = p->next) {
        c = p->c;
        clause_to_string(c, Work_str, MAX_DAC_STRING);
        broadcast_string(Work_str, strlen(Work_str), INPUT_USABLE_MSG); 
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
        }
 
     for (p = Sos->first; p; p = p->next) {
        c = p->c;
        clause_to_string(c, Work_str, MAX_DAC_STRING);
        broadcast_string(Work_str, strlen(Work_str), INPUT_SOS_MSG);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
        }
        
    /* Must broadcast demodulators after usable and after sos.  */
    /* This is to handle the case where an equation c in        */
    /* Sos or Usable has been turned into a demodulator.        */
    /* By broadcasting Sos and Usable first, we make sure       */
    /* that c will be received first and thus stored by the     */
    /* other peers with the id set for its occurrence           */
    /* in Sos or Usable.                                        */ 

    for (p = Demodulators->first; p; p = p->next) {
        c = p->c;
        clause_to_string(c, Work_str, MAX_DAC_STRING);
        broadcast_string(Work_str, strlen(Work_str), INPUT_DEMOD_MSG);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
        }
        
    /* Peers-mcd, October 1996: broadcast also input clauses that */
    /* were disabled, for reasons of proof reconstruction.        */
    for (p = Disabled->first; p; p = p->next) {
        c = p->c;
        clause_to_string(c, Work_str, MAX_DAC_STRING);
        broadcast_string(Work_str, strlen(Work_str), INPUT_DISABLED_MSG);
        Msgs_diff = Msgs_diff + Number_of_peers - 1;
        }
        }
    peer_work();
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
    
    /* Initialization phase at any Peer. */

    MPI_Comm_rank(MPI_COMM_WORLD, &Pid);

    MPI_Comm_size(MPI_COMM_WORLD, &Number_of_peers);

    if (gethostname(hostname, 64) != 0)
        str_copy("???", hostname);

    sprintf(s1,"peer%d", Pid);
  
    Peer_fp = fopen(s1, "w");

    fprintf(Peer_fp, "Peer %d (of %d), host %s, %s\n\n",
            Pid, Number_of_peers, hostname, get_time());
    fflush(Peer_fp);

    if (Pid != 0) {
        init(); /* Init stats, clocks, sym_table, options and special_ops */
        
        for (i = 0; i < MAX_PROCESSES; i++)
             Peers_assigned_clauses[i] = Peers_work_load[i] = 0;
 
        Msgs_diff = 0;
        Have_token = 0;
        How_many_times_token_received = 0;
        
        Usable = get_list();
        Sos = get_list();
        Passive = get_list();
        Demodulators = get_list();
        Disabled = get_list();
        Goal_list=get_list(); 
        Id_table = init_clause_table_id();
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

int work_available(int type)
{

    switch (type) {
      case HALT_MSG:
      case OPTIONS_MSG: 
      case SYM_TAB_MSG:
      case INPUT_PASSIVE_MSG:    
      case INPUT_SOS_MSG:
      case INPUT_USABLE_MSG:
      case INPUT_DEMOD_MSG:
      case INPUT_DISABLED_MSG:
      case GOAL_MSG:				/* Peers_mcd, April 2000 */
      case INFERENCE_MSG:
      case TOKEN_MSG:
        return(messages_available(type));
      case INFERENCE_WORK:
        return(inferences_to_make());
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

int find_high_priority_work(void)
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
    int go, exp_work_type, act_work_type, from, to;
    char *s;
    Clause_ptr c;


    go = 1;
    while (go) {
        exp_work_type = find_high_priority_work();
        switch (exp_work_type) {

            /* cases for `message' work */

          case -1:  /* No messages available.  Wait and receive any
message. */

            if (Number_of_peers == 1) {
                fprintf(Peer_fp, "Peer 0 stops, because it's idle, and
there are no other peers.\n");
                fflush(Peer_fp);
                go = 0;
                break;
                }
            else if (Have_token) {
                     if (Pid == 0 && Msgs_diff == 0 && How_many_times_token_received >= 2) {
                         fprintf(Peer_fp, "\nPeer %d stops, because no one is busy.\n", Pid);
                         fflush(Peer_fp);
                         broadcast_string("", 0, HALT_MSG);
                         go = 0;
                         break;
                         }
                     else {
                          to = (Pid + 1) % Number_of_peers;
                          sprintf(Work_str, "%d ", Msgs_diff);
                          send_string(Work_str, strlen(Work_str), to, TOKEN_MSG);
                          Msgs_diff = 0;
                          /* It resets to 0 the local counter,   */
                          /* having sent its value in the token. */
                          Have_token = 0;
                          fprintf(Peer_fp, "\nPeer %d sends token message to Peer %d.\n", Pid, to);
                          fflush(Peer_fp);
                          }
                   }

          case HALT_MSG:        
          case OPTIONS_MSG: 
          case SYM_TAB_MSG:    
          case GOAL_MSG:                      /* Peers_mcd -- April 2000 */
          case INPUT_SOS_MSG:
          case INPUT_PASSIVE_MSG:
          case INPUT_DEMOD_MSG:
          case INPUT_USABLE_MSG:
          case INPUT_DISABLED_MSG:
          case INFERENCE_MSG:
          case TOKEN_MSG:

         /* This part handles all message types, including wait-for-messages */
         
            receive_string(Work_str, MAX_DAC_STRING, &from, exp_work_type,&act_work_type);
         /* Peers_mcd, November 1996: we need to receive in Work_str, because */
         /* the receive of MPI invoked inside receive_string wants to know the*/
         /* length of the receiving buffer. The version with p4 simply used a */
         /* pointer to char and expected the receive of p4 to allocate enough */
         /* buffer space. */
            switch (act_work_type) {

              case HALT_MSG:
                go = 0;
                fprintf(Peer_fp, "\nPeer %d receives halt message in work_loop from Peer %d.\n", Pid, from);
                fflush(Peer_fp);
                break;

              case TOKEN_MSG:
                Have_token = 1;
                How_many_times_token_received++;
                s = Work_str;
                Msgs_diff = Msgs_diff + next_int(&s);
                fprintf(Peer_fp, "Peer %d gets token message from Peer %d.\n", Pid, from);
                fflush(Peer_fp);
                break;

              case SYM_TAB_MSG:
                Msgs_diff--;
/* Message received: decrement the difference between sent and received. */
/* Peers_mcd, November 1997: previous versions had an init_symbol_table here */
/* but it is useless because already invoked by init.*/
                string_to_sym_array(Work_str, Symbol_array, MAX_SYMBOL_ARRAY);
                sym_array_to_normal_sym_tab(Symbol_array, MAX_SYMBOL_ARRAY);
                fprintf(Peer_fp, "Peer %d gets new symbols.\n", Pid);
                fflush(Peer_fp);
                break;

              case OPTIONS_MSG:
                Msgs_diff--;
                string_to_options(Work_str);
   /* After receiving the flags can do flag-dependent initializations. */
                flag_dep_init();

                /* Peers-mcd -- March 1999 */

                if (Flags[DIVERSE_SEL].val) {
                    if (Pid % 2 == 0)
                        Flags[PARA_PAIRS].val = 1;
                    else Flags[PARA_PAIRS].val = 0;
                    }

                if (Parms[PICK_GIVEN_RATIO].val != -1 && Flags[DIVERSE_PICK].val)
                    Parms[PICK_GIVEN_RATIO].val = Parms[PICK_GIVEN_RATIO].val + Pid;

   /* Peers_mcd, April 2000:   establish heuristic measure to be used .*/

                if(Flags[HEURISTIC_SEARCH].val)
			Parms[HEURISTIC_SEARCH].val = Pid % 3; 

                fprintf(Peer_fp, "Peer %d gets new options.\n", Pid);
                fflush(Peer_fp);
                break;

              case GOAL_MSG:                   /* Peers-mcd, April 2000 */
                Msgs_diff--;
                c= string_to_clause(Work_str);
                clause_up_pointers(c);
                list_append(c, Goal_list);
                break;

              case INPUT_PASSIVE_MSG:
                Msgs_diff--;
                c = string_to_clause(Work_str);
                clause_up_pointers(c);
                fprintf(Peer_fp, "Peer %d gets input passive: ", Pid);
                print_clause(Peer_fp, c);
                fflush(Peer_fp);
                list_append(c, Passive);
                store_clause_by_id(c, Id_table);
                index_for_sub(c, 1);
                index_bs_conflict(c, 1);
                Stats[PASSIVE_SIZE]++;
                Stats[PASSIVE_INPUT]++;
                Stats[CLAUSES_INPUT]++;
                Stats[CLAUSES_KEPT_INPUT]++;

                Stats[VISITORS]++;
                break;
                
              case INPUT_USABLE_MSG:
                Msgs_diff--;

                c = string_to_clause(Work_str);
                clause_up_pointers(c);
                
                fprintf(Peer_fp, "Peer %d gets input usable: ", Pid);
                print_clause(Peer_fp, c);
                fflush(Peer_fp);
                
                store_clause_by_id(c, Id_table);

                Stats[USABLE_INPUT]++;
                Stats[CLAUSES_INPUT]++;
                Stats[CLAUSES_KEPT_INPUT]++;
                store_clause(c, Usable);
                break;

              case INPUT_SOS_MSG:
                Msgs_diff--;
                c = string_to_clause(Work_str);
                clause_up_pointers(c);

                if (!c->literals->sign && Flags[OWN_GOALS].val)
                    c->id = encode_id(Pid, Pid,++(Peers_assigned_clauses[Pid]));
/* If OWN_GOALS is on and the clause is negated,                              */ 
/* the receiving peer Pid assigns the clause to itself by setting its owner   */
/* field to Pid. The birth_place field is also set to Pid and the num field   */
/* is set accordingly. Remark that we could not keep the birth_place 0 to say */
/* that the clause was read by Peer0, because Pid does not know the value of  */
/* Peers_assigned_clauses[Pid] at Peer0.                                      */
                
                fprintf(Peer_fp, "Peer %d gets input sos: ", Pid);
                print_clause(Peer_fp, c);
                fflush(Peer_fp);
                
                store_clause_by_id(c, Id_table);
                
                Stats[SOS_INPUT]++;
                Stats[CLAUSES_INPUT]++;
                Stats[CLAUSES_KEPT_INPUT]++;
                store_clause(c, Sos);
                break;

              case INPUT_DEMOD_MSG:
                Msgs_diff--;
                c = string_to_clause(Work_str);
                clause_up_pointers(c);
                fprintf(Peer_fp, "Peer %d gets input demod: ", Pid);
                print_clause(Peer_fp, c);
                fflush(Peer_fp);
                if (!find_clause_by_id(c->id, Id_table)) {
/* Received an input demodulator which is not in Sos or Usable, and therefore */
/* was not alreay received as such. */
                    Stats[DEMODULATORS_INPUT]++;
                    Stats[CLAUSES_INPUT]++;
                    Stats[CLAUSES_KEPT_INPUT]++;
                    Stats[VISITORS]++;
                    store_clause_by_id(c, Id_table);
                    }
                else
/* Received an input demodulator which is in Sos or Usable, and therefore */
/* was alreay received as such. */
                    Stats[NEW_DEMODULATORS_INPUT]++;
                list_append(c, Demodulators);
                Stats[DEMODULATORS_SIZE]++;
                index_demod(c, 1);
                break;


              case INPUT_DISABLED_MSG:
                Msgs_diff--;
                c = string_to_clause(Work_str);
                clause_up_pointers(c);
                fprintf(Peer_fp, "Peer %d gets disabled input clause: ", Pid);
                print_clause(Peer_fp, c);
                fflush(Peer_fp);
                store_clause_by_id(c, Id_table);

                disable_clause(c);
                break;

              case INFERENCE_MSG:
                Msgs_diff--;
                Stats[IM_RECEIVED]++;
                CLOCK_START(INF_MSG_TIME);
                handle_inference_msg(Work_str,from);

                CLOCK_STOP(INF_MSG_TIME);
                break;
        
                } /* end of innermost switch */
            break;


            /* end of message processing */
               
            /* case for local work */

          case INFERENCE_WORK:
                Stats[GIVEN]++;
                CLOCK_START(INFERENCE_TIME);
                make_inferences();
                CLOCK_STOP(INFERENCE_TIME);
            break;

            /* end of local work */

            } /* end of outermost switch */
        } /* end of while loop */
}  /* work_loop */

/*************
 *
 *    check_for_proof(c) -- ASSUME UNIT CLAUSES!!
 *
 *    Check against x=x, Passive, Usable, and Sos.
 *    Clean up and exit if a proof is found and #proofs >= max_proofs.
 *
 *    Using Peer_fp: Peers-mcd, October 1996
 *
 *************/

void check_for_proof(Clause_ptr c)
{

    Literal_ptr lit;
    Term_ptr t1, t2;
    Context_ptr c1, c2;
    Bt_node_ptr bt_position;
    Clause_ptr conflictor;
    int xx_proof;

    if (literal_count(c) != 1) return;

    CLOCK_START(CONFLICT_TIME)
    lit = c->literals;
    xx_proof = 0;

    if (!lit->sign && is_symbol(lit->atom->symbol, "=", 2)) {
        c1 = get_context();
        c2 = get_context();
        t1 = lit->atom->args[0];
        t2 = lit->atom->args[1];
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
        if (Flags[INDEX_CONFLICT].val) {
            Context_ptr s1, s2;
            Fpa_pos_ptr pos;
            Term_ptr found_atom, atom;

            s1 = get_context();
            s2 = get_context();
            atom = c->literals->atom;
            found_atom = fpa_retrieve_first(atom, COMP_LIT_INDEX(c->literals),
                                            UNIFY, s1, s2, &pos);
                                            
            if (found_atom) {
                fpa_cancel(pos);
                conflictor = found_atom->containing_clause;
                }
            else
                conflictor = NULL;
            free_context(s1);
            free_context(s2);
            }
        else {
            conflictor = conflict_list(c, Passive);
            if (!conflictor)
                conflictor = conflict_list(c, Usable);
            if (!conflictor)
                conflictor = conflict_list(c, Sos);
            }
        }

    CLOCK_STOP(CONFLICT_TIME)
    if (xx_proof || conflictor) {

        fprintf(stderr, "---------------- PROOF ----------------");
        fprintf(stderr, "\007\n\n");
        if (conflictor)
            fprintf(Peer_fp, "\nUNIT CONFLICT from <%d,%d,%d> and <%d,%d,%d>",
            owner(c->id), birth_place(c->id), num(c->id),
            owner(conflictor->id), birth_place(conflictor->id),num(conflictor->id));
        else
            fprintf(Peer_fp, "\nUNIT CONFLICT from <%d,%d,%d> and x=x",
            owner(c->id), birth_place(c->id), num(c->id));

        fprintf(Peer_fp, " at %6.2f seconds.\n", run_time() / 1000.);

        Stats[PROOFS]++;

        print_proof(Peer_fp, c, conflictor, Id_table);

        fflush(Peer_fp);

        if (Stats[PROOFS] >= Parms[MAX_PROOFS].val) {
            broadcast_string("", 0, HALT_MSG);
            /* Peers-mcd, October 1996 */
            if (Flags[PRINT_LISTS_AT_END].val) {
                fprintf(Peer_fp, "\nUsable:\n");
                print_list(Peer_fp, Usable);
                fprintf(Peer_fp, "\nSos:\n");
                print_list(Peer_fp, Sos);
                fprintf(Peer_fp, "\nDemodulators:\n");
                print_list(Peer_fp, Demodulators);
                }
            output_stats(Peer_fp); /* Peers-mcd, October 1996 */
            fprintf(Peer_fp, "\nPeer %d found a proof.\n", Pid);
            fprintf(Peer_fp, "THE END\n");
            fflush(Peer_fp);
#if 0
            printf("Fpa_bd: "); print_fpa_index_summary(stdout, Fpa_bd);
            printf("Fpa_terms: "); print_fpa_index_summary(stdout, Fpa_terms);
            printf("Fpa_alphas: "); print_fpa_index_summary(stdout,Fpa_alphas);
            printf("Fpa_pos: "); print_fpa_index_summary(stdout, Fpa_pos);
            printf("Fpa_neg: "); print_fpa_index_summary(stdout, Fpa_neg);

#endif
            MPI_Finalize(); /* Peers-mcd, October 1996 */
            exit(PROOF_EXIT);
            }
        }
}  /* check_for_proof */

/*************
 *
 *   random_by_para_parents
 *
 *************/

int random_by_para_parents(Clause_ptr c)
{
Gen_ptr_ptr g1;

       g1 = c->justification;
       if (g1 && (g1->u.i == PARA_RULE || g1->u.i == PARA_FX_RULE ||
               g1->u.i == PARA_IX_RULE || g1->u.i == PARA_FX_IX_RULE))
               return((g1->next->u.i + g1->next->next->u.i) %Number_of_peers);
       else return(0);

}  /* random_by_para_parents */
 
/*************
 *
 *   random_by_all_parents
 *
 *************/


int random_by_all_parents(Clause_ptr c)
{
Gen_ptr_ptr g1;

        g1 = c->justification;

        if (g1)
            if (g1->u.i == PARA_RULE || g1->u.i == PARA_FX_RULE ||
                g1->u.i == PARA_IX_RULE || g1->u.i == PARA_FX_IX_RULE)
                return((g1->next->u.i + g1->next->next->u.i) %Number_of_peers);
            else if (g1->u.i == BACK_DEMOD_RULE)
                     return(g1->next->u.i % Number_of_peers);
                 else return(0);
        else return(0);

}  /* random_by_all_parents */

/*************
 *
 *   random_by_history
 *
 *************/

int random_by_history(Clause_ptr c)
{
Gen_ptr_ptr p1;
int sum;
        
        sum = 0;    
        for (p1 = c->justification; p1; p1 = p1->next)
             if (p1->u.i > 0)   /* it's a clause id, not a rule code */
                 sum = sum + num(p1->u.i);
        return(sum % Number_of_peers); 

}  /* random_by_history */

/*************
 *
 *   rotation
 *
 *************/

int rotation()
{
static int next_choice = -1;
int cand;
            
if (next_choice == -1)
    next_choice = Pid; /* The first choice in ROTATE is oneself. */
cand = next_choice++ % Number_of_peers;

return(cand);

}  /* rotation */

/*************
 *
 *   select_min
 *
 *************/

int select_min()
{
int j, min, cand;
            
min = MAX_INT;
for (j = 0; j < Number_of_peers; j++)
     if (Peers_work_load[j] < min) {
         min = Peers_work_load[j];
         cand = j;
        }
return(cand);

}  /* select_min */

/*************
 *
 *   ago
 *
 *************/

int ago(Clause_ptr c)
{
int cand, j, max, votes[MAX_PROCESSES];
Gen_ptr_ptr p1, p2, p3;
Clause_ptr c1;

p1 = NULL;
for (p3 = c->justification; p3; p3 = p3->next)
     if (p3->u.i > 0)   /* it's clause id, not rule code */

         get_ancestors(find_clause_by_id(p3->u.i, Id_table), Id_table, &p1);
         
for (j = 0; j < MAX_PROCESSES; j++)
        votes[j] = 0;
        

while (p1) {
        c1 = p1->u.c;
        if (c1)             
            votes[owner(c1->id)]++;
        p2 = p1;
        p1 = p1->next;
        free_gen_ptr(p2);
        }
 
cand = max = 0;
for (j = 0; j < MAX_PROCESSES; j++)
        if (votes[j] > max) {
            max = votes[j];
            cand = j;
           }
  
return(cand);
}  /* ago */

/*************
 *
 *   decide_owner(c, input)
 *
 *   input 1 -- input clause
 *         0 -- not an input clause
 *   
 *************/

int decide_owner(Clause_ptr c, int input)
{
    static int number_of_choices = 0;   /* used only by the AGO strategies */
          
    if (Number_of_peers == 1)
        return(Pid);
    else if (Flags[OWN_GOALS].val && !(c->literals->sign))
        return(Pid);
  /*  else if (c->justification && c->justification->u.i == FLIP_RULE) */
    /*    return(owner(c->justification->next->u.i)); */
    else {
        switch (Parms[DECIDE_OWNER_STRAT].val) {

          case ROTATE:
            return(rotation());
          case SYNTAX:
            return(clause_to_peer_function(c));
          case SELECT_MIN:
            return(select_min());
          case PARENTS:
            return(random_by_para_parents(c));
          case ALL_PARENTS:
            return(random_by_all_parents(c));
          case HISTORY:
            return(random_by_history(c));
          case R_PARENTS:
            if (input)
                return(rotation());
            else  
                return(random_by_para_parents(c));
          case R_ALL_PARENTS:
            if (input)
                return(rotation());
            else
                return(random_by_all_parents(c));
          case R_HISTORY:
            if (input)
                return(rotation());
            else
                return(random_by_history(c));    
          case R_AGO:
            {
            number_of_choices++;  
            if (!input && Parms[SWITCH_OWNER_STRAT].val < MAX_INT && 
                number_of_choices > Parms[SWITCH_OWNER_STRAT].val)
                return(ago(c));
            else
                return(rotation());
            }
          case NO_SUBDIVIDE:    /* Peers-mcd -- March 1999 */
                return(Pid);
          default:
            abend("decide_owner, bad decide_owner_strategy");
            return(0);  /* to quiet lint */
            }   /* end of switch */
        } /* end of else */

}  /* decide_owner */

/*************
 *
 *
 *    broadcast_clause(Clause_ptr c)
 *
 *************/
 
void broadcast_clause(Clause_ptr c)
{

/* Peers_mcd, August 2000 */
  int yes_broadcast=0;
  CLOCK_START(INF_MSG_TIME);
  
  if (Parms[DECIDE_OWNER_STRAT].val != NO_SUBDIVIDE)
      yes_broadcast = 1;                  /* distributed search or hybrid */
  else {                                  /* pure multi-search */
        if (Flags[HEURISTIC_SEARCH].val) {
            if (c->heuristic_value <= Parms[MAX_HV_SEND].val)
               yes_broadcast = 1;
               }
        else {
             if (c->weight <= Parms[MAX_WEIGHT_SEND].val)
               yes_broadcast = 1;
              }
       }

  if(Number_of_peers > 1 && (c->literals->sign || !Flags[OWN_GOALS].val) && yes_broadcast){
     /* Trying to broadcast messages when Number_of_peers == 1 has no     */
     /* effect, but this check is added here to avoid having in the final */
     /* statistics Stats[IM_BROADCAST] != 0 when Number_of_peers == 1.    */
     if (Flags[PRINT_MSGS].val) {
         fprintf(Peer_fp, "\n** BROADCAST AS INFERENCE MESSAGE by Peer%d: ", Pid);
         print_clause(Peer_fp, c);
         fflush(Peer_fp);
         }
     clause_to_string(c, Work_str, MAX_DAC_STRING);
     Stats[IM_BROADCAST]++;
     broadcast_string(Work_str, strlen(Work_str), INFERENCE_MSG);
     Msgs_diff = Msgs_diff + Number_of_peers - 1;
     Peers_work_load[Pid]++;
  } 
    CLOCK_STOP(INF_MSG_TIME);
}  /* broadcast_clause */

/*************
 *
 *    check_for_halt_msg(void)
 *
 *************/
 
void check_for_halt_msg(void)
{
int flag, from, act_work_type;
MPI_Status status;

    MPI_Iprobe(MPI_ANY_SOURCE, HALT_MSG, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        receive_string(Work_str, MAX_DAC_STRING, &from, HALT_MSG, &act_work_type);
        fprintf(Peer_fp, "\nPeer %d receives halt message from Peer %d.\n", Pid, status.MPI_SOURCE);
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
        MPI_Finalize();
        exit(0);
        }

} /* check_for_halt_msg */

/*************
 *
 *    process_inference_msg(Clause_ptr c)
 *
 *************/

void process_inference_msg(Clause_ptr c)  
{
        
   orient_eq_literals(c);       /* Needed to set bits in the clause. */
   renumber_vars(c);  /* This can make it non-ac-canonical. */
   ac_canonical(c->literals->atom);
   clause_up_pointers(c);
   store_clause_by_id(c, Id_table);

   store_clause(c, Sos);
        
   if (eq_can_be_demod(c->literals)) {
       new_demodulator(c, 0);   /* 0 means not input clause */
        if (!Flags[DELAY_BACK_DEMOD].val)
            back_demodulate(c, 0);
            }
                
}  /* process_inference_msg */

/*************
 *
 *   handle_inference_msg(msg, from)
 *
 *************/

void handle_inference_msg(char *msg, int from)
{
    Clause_ptr c;

    c = string_to_clause(msg);
    if (Flags[PRINT_MSGS].val) {
        fprintf(Peer_fp, "Inference_message received from Peer %d: ", from);
        print_clause(Peer_fp, c);
        }
    Peers_work_load[from]++;
    process_inference_msg(c);
    
}  /* handle_inference_msg */

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
 *   mine(clause)
 *
 *************/

int mine(Clause_ptr c)  
{
    return(owner(c->id) == Pid);
}  /* mine */

/*************
 *
 *   term_to_int()
 *
 *************/

static int term_to_int(Term_ptr t)
{
    int i, j;

    if (VARIABLE(t))
           i = 0;
           /* So that not only copies but also variants go to the same
peer. */
    else if (CONSTANT(t))
        i = (t->symbol >= 0) ? t->symbol : -t->symbol;
        /* Peers-mcd, March 1999: modified because integers in the symbol */
        /* table of EQP0.9 and beyond are negative, whereas when this code */
        /* was written in Jan.-Feb. 1993 they were guaranteed to be positive.*/
    else {
        i = (t->symbol >= 0) ? t->symbol : -t->symbol;
        for (j = 0; j < t->arity; j++)
            i += term_to_int(t->args[j]);
        }
    return(i);
}  /* term_to_int */

/*************
 *
 *   clause_to_peer_function
 *
 *************/

int clause_to_peer_function(Clause_ptr c)
{
    return(term_to_int(c->literals->atom) % Number_of_peers);
}  /* clause_to_peer_function */


