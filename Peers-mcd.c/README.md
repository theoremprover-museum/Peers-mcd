The Peers-mcd.c Theorem Prover

This is the code of the historic Peers-mcd.c theorem prover developed by Maria Paola Bonacina 
at the University of Iowa in the Spring of 1999, during her time as Associate Professor
at UIowa.

Peers-mcd.c was a parallel theorem prover based on the Modified Clause-Diffusion methodology
that Maria Paola Bonacina invented and described in her papers:

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving with contraction: a modified Clause-Diffusion method. In Proceedings of the First International Symposium on Parallel Symbolic Computation (PASCO), World Scientific, Lecture Notes Series in Computing 5, 22-33, 1994.

Maria Paola Bonacina. Future directions of automated deduction: Distributed automated deduction. National Science Foundation Workshop on the Future Directions of Automated Deduction, Chicago, Illinois, USA, April 1996.

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving: a modified Clause-Diffusion method. Journal of Symbolic Computation, 21(4-6):507-522, April-June 1996; DOI: 10.1006/jsco.1996.0028.

Peers-mcd.c was a Modified Clause-Diffusion prover for equational theories, possibly modulo
associativity and commutativity, incorporating as sequential base the EQP0.9d prover by
William W. (Bill) McCune, of Robbins algebras' fame.
The name emphasizes that all processes in Clause-Diffusion are peers, with no master-slave
relation, and the mcd part abbreviates Modified Clause-Diffusion.
Peers-mcd.c featured the Ancestor-Graph Oriented criteria for the subdivision of clauses,
and exhibited super-linear speed-up over EQP0.9d in two and almost super-linear in the third
of the lemmas that make the proof of the Robbins theorem.

Peers-mcd.c was written in C and MPI for workstation networks.
This source code repository includes all .c files and .h files.
It also includes a makefile showing how the binary, named peers-mcd, was built.

The experiments on the Robbins problem are described in:

Maria Paola Bonacina. A taxonomy of parallel strategies for deduction. Annals of Mathematics and Artificial Intelligence, 29(1-4):223-257, 2000; DOI: 10.1023/A:1018932114059.

For further information on Modified Clause-Diffusion see:
http://profs.sci.univr.it/~bonacina/distributed.html

For further information on the Clause-Diffusion provers see:
http://profs.sci.univr.it/~bonacina/cdprovers.html

