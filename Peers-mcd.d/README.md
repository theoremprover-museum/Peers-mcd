The Peers-mcd.d Theorem Prover

This is the code of the historic Peers-mcd.d theorem prover developed by Maria Paola Bonacina 
at the University of Iowa and at the Università degli Studi di Roma "La Sapienza" in the
Spring and Summer of 2000, during her time as Associate Professor at UIowa. Javeed Chida
contributed the implementation of some heuristics for multi-search as part of his MS thesis
work at UIowa.

Peers-mcd.d was a parallel theorem prover based on the Modified Clause-Diffusion methodology
that Maria Paola Bonacina invented and described in her papers:

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving with contraction: a modified Clause-Diffusion method. In Proceedings of the First International Symposium on Parallel Symbolic Computation (PASCO), World Scientific, Lecture Notes Series in Computing 5, 22-33, 1994.

Maria Paola Bonacina. Future directions of automated deduction: Distributed automated deduction. National Science Foundation Workshop on the Future Directions of Automated Deduction, Chicago, Illinois, USA, April 1996.

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving: a modified Clause-Diffusion method. Journal of Symbolic Computation, 21(4-6):507-522, April-June 1996; DOI: 10.1006/jsco.1996.0028.

Peers-mcd.d was a Modified Clause-Diffusion prover for equational theories, possibly modulo
associativity and commutativity, incorporating as sequential base the EQP0.9d prover by
William W. (Bill) McCune, of Robbins algebras' fame.
The name emphasizes that all processes in Clause-Diffusion are peers, with no master-slave
relation, and the mcd part abbreviates Modified Clause-Diffusion.
Peers-mcd.d made available in the framework of Modified Clause-Diffusion both distributed
search (subdivision of the search space) and multi-search, that differentiates the searches
generated by the parallel processes by assigning them different search plans.
Peers-mcd.d can work in three modes:
    * Pure distributed-search mode: search space subdivided among the processes - all processes execute the same search plan;
    * Pure multi-search mode: search space not subdivided - every process executes a different search plan;
    * Hybrid mode: search space subdivided - the processes execute different search plans. 
For each mode, the prover implements various strategies.

Peers-mcd.d was written in C and MPI for workstation networks.
This source code repository includes all .c files and .h files.
It also includes a makefile showing how the binary, named peers-mcd, was built.

Peers-mcd.d, its modes and strategies, and experiments showing that Peers-mcd.d could prove
the Moufang identities in alternative rings without cancellation laws and exhibiting
instances of super-linear speed-up due to parallel search are described in:

Maria Paola Bonacina. Combination of distributed search and multi-search in Peers-mcd.d. In Proceedings of the 1st International Joint Conference on Automated Reasoning (IJCAR), Siena, Italy, June 2001. Springer, Lecture Notes in Artificial Intelligence 2083, 448-452, 2001; DOI: 10.1007/3-540-45744-5_37.

For further information on Modified Clause-Diffusion see:
http://profs.sci.univr.it/~bonacina/distributed.html

For further information on the Clause-Diffusion provers see:
http://profs.sci.univr.it/~bonacina/cdprovers.html
