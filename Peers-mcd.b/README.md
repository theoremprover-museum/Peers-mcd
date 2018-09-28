The Peers-mcd.b Theorem Prover

This is the code of the historic Peers-mcd.b theorem prover developed by Maria Paola Bonacina 
at the University of Iowa from June 1996 through October 1996, during her time as Assistant
Professor at UIowa.

Peers-mcd.b was a parallel theorem prover based on the Modified Clause-Diffusion methodology
that Maria Paola Bonacina invented and described in her papers:

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving with contraction: a modified Clause-Diffusion method. In Proceedings of the First International Symposium on Parallel Symbolic Computation (PASCO), World Scientific, Lecture Notes Series in Computing 5, 22-33, 1994.

Maria Paola Bonacina. Future directions of automated deduction: Distributed automated deduction. National Science Foundation Workshop on the Future Directions of Automated Deduction, Chicago, Illinois, USA, April 1996.

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving: a modified Clause-Diffusion method. Journal of Symbolic Computation, 21(4-6):507-522, April-June 1996; DOI: 10.1006/jsco.1996.0028.

Peers-mcd.b was a Modified Clause-Diffusion prover for equational theories, possibly modulo
associativity and commutativity, incorporating as sequential base the EQP0.9 prover by
William W. (Bill) McCune, that had just proved that Robbins algebras are Boolean.
The name emphasizes that all processes in Clause-Diffusion are peers, with no master-slave
relation, and the mcd part abbreviates Modified Clause-Diffusion.
Peers-mcd.b differed from its predecessors also because it featured a new family of criteria
for subdivision of clauses, the Ancestor-Graph Oriented criteria, or AGO criteria for short.
Peers-mcd.b exhibited instances of super-linear speed-up over EQP0.9 in proving two lemmas
that represent two thirds of the proof of the Robbins problem.
In May 1998, Peers-mcd.b succeded in solving the Levi Commutator Problem in Group Theory,
given as a challenge at the CADE-15 Workshop on Problem Solving Methodologies with
Automated Deduction.

Peers-mcd.b was written in C and MPI for both workstation networks and multiprocessors.
This source code repository includes all .c files and .h files.
It also includes makefiles showing how the binary, named peers-mcd, was built for workstation
networks and a multiprocessor.

Peers-mcd.b, the AGO criteria, and the experiments on the Robbins and Levi Commutator
problems, are described in:

Maria Paola Bonacina. The Clause-Diffusion theorem prover Peers-mcd. In Proceedings of the Fourteenth International Conference on Automated Deduction (CADE), Springer, Lecture Notes in Artificial Intelligence 1249, 53-56, 1997; DOI: 10.1007/3-540-63104-6_6.

Maria Paola Bonacina. Experiments with subdivision of search in distributed theorem proving. In Proceedings of the Second International Symposium on Parallel Symbolic Computation (PASCO), Wailea, Maui, Hawaii, July 1997. ACM Press, 88-100, 1997; DOI: 10.1145/266670.266696 (©[ACM Inc.][1997]).

Maria Paola Bonacina. Mechanical proofs of the Levi commutator problem. Workshop on Problem Solving Methodologies with Automated Deduction, at the Fifteenth International Conference on Automated Deduction (CADE), July 1998.

For further information on Modified Clause-Diffusion see:
http://profs.sci.univr.it/~bonacina/distributed.html

For further information on the Clause-Diffusion provers see:
http://profs.sci.univr.it/~bonacina/cdprovers.html

