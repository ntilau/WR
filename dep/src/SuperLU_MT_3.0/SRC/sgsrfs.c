
#include <math.h>
#include "slu_mt_sdefs.h"


void
sgsrfs(trans_t trans, SuperMatrix *A, SuperMatrix *L, SuperMatrix *U,
       int_t *perm_r, int_t *perm_c, equed_t equed, float *R, float *C,
       SuperMatrix *B, SuperMatrix *X, float *ferr, float *berr,
       Gstat_t *Gstat, int_t *info)
{
/*
 * -- SuperLU MT routine (version 2.0) --
 * Lawrence Berkeley National Lab, Univ. of California Berkeley,
 * and Xerox Palo Alto Research Center.
 * September 10, 2007
 *
 *
 * Purpose
 * =======   
 *
 * sgsrfs improves the computed solution to a system of linear
 * equations and provides error bounds and backward error estimates for
 * the solution.
 *
 * See supermatrix.h for the definition of 'SuperMatrix' structure.
 *
 * Arguments
 * =========
 *
 * trans   (input) trans_t
 *         Specifies the form of the system of equations:
 *         = NOTRANS:  A * X = B     (No transpose)
 *         = TRANS:    A**T * X = B  (Transpose)
 *         = CONJ:     A**H * X = B  (Conjugate transpose = Transpose)
 *
 * A       (input) SuperMatrix*
 *         The original matrix A in the system, or the scaled A if
 *         equilibration was done. The type of A can be:
 *         Stype = NC, Dtype = _D, Mtype = GE.
 *
 * L       (input) SuperMatrix*
 *         The factor L from the factorization Pr*A*Pc=L*U. Use
 *         compressed row subscripts storage for supernodes,
 *         i.e., L has types: Stype = SCP, Dtype = _D, Mtype = TRLU.
 *
 * U       (input) SuperMatrix*
 *         The factor U from the factorization Pr*A*Pc=L*U as computed by
 *         dgstrf(). Use column-wise storage scheme,
 *         i.e., U has types: Stype = NCP, Dtype = _D, Mtype = TRU.
 *
 * perm_r  (input) int_t*, dimension (A->nrow)
 *         Row permutation vector, which defines the permutation matrix Pr;
 *         perm_r[i] = j means row i of A is in position j in Pr*A.
 *
 * perm_c  (input) int_t*, dimension (A->ncol)
 *         Column permutation vector, which defines the
 *         permutation matrix Pc; perm_c[i] = j means column i of A is 
 *         in position j in A*Pc.
 *
 * equed   (input) equed_t
 *         Specifies the form of equilibration that was done.
 *         = NOEQUIL: No equilibration.
 *         = ROW:  Row equilibration, i.e., A was premultiplied by diag(R).
 *         = COL:  Column equilibration, i.e., A was postmultiplied by
 *                 diag(C).
 *         = BOTH: Both row and column equilibration, i.e., A was replaced
 *                 by diag(R)*A*diag(C).
 *
 * R       (input) double*, dimension (A->nrow)
 *         The row scale factors for A.
 *         If equed = ROW or BOTH, A is premultiplied by diag(R).
 *         If equed = NOEQUIL or COL, R is not accessed.
 *
 * C       (input) double*, dimension (A->ncol)
 *         The column scale factors for A.
 *         If equed = COL or BOTH, A is postmultiplied by diag(C).
 *         If equed = NOEQUIL or ROW, C is not accessed.
 *
 * B       (input) SuperMatrix*
 *         B has types: Stype = DN, Dtype = _D, Mtype = GE.
 *         The right hand side matrix B.
 *
 * X       (input/output) SuperMatrix*
 *         X has types: Stype = DN, Dtype = _D, Mtype = GE.
 *         On entry, the solution matrix X, as computed by dgstrs().
 *         On exit, the improved solution matrix X.
 *
 * FERR    (output) double*, dimension (B->ncol)
 *         The estimated forward error bound for each solution vector
 *         X(j) (the j-th column of the solution matrix X).
 *         If XTRUE is the true solution corresponding to X(j), FERR(j)
 *         is an estimated upper bound for the magnitude of the largest
 *         element in (X(j) - XTRUE) divided by the magnitude of the
 *         largest element in X(j).  The estimate is as reliable as
 *         the estimate for RCOND, and is almost always a slight
 *         overestimate of the true error.
 *
 * BERR    (output) double*, dimension (B->ncol)
 *         The componentwise relative backward error of each solution
 *         vector X(j) (i.e., the smallest relative change in
 *         any element of A or B that makes X(j) an exact solution).
 *
 * info    (output) int_t*
 *         = 0:  successful exit
 *         < 0:  if INFO = -i, the i-th argument had an illegal value
 *
 * Internal Parameters
 * ===================
 *
 * ITMAX is the maximum number of steps of iterative refinement.
 *
 */

#define ITMAX 5
    
    /* Table of constant values */
    int    ione = 1, nrow = A->nrow;
    float ndone = -1.;
    float done = 1.;
    
    /* Local variables */
    NCformat *Astore;
    float   *Aval;
    SuperMatrix Bjcol;
    DNformat *Bstore, *Xstore, *Bjcol_store;
    float   *Bmat, *Xmat, *Bptr, *Xptr;
    int_t      kase;
    float   safe1, safe2;
    int_t      j, k, irow, nz, count, notran, rowequ, colequ;
    int        i;
    int_t      ldb, ldx, nrhs;
    float   s, xk, lstres, eps, safmin;
    char     transc[1];
    trans_t  transt;
    float   *work;
    float   *rwork;
    int_t      *iwork;
    extern double slamch_(char *);
    extern int_t slacon_(int_t *, float *, float *, int_t *, float *, int_t *);
#ifdef _CRAY
    extern int SCOPY(int *, float *, int *, float *, int *);
    extern int SSAXPY(int *, float *, float *, int *, float *, int *);
#else
    extern int scopy_(int *, float *, int *, float *, int *);
    extern int saxpy_(int *, float *, float *, int *, float *, int *);
#endif

    Astore = A->Store;
    Aval   = Astore->nzval;
    Bstore = B->Store;
    Xstore = X->Store;
    Bmat   = Bstore->nzval;
    Xmat   = Xstore->nzval;
    ldb    = Bstore->lda;
    ldx    = Xstore->lda;
    nrhs   = B->ncol;
    
    /* Test the input parameters */
    *info = 0;
    notran = (trans == NOTRANS);
    if ( !notran && trans != TRANS && trans != CONJ ) *info = -1;
    else if ( A->nrow != A->ncol || A->nrow < 0 ||
	      A->Stype != SLU_NC || A->Dtype != SLU_S || A->Mtype != SLU_GE )
	*info = -2;
    else if ( L->nrow != L->ncol || L->nrow < 0 ||
 	      L->Stype != SLU_SCP || L->Dtype != SLU_S || L->Mtype != SLU_TRLU )
	*info = -3;
    else if ( U->nrow != U->ncol || U->nrow < 0 ||
 	      U->Stype != SLU_NCP || U->Dtype != SLU_S || U->Mtype != SLU_TRU )
	*info = -4;
    else if ( ldb < SUPERLU_MAX(0, A->nrow) ||
 	      B->Stype != SLU_DN || B->Dtype != SLU_S || B->Mtype != SLU_GE )
        *info = -10;
    else if ( ldx < SUPERLU_MAX(0, A->nrow) ||
 	      X->Stype != SLU_DN || X->Dtype != SLU_S || X->Mtype != SLU_GE )
	*info = -11;
    if (*info != 0) {
	i = -(*info);
	xerbla_("sgsrfs", &i);
	return;
    }

    /* Quick return if possible */
    if ( A->nrow == 0 || nrhs == 0) {
	for (j = 0; j < nrhs; ++j) {
	    ferr[j] = 0.;
	    berr[j] = 0.;
	}
	return;
    }

    rowequ = (equed == ROW) || (equed == BOTH);
    colequ = (equed == COL) || (equed == BOTH);
    
    /* Allocate working space */
    work = floatMalloc(2*A->nrow);
    rwork = (float *) SUPERLU_MALLOC( (size_t) A->nrow * sizeof(float) );
    iwork = intMalloc(2*A->nrow);
    if ( !work || !rwork || !iwork ) 
        SUPERLU_ABORT("Malloc fails for work/rwork/iwork.");
    
    if ( notran ) {
	*(unsigned char *)transc = 'N';
        transt = TRANS;
    } else {
	*(unsigned char *)transc = 'T';
	transt = NOTRANS;
    }

    /* NZ = maximum number of nonzero elements in each row of A, plus 1 */
    nz     = A->ncol + 1;
    eps    = slamch_("Epsilon");
    safmin = slamch_("Safe minimum");
    /* Set SAFE1 essentially to be the underflow threshold times the
       number of additions in each row. */
    safe1  = nz * safmin;
    safe2  = safe1 / eps;

    /* Compute the number of nonzeros in each row (or column) of A */
    for (i = 0; i < A->nrow; ++i) iwork[i] = 0;
    if ( notran ) {
	for (k = 0; k < A->ncol; ++k)
	    for (i = Astore->colptr[k]; i < Astore->colptr[k+1]; ++i) 
		++iwork[Astore->rowind[i]];
    } else {
	for (k = 0; k < A->ncol; ++k)
	    iwork[k] = Astore->colptr[k+1] - Astore->colptr[k];
    }	

    /* Copy one column of RHS B into Bjcol. */
    Bjcol.Stype = B->Stype;
    Bjcol.Dtype = B->Dtype;
    Bjcol.Mtype = B->Mtype;
    Bjcol.nrow  = B->nrow;
    Bjcol.ncol  = 1;
    Bjcol.Store = (void *) SUPERLU_MALLOC( sizeof(DNformat) );
    if ( !Bjcol.Store ) SUPERLU_ABORT("SUPERLU_MALLOC fails for Bjcol.Store");
    Bjcol_store = Bjcol.Store;
    Bjcol_store->lda = ldb;
    Bjcol_store->nzval = work; /* address aliasing */
	
    /* Do for each right hand side ... */
    for (j = 0; j < nrhs; ++j) {
	count = 0;
	lstres = 3.;
	Bptr = &Bmat[j*ldb];
	Xptr = &Xmat[j*ldx];

	while (1) { /* Loop until stopping criterion is satisfied. */

	    /* Compute residual R = B - op(A) * X,   
	       where op(A) = A, A**T, or A**H, depending on TRANS. */
	    
#ifdef _CRAY
	    SCOPY(&nrow, Bptr, &ione, work, &ione);
#else
	    scopy_(&nrow, Bptr, &ione, work, &ione);
#endif
	    sp_sgemv(transc, ndone, A, Xptr, ione, done, work, ione);

	    /* Compute componentwise relative backward error from formula 
	       max(i) ( abs(R(i)) / ( abs(op(A))*abs(X) + abs(B) )(i) )   
	       where abs(Z) is the componentwise absolute value of the matrix
	       or vector Z.  If the i-th component of the denominator is less
	       than SAFE2, then SAFE1 is added to the i-th component of the   
	       numerator before dividing. */

	    for (i = 0; i < A->nrow; ++i) rwork[i] = fabs( Bptr[i] );
	    
	    /* Compute abs(op(A))*abs(X) + abs(B). */
	    if (notran) {
		for (k = 0; k < A->ncol; ++k) {
		    xk = fabs( Xptr[k] );
		    for (i = Astore->colptr[k]; i < Astore->colptr[k+1]; ++i)
			rwork[Astore->rowind[i]] += fabs(Aval[i]) * xk;
		}
	    } else {
		for (k = 0; k < A->ncol; ++k) {
		    s = 0.;
		    for (i = Astore->colptr[k]; i < Astore->colptr[k+1]; ++i) {
			irow = Astore->rowind[i];
			s += fabs(Aval[i]) * fabs(Xptr[irow]);
		    }
		    rwork[k] += s;
		}
	    }
	    s = 0.;
	    for (i = 0; i < A->nrow; ++i) {
		if (rwork[i] > safe2) {
		    s = SUPERLU_MAX( s, fabs(work[i]) / rwork[i] );
		} else if ( rwork[i] != 0.0 ) {
		    s = SUPERLU_MAX( s, (fabs(work[i]) + safe1) / rwork[i] );
                }
                /* If rwork[i] is exactly 0.0, then we know the true 
                   residual also must be exactly 0.0. */
	    }
	    berr[j] = s;

	    /* Test stopping criterion. Continue iterating if   
	       1) The residual BERR(J) is larger than machine epsilon, and   
	       2) BERR(J) decreased by at least a factor of 2 during the   
	          last iteration, and   
	       3) At most ITMAX iterations tried. */

	    if (berr[j] > eps && berr[j] * 2. <= lstres && count < ITMAX) {
		/* Update solution and try again. */
		sgstrs (trans, L, U, perm_r, perm_c, &Bjcol, Gstat, info);
		
#ifdef _CRAY
		SAXPY(&nrow, &done, work, &ione,
		       &Xmat[j*ldx], &ione);
#else
		saxpy_(&nrow, &done, work, &ione,
		       &Xmat[j*ldx], &ione);
#endif
		lstres = berr[j];
		++count;
	    } else {
		break;
	    }
        
	} /* end while */

	/* Bound error from formula:
	   norm(X - XTRUE) / norm(X) .le. FERR = norm( abs(inv(op(A)))*   
	   ( abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) / norm(X)   
          where   
            norm(Z) is the magnitude of the largest component of Z   
            inv(op(A)) is the inverse of op(A)   
            abs(Z) is the componentwise absolute value of the matrix or
	       vector Z   
            NZ is the maximum number of nonzeros in any row of A, plus 1   
            EPS is machine epsilon   

          The i-th component of abs(R)+NZ*EPS*(abs(op(A))*abs(X)+abs(B))   
          is incremented by SAFE1 if the i-th component of   
          abs(op(A))*abs(X) + abs(B) is less than SAFE2.   

          Use SLACON to estimate the infinity-norm of the matrix   
             inv(op(A)) * diag(W),   
          where W = abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) */
	
	for (i = 0; i < A->nrow; ++i) rwork[i] = fabs( Bptr[i] );
	
	/* Compute abs(op(A))*abs(X) + abs(B). */
	if ( notran ) {
	    for (k = 0; k < A->ncol; ++k) {
		xk = fabs( Xptr[k] );
		for (i = Astore->colptr[k]; i < Astore->colptr[k+1]; ++i)
		    rwork[Astore->rowind[i]] += fabs(Aval[i]) * xk;
	    }
	} else {
	    for (k = 0; k < A->ncol; ++k) {
		s = 0.;
		for (i = Astore->colptr[k]; i < Astore->colptr[k+1]; ++i) {
		    irow = Astore->rowind[i];
		    xk = fabs( Xptr[irow] );
		    s += fabs(Aval[i]) * xk;
		}
		rwork[k] += s;
	    }
	}
	
	for (i = 0; i < A->nrow; ++i)
	    if (rwork[i] > safe2)
		rwork[i] = fabs(work[i]) + (iwork[i]+1)*eps*rwork[i];
	    else
		rwork[i] = fabs(work[i])+(iwork[i]+1)*eps*rwork[i]+safe1;

	kase = 0;

	do {
	    slacon_(&A->nrow, &work[A->nrow], work,
		    &iwork[A->nrow], &ferr[j], &kase);
	    if (kase == 0) break;

	    if (kase == 1) {
		/* Multiply by diag(W)*inv(op(A)**T)*(diag(C) or diag(R)). */
		if ( notran && colequ )
		    for (i = 0; i < A->ncol; ++i) work[i] *= C[i];
		else if ( !notran && rowequ )
		    for (i = 0; i < A->nrow; ++i) work[i] *= R[i];
		
		sgstrs (transt, L, U, perm_r, perm_c, &Bjcol, Gstat, info);
		
		for (i = 0; i < A->nrow; ++i) work[i] *= rwork[i];
	    } else {
		/* Multiply by (diag(C) or diag(R))*inv(op(A))*diag(W). */
		for (i = 0; i < A->nrow; ++i) work[i] *= rwork[i];
		
		sgstrs (trans, L, U, perm_r, perm_c, &Bjcol, Gstat, info);
		
		if ( notran && colequ )
		    for (i = 0; i < A->ncol; ++i) work[i] *= C[i];
		else if ( !notran && rowequ )
		    for (i = 0; i < A->ncol; ++i) work[i] *= R[i];  
	    }
	    
	} while ( kase != 0 );


	/* Normalize error. */
	lstres = 0.;
 	if ( notran && colequ ) {
	    for (i = 0; i < A->nrow; ++i)
	    	lstres = SUPERLU_MAX( lstres, C[i] * fabs( Xptr[i]) );
  	} else if ( !notran && rowequ ) {
	    for (i = 0; i < A->nrow; ++i)
	    	lstres = SUPERLU_MAX( lstres, R[i] * fabs( Xptr[i]) );
	} else {
	    for (i = 0; i < A->nrow; ++i)
	    	lstres = SUPERLU_MAX( lstres, fabs( Xptr[i]) );
	}
	if ( lstres != 0. )
	    ferr[j] /= lstres;

    } /* for each RHS j ... */
    
    SUPERLU_FREE(work);
    SUPERLU_FREE(rwork);
    SUPERLU_FREE(iwork);
    SUPERLU_FREE(Bjcol.Store);

    return;

} /* sgsrfs */
