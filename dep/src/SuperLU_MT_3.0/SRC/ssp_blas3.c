
/*
 * -- SuperLU routine (version 1.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * November 15, 1997
 *
 */
/*
 * File name:		sp_blas3.c
 * Purpose:		Sparse BLAS3, using some dense BLAS3 operations.
 */

#include "slu_mt_sdefs.h"


int_t
sp_sgemm(char *trans, int_t m, int_t n, int_t k, 
         float alpha, SuperMatrix *A, float *b, int_t ldb, 
         float beta, float *c, int_t ldc)
{
/*  Purpose   
    =======   

    sp_s performs one of the matrix-matrix operations   

       C := alpha*op( A )*op( B ) + beta*C,   

    where  op( X ) is one of 

       op( X ) = X   or   op( X ) = X'   or   op( X ) = conjg( X' ),

    alpha and beta are scalars, and A, B and C are matrices, with op( A ) 
    an m by k matrix,  op( B )  a  k by n matrix and  C an m by n matrix. 
  

    Parameters   
    ==========   

    TRANS  - (input) char*
             On entry, TRANS specifies the operation to be performed as
             follows:
                TRANS = 'N' or 'n'   y := alpha*A*x + beta*y.
                TRANS = 'T' or 't'   y := alpha*A'*x + beta*y.
                TRANS = 'C' or 'c'   y := alpha*A'*x + beta*y.

    M      - (input) int_t   
             On entry,  M  specifies  the number of rows of the matrix 
	     op( A ) and of the matrix C.  M must be at least zero. 
	     Unchanged on exit.   

    N      - (input) int_t
             On entry,  N specifies the number of columns of the matrix 
	     op( B ) and the number of columns of the matrix C. N must be 
	     at least zero.
	     Unchanged on exit.   

    K      - (input) int_t
             On entry, K specifies the number of columns of the matrix 
	     op( A ) and the number of rows of the matrix op( B ). K must 
	     be at least  zero.   
             Unchanged on exit.
	     
    ALPHA  - (input) float
             On entry, ALPHA specifies the scalar alpha.   

    A      - (input) SuperMatrix*
             Matrix A with a sparse format, of dimension (A->nrow, A->ncol).
             Currently, the type of A can be:
                 Stype = NC or NCP; Dtype = SLU_S; Mtype = GE. 
             In the future, more general A can be handled.

    B      - FLOAT PRECISION array of DIMENSION ( LDB, kb ), where kb is 
             n when TRANSB = 'N' or 'n',  and is  k otherwise.   
             Before entry with  TRANSB = 'N' or 'n',  the leading k by n 
             part of the array B must contain the matrix B, otherwise 
             the leading n by k part of the array B must contain the 
             matrix B.   
             Unchanged on exit.   

    LDB    - (input) int_t
             On entry, LDB specifies the first dimension of B as declared 
             in the calling (sub) program. LDB must be at least max( 1, n ).  
             Unchanged on exit.   

    BETA   - (input) float
             On entry, BETA specifies the scalar beta. When BETA is   
             supplied as zero then C need not be set on input.   

    C      - FLOAT PRECISION array of DIMENSION ( LDC, n ).   
             Before entry, the leading m by n part of the array C must 
             contain the matrix C,  except when beta is zero, in which 
             case C need not be set on entry.   
             On exit, the array C is overwritten by the m by n matrix 
	     ( alpha*op( A )*B + beta*C ).   

    LDC    - (input) int_t
             On entry, LDC specifies the first dimension of C as declared 
             in the calling (sub)program. LDC must be at least max(1,m).   
             Unchanged on exit.   

    ==== Sparse Level 3 Blas routine.   
*/
    int    incx = 1, incy = 1;
    int    j;

    for (j = 0; j < n; ++j) {
	sp_sgemv(trans, alpha, A, &b[ldb*j], incx, beta, &c[ldc*j], incy);
    }
    return 0;    
}
