/* File of the MODIFIED CLAUSE DIFFUSION distributed prover Peers_mcd        */
/* Added by Javeed Chida in April 2000 to implement the heuristics of        */
/* Joerg Denzinger and Matthias Fuchs "Goal-oriented equational theorem      */
/* proving using Team-work" KI 1994. Fixed by MPB in May 2000.               */
/* Modified by MPB in September 2000 to take the presence of varyadic AC     */
/* symbols into account according to Siva Anantharaman and Nirina            */
/* Andrianarivelo "Heuristical criteria in refutational theorem proving"     */
/* (see comments for function occ()).                                        */

#include <limits.h>
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
#include "Symbols.h"
#include "Term.h"
#include "Heuristic.h"

/* Global variables used by occnest */

short Heuristic_symbols[MAX_SYMBOLS];
int Used;
/* Array and counter to keep track of which symbols in the goal have been */
/* already considered in occnest.                                         */

/* end Global variables */

/***************
 *
 *    term_hv(t) --  computes and returns the heuristic value 
 *		     of an equation depending on the value of HEURISTIC_MEASURE.
 *
 *  The parameter t is of type Term_ptr because it is a pointer to struct term,
 *  in reality a pointer to an equation represented as a term with = as the root
 *  symbol.
 *  Indeed term_hv is called by process_clause() in the form
 *  term_hv(c->literals->atom) where c is a pointer to a clause.
 *
 * 	To add a heuristic, add the corresponding heuristic function in this 
 * 	file with prototype:
 * 		int heuristic_function(Term_ptr t)
 * 	and make relevant changes to code in switch construct below and in the 
 * 	case construct for OPTIONS_MSG in work_loop() of peers.c if we want
 *      to assign different heuristics to the peers.
 *
 ***************/

int term_hv(Term_ptr t)
{
 int hv;

    switch (Parms[HEURISTIC_MEASURE].val) {
      case 0: hv = occnest(t);
		   break;
      case 1: hv = goal_in_CP(t);
		   break;
      case 2: hv = CP_in_goal(t);
		   break;
     default: abend("in term_heuristic_value, heuristic_measure out of range.");
     }
 return hv;
} /* term_heuristic_value */


/**************
*
*	occnest(t) -- computes and returns the structural complexity
*		      measure 'occnest' for the equation t.
*
*
***************/

int occnest(Term_ptr t)
{
 List_pos_ptr g;
 int occnest_val=1;
 int min=INT_MAX;

 /* do goal-similarity check for every goal */
 for(g=Goal_list->first; g; g=g->next)
    {
     occnest_val = weight_phi(t->args[0]) + weight_phi(t->args[1]);
     occnest_val *= effective_m_f(t,g->c->literals->atom);
     if(occnest_val < min) 
       min=occnest_val;
    }
    
 /* return the lowest heuristic value computed among all the goal equations */
 return min;
 
} /* occnest */


/**************
*
*       goal_in_CP(tt) -- computes and returns the goal similarity
*                     	  measure goal_in_CP for the equation tt.
*
*   See comment in sim_GC()
***************/

int goal_in_CP(Term_ptr tt)
{
 int subcase=0;
 int min=MAX_INT;
 int sim_GC_u_s=0, sim_GC_v_t=0, sim_GC_u_t=0, sim_GC_v_s=0;
 int goal_in_CP_val;
 int min_struct;
 List_pos_ptr g;
 Term_ptr u, v, s, t;
 Clause_ptr contain_cl;

 u=tt->args[0];
 v=tt->args[1];

 /* do goal-similarity check for every goal */
 for(g=Goal_list->first; g; g=g->next)
    {
     s=g->c->literals->atom->args[0];  /* store the sides of the goal */
     t=g->c->literals->atom->args[1];   /* in s and t */

     /* min_struct fixed as per Denzinger-Fuchs paper, page 9 */
     min_struct=u->arity + 3; 	
     sim_GC_u_s=sim_GC(u,u,s,min_struct);	

     min_struct=v->arity + 3;
     sim_GC_v_t=sim_GC(v,v,t,min_struct); 

     min_struct=u->arity + 3;
     sim_GC_u_t=sim_GC(u,u,t,min_struct);

     min_struct=v->arity + 3; 
     sim_GC_v_s=sim_GC(v,v,s,min_struct);

 /* These four top calls have the first two arguments equal, because we start */
 /* from the top subterm of the term, which is the term itself, e.g., u for u.*/

     subcase=find_subcase(sim_GC_u_s,sim_GC_v_t);
     /* determine which case applies */

/* The following switch statement implements the second half of Def. 4.4 */
/* pag. 10 of Denzinger-Fuchs.                                           */

     switch(subcase) {
     case 1: goal_in_CP_val=MIN_2(sim_GC_u_s+sim_GC_v_t, sim_GC_u_t+sim_GC_v_s);
             break;

     case 2: /* Javeed had to do this despite MAX_INT */
             if((sim_GC_u_s + weight_phi(v)) > INT_MAX)
                 sim_GC_u_s -= weight_phi(v);
             if((sim_GC_u_t + weight_phi(v)) > INT_MAX)
                 sim_GC_u_t -= weight_phi(v);
             if((sim_GC_v_s + weight_phi(u)) > INT_MAX)
                 sim_GC_v_s -= weight_phi(u);
             if((sim_GC_v_t + weight_phi(u)) > INT_MAX)
                 sim_GC_v_t -= weight_phi(u);

             goal_in_CP_val=min_4(sim_GC_u_s+weight_phi(v),sim_GC_u_t
              +weight_phi(v),sim_GC_v_s+weight_phi(u),sim_GC_v_t
               +weight_phi(u)) * SINGLE_MATCH; 

     default: goal_in_CP_val=(weight_phi(u) + weight_phi(v) ) * NO_MATCH;
      } /* end switch */

     if(goal_in_CP_val < min)
       min=goal_in_CP_val;
    } /* end for */

 return min;          
 /* return the lowest heuristic value computed among all the goals */
 
} /* goal_in_CP */


/**************
*
*       sim_GC(u,u_at_p,s,min_struct)
*       used by the goal similarity measure, goal_in_CP.
*	u is the side of the equation whose heuristic value we are computing
*       u_at_p is the proper subterm of u at position p; initially, u_at_p	is u itself.
*       s is the side of the goal with respect to which we are computing the
*       heuristic value
*       min_struct is an int parameter used in the computation and defined in
*       goal_in_CP()
*
***************/

int sim_GC(Term_ptr u, Term_ptr u_at_p, Term_ptr s, int min_struct)
{
 int min=INT_MAX;
 int i,ans;
 Context_ptr c1;

 c1=get_context();
 if (Internal_flags[BT_UNIFY]) 
   {
    Bt_node_ptr bt_position;
    bt_position = match_bt_first(u_at_p, c1, s, 0); 
    if((bt_position) && (weight_phi(u_at_p) >= min_struct))  
        /* s is an instance of u_at_p */
      {
       match_bt_cancel(bt_position);
       ans=(weight_phi(u) - weight_phi(u_at_p));
      }
    else ans=MAX_INT;
   }
 else 
    {
     Trail_ptr tr = NULL;

/* Javeed's code had  match(u_at_p,c1,u,&tr) in the next line which does not */
/* make sense because match(t_1, c_1, t_2, trail_addr) returns 1 if t_2 is   */
/* an instance of t_1 and we are checking whether s is an instance of u_at_p,*/
/* not whether u is an instance of u_at_p!                                   */
/* The usage of context c1 is the same as in subsume() in eqp.c:             */
/* match() reuses c1 after match_bt_first() has used it.                     */

     if (match(u_at_p, c1, s, &tr) && (weight_phi(u_at_p) >= min_struct)) 
       {
        clear_subst_1(tr);
        ans=(weight_phi(u) - weight_phi(u_at_p));
       }
     else ans=MAX_INT;
    }
 free_context(c1);

 if(ans < min)
    return ans;
/* If we got ans < infinity with u_at_p we may as well return it, because we */
/* need the minimum such value over the subterms of u (according to Def. 4.3 */
/* at page 9 in the Denzinger-Fuchs paper), and the value of                 */
/* weight_phi(u) - weight_phi(u_at_p) may only grow as we consider subterms  */
/* u_at_p deeper in u.                                                       */
/* Otherwise, we proceed to consider the subterms of u_at_p.                 */

 for(i=0; i<u_at_p->arity; i++)
     {
      Term_ptr tt=u_at_p->args[i];
      ans=sim_GC(u,tt,s,min_struct);
      if(ans<min)
        min=ans;
     }
 return min;
} /* sim_GC */


/**************
*
*       CP_in_goal(t) -- computes and returns the goal similarity 
*                        measure, CP_in_goal, for the term t.
*
***************/

int CP_in_goal(Term_ptr tt)
{
 int subcase=0;
 int min=INT_MAX;
 int sim_CG_u_s=0, sim_CG_v_t=0, sim_CG_u_t=0, sim_CG_v_s=0;
 int CP_in_goal_val;
 int temp_weight;
 List_pos_ptr g;
 Term_ptr u, v, s, t;

 u=tt->args[0];
 v=tt->args[1];

 /* do goal-similarity check for every goal */
 for(g=Goal_list->first; g; g=g->next)
    {
     s=g->c->literals->atom->args[0];  /* store the terms of the goal */
     t=g->c->literals->atom->args[1];   /* in s and t */

     sim_CG_u_s=sim_CG(u,s,s);
     sim_CG_v_t=sim_CG(v,t,t);
     sim_CG_u_t=sim_CG(u,t,t);
     sim_CG_v_s=sim_CG(v,s,s);

/* These four top calls have the last two arguments equal, because we start  */
/* from the top subterm of the term, which is the term itself, e.g. s for s. */

     subcase=find_subcase(sim_CG_u_s,sim_CG_v_t);
     /* determine which case applies */

/* The following switch statement implements the first half of Def. 4.4      */
/* page 10 of Denzinger-Fuchs.                                               */

     switch(subcase) {
     case 1: CP_in_goal_val=MIN_2(sim_CG_u_s+sim_CG_v_t, sim_CG_u_t+sim_CG_v_s);
             break;

     case 2: /* Javeed had to do this despite MAX_INT */
             if((sim_CG_u_s + weight_phi(t)) > INT_MAX)
               sim_CG_u_s -= weight_phi(t);
             if((sim_CG_u_t + weight_phi(s)) > INT_MAX)
               sim_CG_u_t -= weight_phi(s);
             if((sim_CG_v_s + weight_phi(t)) > INT_MAX)
               sim_CG_v_s -= weight_phi(s);
             if((sim_CG_v_t + weight_phi(s)) > INT_MAX)
               sim_CG_v_t -= weight_phi(s);

             CP_in_goal_val=min_4(sim_CG_u_s+weight_phi(t),sim_CG_u_t
              +weight_phi(s),sim_CG_v_s+weight_phi(t),sim_CG_v_t
               +weight_phi(s)) * SINGLE_MATCH;
               break;

     default: CP_in_goal_val=(weight_phi(u) + weight_phi(v) ) * NO_MATCH;
     
      } /* end switch */
     if(CP_in_goal_val < min)
       min=CP_in_goal_val;
    } /* end for */

 return min;
 /* return the lowest heuristic value computed among all the goal terms */
 
} /* CP_in_goal */


/**************
*
*       sim_CG(u,s,s_at_p)
*       used by the goal similarity measure, CP_in_goal.
*	u is the side of the equation whose heuristic value we are computing
*       s is the side of the goal with respect to which we are computing the
*       heuristic value
*       s_at_p is the proper subterm of s at position p; initially, s_at_p	is s itself.
*
***************/

int sim_CG(Term_ptr u, Term_ptr s, Term_ptr s_at_p)
{
 int min=MAX_INT;
 int i,ans;
 Context_ptr c1;

 c1=get_context();
 if (Internal_flags[BT_UNIFY])
   {
    Bt_node_ptr bt_position;
    bt_position = match_bt_first(u, c1, s_at_p, 0);         
    /* s_at_p is an instance of u */
    if(bt_position)
      {
       match_bt_cancel(bt_position);
       ans=(weight_phi(s) - weight_phi(s_at_p));
      }
    else ans=MAX_INT;
   }
 else
    {
     Trail_ptr tr = NULL;

/* Javeed's code had  match(u,c1,u,&tr) in the next line which does not make */
/* sense because match(t_1, c_1, t_2, trail_addr) returns 1 if t_2 is an     */
/* instance of t_1 and we are checking whether s_at_p is an instance of u,   */
/* not whether u is an instance of u!                                        */
/* The usage of context c1 is the same as in subsume() in eqp.c:             */
/* match() reuses c1 after match_bt_first() has used it.                     */

     if (match(u, c1, s_at_p, &tr))
       {
        clear_subst_1(tr);
        ans=(weight_phi(s) - weight_phi(s_at_p));
       }
     else ans=MAX_INT;
    }
 free_context(c1); 

 if(ans < min)
   return ans; 
/* If we got ans < infinity with s_at_p we may as well return it, because we */
/* need the minimum such value over the subterms of s (according to Def. 4.3 */
/* at page 9 in the Denzinger-Fuchs paper), and the value of                 */
/* weight_phi(s) - weight_phi(s_at_p) may only grow as we consider subterms  */
/* s_at_p deeper in s.                                                       */
/* Otherwise, we proceed to consider the subterms of s_at_p.                 */

 for(i=0; i<s_at_p->arity; i++)
     {
      Term_ptr tt=s_at_p->args[i];
      ans=sim_CG(u,s,tt);
      if(ans<min)
        min=ans;
     }

 return min;
} /* sim_CG */


/************
*
*        find_subcase(sim_us,sim_vt)
*        returns the case (I, II, or III, according to page 9
*        of the Denzinger-Fuchs paper) that is satisfied by the substitutions
*		      of CP_in_goal and goal_in_CP.
*
***************/

int find_subcase(int us, int vt)
{
 if ((us<MAX_INT) && (vt<MAX_INT))
   return 1;		                   /* Case 1 is satisfied */
 else if ((us<MAX_INT) || (vt<MAX_INT))
         return 2;	                   /* Case 2 is satisfied */
 else if ((us==MAX_INT) && (vt==MAX_INT))  /* Case 3 is satisfied */
        return 3;

} /* find_subcase */


/************
*
*        insert_clause_by_hv(c, l) -- inserts a clause c into list l
*			              in sorted order, sorted by heuristic
*				      value, adapted from term_weight.
*
***************/

void insert_clause_by_hv(Clause_ptr c, List_ptr l)
{
 List_pos_ptr p;

for (p = l->first; p && c->heuristic_value >= p->c->heuristic_value; p = p->next);
    if (p)
        list_insert_before(c, p);
    else
        list_append(c, l);
        
} /* insert_clause_by_hv */


/**************
*
*        weight_phi(t) -- computes the weight, phi, of a term t,
* 	 		  used by occnest, goal_in_CP and CP_in_goal.
*
***************/

int weight_phi(Term_ptr t)
{
 int w=2;
 int i=0;
 if (is_symbol(t->symbol, "=", 2))
    return (weight_phi(t->args[0]) + weight_phi(t->args[1]));
 if (VARIABLE(t))
    return 1;
 else if (CONSTANT(t))
         return 2;
 else /* complex term */
     while(i < t->arity)
     	   w+=weight_phi(t->args[i++]);
 return w;
} /* weight_phi /*


/**************
*
*        occ(f,t) -- computes the number of occurences of a function symbol f
*        in a term t; called by occ_fins, which in turn is called by
*        effective_m_f, which in turn is called by occnest.
*        Note that in the top call by occ_fins t is not a term,
*        but an equation or goal. In the proceeding recursive calls
*        t is a term. In this respect, it is useful that equations and goals
*        (which are negated equations) are implemented as terms
*        with = as top symbol.
*        Modified in September 2000 to treat AC symbols as varyadic symbols:
*        AC symbols are varyadic because of flattened form as pointed out
*        in the Anantharaman-Hsiang paper on JAR on the Moufang identities.
*        The Denzinger-Fuchs paper refers to UKB, not AC-UKB, and does not
*        mention varyadic symbols. The Anantharaman-Andrianarivelo paper
*        covers also varyadic symbols. Since the Denzinger-Fuchs occurrence
*        measure is the same as the occurrence measure m_0 in the
*        Anantharaman-Andrianarivelo paper, except for varyadic symbols,
*        the treatment of varyadic symbols of m_0 is added here.
*
***************/

int occ(Symbol_type fsymbol, Term_ptr t)
{
 int i;
 int occ_val = 0;
 int temp;
 
 if (is_symbol(t->symbol, "=", 2))
    return MAXVAL( occ(fsymbol, t->args[0]), occ(fsymbol, t->args[1]) );
 /* this is the case used in the top call when t is an equation */

 if (VARIABLE(t))
     return 0;
 else if (is_assoc_comm(fsymbol))      /* Peers-mcd, September 2000 */
          return 0;
 else if (is_assoc_comm(t->symbol)) {
          for (i=0; i < t->arity; i++) {
               temp = occ(fsymbol, t->args[i]);
               if (occ_val < temp)
                   occ_val = temp;
              }
         }
 else {
      if (fsymbol == t->symbol)
          occ_val = 1;   /* case : t = f(t1,t2,...,tn)        */
      else occ_val=0; 	 /* case : t = g(t1,t2,...,tn), f!=g  */
      
      for (i=0; i < t->arity; i++)
           occ_val += occ(fsymbol, t->args[i]);
      }
      
 return occ_val;
} /* occ */


/**************
*
*       effective_m_f(tt, goal)
*       computes the values of factor m_f, with respect to the
*       equation tt = (u=v) and goal = (s=t),
*       for all function symbols f occurring in goal
*       and returns their product.
*	
*      	Note that when the goal term is heavier than the term t, m_f_val is 1
*      	since psi invoked in occ_fins() returns 1 for negative arguments.
*
***************/

int effective_m_f(Term_ptr tt, Term_ptr goal)
{
 int i;
 Term_ptr s,t;
 int m_f_val=1;	/* m_f_val=1 untouched inside for-loop below for case 1 */
 int occ_f_in_s, occ_f_in_t;

 s=goal->args[0];
 t=goal->args[1];
 /* compute the value of m_f for every f that appears in the goal, i.e, for */
 /* every f that appears in s and t if the goal is s=t */

 Used=0;
/* This global variable is used to avoid computing m_f twice if f occurs in */
/* goal more than once.                                                                                          */

 occ_f_in_s=occ_fins(tt, goal, s);
 occ_f_in_t=occ_fins(tt, goal, t);

 return occ_f_in_s * occ_f_in_t;

} /* effective_m_f */


/**************
*
*        occ_fins(tt, goal, s) -- used by effective_m_f;
*        computes the value of m_f with respect to the equation tt (u=v)
*        and the goal (s=t)
*        for all function symbol f that appears in the term s
*        (which is a side of the goal).
*
***************/


int occ_fins(Term_ptr tt, Term_ptr goal, Term_ptr s)
{
 int i, m_f_val;
 Symbol_type f;

 if(s->arity==0)
  return 1;      /* s contains no function symbol */
 else
     {
      f=s->symbol;
      if( (Used==0) || not_in_Heuristic_symbols(f)) /* f not already considered */
        {
          Heuristic_symbols[Used]=f;
          Used++;

          /* we first compute the m_f value for the symbol s->symbol */
          m_f_val=psi(occ(f,tt)-occ(f,goal)) * psi(nest(s,tt)-nest(s,goal));

          /* and now for all other function symbols occurring in s */
          for(i=0; i < s->arity; i++)
              m_f_val*=occ_fins(tt, goal, s->args[i]);

          return m_f_val;
         }
       else return 1;
       /* f already considered, return 1 which is identity of product */
     }
} /* occ_fins */


/**************
*
*        not_in_Heuristic_symbols(f)
*        the function symbols that appear in the
* 	 goal term are accumulated in f (no duplicates). Thus the value of 
* 	 occ_fins(tt, goal, s) is computed only if a symbol f (= s->symbol)
* 	 does not appear in Heuristic_symbols[ ]. Note that s is a subterm of 
* 	 goal at every invocation of occ_fins().
*
***************/


int not_in_Heuristic_symbols(short f)
{
 int i;

 if(Used!=0)
   for(i=0; i<Used; i++)
      {
       if(f==Heuristic_symbols[i])
         return 0;
      }
 return 1;
}

/**************
*
*        nest(s, t)
*        computes the nesting of a function symbol, s->symbol, in a term	t.
*        Similar to occ(), in the top call by occ_fins() t is not a term,
*        but an equation.
*
***************/

int nest(Term_ptr s, Term_ptr t)
{

 if (is_symbol(t->symbol, "=", 2))
 /* Javeed's code here had s->symbol which does not make sense -- MPB, May 2000 */
    return MAXVAL( nest(s, t->args[0]), nest(s, t->args[1]) );

 if(CONSTANT(s))
   return 0;
 else
   return hnest(s->symbol, t, 0, 0);

} /* nest */


/**************
*
*        hnest(f, t) -- auxiliary function, called by nest()
*        f is a symbol and t is a term: it cannot be an equation,
*        because the top case with the equation has been already handled
*        by nest()
*
***************/

int hnest(Symbol_type fsymbol, Term_ptr t, int cur, int abs)
{
 int i, hnest_val, max_val=0;

 if(VARIABLE(t) || CONSTANT(t))
   return MAXVAL(cur,abs);
 else if(fsymbol!=t->symbol)	      /* t= g(t1, t2,..., tn), g != fsymbol */
        {
         for(i=0; i < t->arity; i++)
            {
             hnest_val=hnest(fsymbol, t->args[i], 0, MAXVAL(cur, abs));
             if(hnest_val > max_val)
                max_val=hnest_val;
            }
	         return max_val;
        } 
 else{	                              /* t= f(t1, t2,..., tn), f == fsymbol */
      for(i=0; i < t->arity; i++)
           {
            hnest_val=hnest(fsymbol, t->args[i], cur+1, abs);
            if(hnest_val > max_val)
               max_val=hnest_val;
            }
       return max_val;
     }
}/* hnest */


/**************
*
*        psi(x) -- used by effective_m_f.
*
***************/

int psi(int x)
{
 return ( x<=0 ? 1 : (x+1));
}/* psi */



/**************
*
*        build_goal_list()
*
*        It builds a list of goal clauses which 
* 	 may be used by goal-oriented heuristic functions.
* 	 Note that copy_clause() does not copy id, weight, heuristic_value,
*        bits, or justification. It is not a problem, because the goal
*        goal clauses in this list are accessed only by the functions
*        that compute the heuristic values, they are used as syntactic
*        patterns, not as "real clauses". -- Peers-mcd, September 2000		      
*
***************/

void build_goal_list(List_ptr usable, List_ptr sos, List_ptr passive)
{
 List_ptr cursor;
 List_pos_ptr p;
 Clause_ptr cl;
 int list_counter = 0;

 while (list_counter < 3) {
       if (list_counter == 0) 
         cursor = usable;	
       else if (list_counter == 1)
              cursor = sos;
       else cursor = passive;
       
       for (p = cursor->first; p; p = p->next)
          if (!p->c->literals->sign) { /* negated clause */
             cl = copy_clause(p->c);
 		  /* add goal term to list */
	     list_append(cl, Goal_list);  
            } 
       list_counter++;
      } 
      
 if (Internal_flags[BT_UNIFY]) {
 /* Make terms in Goal_list ac_canonical to aid in backtrack unifying */
 /* when the computation of heuristic values requires unification.    */
 for (p = Goal_list->first; p; p = p->next)
      ac_canonical(p->c->literals->atom);
 }
 
}/* build_goal_list*/


/**************
*	         min_4(a,b,c,d) --- custom function to return the smaller of
* 			            4 integers - used by CP_in_goal, goal_in_CP.
***************/

int min_4(int a, int b, int c, int d)
{
 int min;
 min=MIN_2(MIN_2(a,b),MIN_2(c,d));
 return min;
}
 
