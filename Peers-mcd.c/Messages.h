/* MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#ifndef TP_MSGS_H
#define TP_MSGS_H

/* Types of work---messages and local work---and their priorities. */
 
#define HALT_MSG		 1
#define OPTIONS_MSG		 2
#define SYM_TAB_MSG		 3
#define INPUT_PASSIVE_MSG	 4
#define INPUT_USABLE_MSG	 5
#define INPUT_SOS_MSG		 6
#define INPUT_DEMOD_MSG		 7
#define INPUT_DISABLED_MSG	 8
#define INFERENCE_MSG		 9
#define TOKEN_MSG		10
#define INFERENCE_WORK		11

#define NUMBER_OF_WORK_TYPES	11

/* Fast symbol table used to handle messages. */

extern struct sym_ent Symbol_array[MAX_SYMBOL_ARRAY];

/* function prototypes from messages.c */

void send_string(char *buf, int count, int dest, int tag);

void receive_string(char *buf, int count, int *from, int exp_tag, int *act_tag);

void broadcast_string(char *buf, int count, int tag);

int messages_available(int tag);

void options_to_string(char *s, int size);

void string_to_options(char *s);

int next_int(char **sp);

void sym_array_to_normal_sym_tab(Sym_ent_ptr a, int size);

void string_to_sym_array(char *s, Sym_ent_ptr a, int a_size);

void clause_to_string(Clause_ptr c, char *s, int size);

Clause_ptr string_to_clause(char *s);

#endif  /* ! TP_MSGS_H */
