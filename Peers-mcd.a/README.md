The Peers-mcd.a Theorem Prover

This is the code of the historic Peers-mcd.a theorem prover developed by Maria Paola Bonacina 
at the University of Iowa in June 1995, during her time as Assistant Professor at UIowa.

Peers-mcd.a was a parallel theorem prover based on the Modified Clause-Diffusion methodology
that Maria Paola Bonacina invented and described in her papers:

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving with contraction: a modified Clause-Diffusion method. In Proceedings of the First International Symposium on Parallel Symbolic Computation (PASCO), World Scientific, Lecture Notes Series in Computing 5, 22-33, 1994.

Maria Paola Bonacina. Future directions of automated deduction: Distributed automated deduction. National Science Foundation Workshop on the Future Directions of Automated Deduction, Chicago, Illinois, USA, April 1996.

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving: a modified Clause-Diffusion method. Journal of Symbolic Computation, 21(4-6):507-522, April-June 1996; DOI: 10.1006/jsco.1996.0028.

Peers-mcd.a was a Modified Clause-Diffusion prover for equational theories, possibly modulo
associativity and commutativity, incorporating as sequential base code from William W. (Bill)
McCune's Otter Parts Store (version 0.2 of December 1992). The name emphasizes that all
processes in Clause-Diffusion are peers, with no master-slave relation, and the mcd part
emphasizes the move from Clause-Diffusion to Modified Clause-Diffusion.

Peers-mcd.a was written in C and p4 for workstation networks.
This source code repository includes all .c files and .h files.
It also includes a makefile showing how the binary, named peers-mcd, was built.

Peers-mcd.a and its experiments up to October 1996 were presented in:

Maria Paola Bonacina. On the reconstruction of proofs in distributed theorem proving: a modified Clause-Diffusion method. Journal of Symbolic Computation, 21(4-6):507-522, April-June 1996; DOI: 10.1006/jsco.1996.0028.

For further information on Modified Clause-Diffusion see:
http://profs.sci.univr.it/~bonacina/distributed.html

For further information on the Clause-Diffusion provers see:
http://profs.sci.univr.it/~bonacina/cdprovers.html

