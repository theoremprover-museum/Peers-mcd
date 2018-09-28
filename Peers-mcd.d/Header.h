/* File inherited from EQP0.9 and adapted for the */
/* MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd */

#ifndef TP_HEADER_H
#define TP_HEADER_H

/************ BASIC INCLUDES ************/

#if 0
#    include <fpu_control.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*********** Sizes of integers ***************/

#define MAX_LONG_INT 2147483647
#define MAX_INT      2147483647

/******** Types of exit ********/

#define PROOF_EXIT           10
#define ABEND_EXIT           11
#define SOS_EMPTY_EXIT       12
#define MAX_GIVEN_EXIT       13
#define INTERRUPT_EXIT       14
#define INPUT_ERROR_EXIT     15
#define MAX_SECONDS_EXIT     16

/****** Declaration of constants for Peers_mcd (MCD prover) **********/

#define MAX_SYMBOL_ARRAY    200
#define MAX_DAC_STRING    15000

/************* END OF ALL GLOBAL CONSTANT DEFINITIONS ****************/

#include "Options.h"
#include "Stats.h"
#include "Misc.h"
#include "Avail.h"
#include "Term.h"

/****** Declaration of global variables for Peers_mcd (MCD prover) *******/

/* Declared here and defined in peers.c (Peers_mcd -- October 1996) */

/* Output files: each peer has its own output file: peer1, ..., peern. */

extern FILE *Peer_fp;

/* Peer ID and number of peers. */

extern int Pid;
extern int Number_of_peers;

/***********************************************************************/

#endif  /* ! TP_HEADER_H */
