
#include "slu_mt_sdefs.h"

void
sgstrs(trans_t trans, SuperMatrix *L, SuperMatrix *U, 
       int_t *perm_r, int_t *perm_c, SuperMatrix *B, Gstat_t *Gstat, int_t *info)
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
 * sgstrs() solves a system of linear equations A*X=B or A'*X=B
 * with A sparse and B dense, using the LU factorization computed by
 * psgstrf().
 *
 * Arguments
 * =========
 *
 * trans   (input) Specifies the form of the system of equations:
 *          = NOTRANS: A * X = B  (No transpose)
 *          = TRANS:   A'* X = B  (Transpose)
 *
 * L       (input) SuperMatrix*
 *         The factor L from the factorization Pr*A*Pc=L*U as computed by
 *         psgstrf(). Use compressed row subscripts storage for supernodes,
 *         i.e., L has types: Stype = SCP, Dtype = _D, Mtype = TRLU.
 *
 * U       (input) SuperMatrix*
 *         The factor U from the factorization Pr*A*Pc=L*U as computed by
 *         psgstrf(). Use column-wise storage scheme, i.e., U has types:
 *         Stype = NCP, Dtype = _D, Mtype = TRU.
 *
 * perm_r  (input) int_t*
 *         Row permutation vector of size L->nrow, which defines the
 *         permutation matrix Pr; perm_r[i] = j means row i of A is in
 *         position j in Pr*A.
 *
 * perm_c  (int_t*) dimension A->ncol
 *	   Column permutation vector, which defines the 
 *         permutation matrix Pc; perm_c[i] = j means column i of A is 
 *         in position j in A*Pc.
 *
 * B       (input/output) SuperMatrix*
 *         B has types: Stype = DN, Dtype = _D, Mtype = GE.
 *         On entry, the right hand side matrix.
 *         On exit, the solution matrix if info = 0;
 *
 * Gstat   (output) Gstat_t*
 *          Record all the statistics about the triangular solves; 
 *          See Gstat_t structure defined in slu_mt_util.h.
 *
 * info    (output) Diagnostics
 * 	   = 0: successful exit
 *	   < 0: if info = -i, the i-th argument had an illegal value
 *
 */
#if ( MACH==CRAY_PVP )
    _fcd ftcs1, ftcs2, ftcs3, ftcs4;
#endif

#ifdef USE_VENDOR_BLAS
    int      incx = 1, incy = 1;
    float   alpha = 1.0, beta = 1.0;
#endif

    register int_t j, k, jcol, iptr, luptr, ksupno, istart, irow, bptr;
    register int_t fsupc, nsuper;
    int        i, n, nsupc, nsupr, nrow, nrhs, ldb;
    int_t      *supno;
    DNformat *Bstore;
    SCPformat *Lstore;
    NCPformat *Ustore;
    float   *Lval, *Uval, *Bmat;
    float   *work, *work_col, *rhs_work, *soln;
    flops_t  solve_ops;
    void sprint_soln();

    /* Test input parameters ... */
    *info = 0;
    Bstore = B->Store;
    ldb = Bstore->lda;
    nrhs = B->ncol;
    if ( trans != NOTRANS && trans != TRANS ) *info = -1;
    else if ( L->nrow != L->ncol || L->nrow < 0 ) *info = -3;
    else if ( U->nrow != U->ncol || U->nrow < 0 ) *info = -4;
    else if ( ldb < SUPERLU_MAX(0, L->nrow) ) *info = -6;
    if ( *info ) {
        i = -(*info);
	xerbla_("sgstrs", &i);
	return;
    }

    n = L->nrow;
    work = floatCalloc(n * nrhs);
    if ( !work ) SUPERLU_ABORT("Malloc fails for local work[].");
    soln = floatMalloc(n);
    if ( !soln ) SUPERLU_ABORT("Malloc fails for local soln[].");

    Bmat = Bstore->nzval;
    Lstore = L->Store;
    Lval = Lstore->nzval;
    Ustore = U->Store;
    Uval = Ustore->nzval;
    supno = Lstore->col_to_sup;
    nsuper = Lstore->nsuper;
    solve_ops = 0;
    
    if ( trans == NOTRANS ) {
	/* Permute right hand sides to form Pr*B */
	for (i = 0, bptr = 0; i < nrhs; i++, bptr += ldb) {
	    rhs_work = &Bmat[bptr];
	    for (k = 0; k < n; k++) soln[perm_r[k]] = rhs_work[k];
	    for (k = 0; k < n; k++) rhs_work[k] = soln[k];
	}
	
	/* Forward solve PLy=Pb. */
/*>>	for (k = 0; k < n; k += nsupc) {
	    ksupno = supno[k];
*/
	for (ksupno = 0; ksupno <= nsuper; ++ksupno) {
	    fsupc = L_FST_SUPC(ksupno);
	    istart = L_SUB_START(fsupc);
	    nsupr = L_SUB_END(fsupc) - istart;
	    nsupc = L_LAST_SUPC(ksupno) - fsupc;
	    nrow = nsupr - nsupc;

	    solve_ops += nsupc * (nsupc - 1) * nrhs;
	    solve_ops += 2 * nrow * nsupc * nrhs;
	    
	    if ( nsupc == 1 ) {
		for (j = 0, bptr = 0; j < nrhs; j++, bptr += ldb) {
		    rhs_work = &Bmat[bptr];
	    	    luptr = L_NZ_START(fsupc);
		    for (iptr=istart+1; iptr < L_SUB_END(fsupc); iptr++){
			irow = L_SUB(iptr);
			++luptr;
                        rhs_work[irow] -= rhs_work[fsupc] * Lval[luptr];
		    }
		}
	    } else {
	    	luptr = L_NZ_START(fsupc);
#ifdef USE_VENDOR_BLAS
#if ( MACH==CRAY_PVP )
		ftcs1 = _cptofcd("L", strlen("L"));
		ftcs2 = _cptofcd("N", strlen("N"));
		ftcs3 = _cptofcd("U", strlen("U"));
 		STRSM(ftcs1, ftcs1, ftcs2, ftcs3, &nsupc, &nrhs, &alpha,
		      &Lval[luptr], &nsupr, &Bmat[fsupc], &ldb);
		
		SGEMM(ftcs2, ftcs2,  &nrow, &nrhs, &nsupc, &alpha, 
		      &Lval[luptr+nsupc], &nsupr, &Bmat[fsupc], &ldb, 
		      &beta, &work[0], &n );
#else
 		strsm_("L", "L", "N", "U", &nsupc, &nrhs, &alpha,
		       &Lval[luptr], &nsupr, &Bmat[fsupc], &ldb);
		
		sgemm_( "N", "N", &nrow, &nrhs, &nsupc, &alpha, 
			&Lval[luptr+nsupc], &nsupr, &Bmat[fsupc], &ldb, 
			&beta, &work[0], &n );
#endif
		for (j = 0, bptr = 0; j < nrhs; j++, bptr += ldb) {
		    rhs_work = &Bmat[bptr];
		    work_col = &work[j*n];
		    iptr = istart + nsupc;
		    for (i = 0; i < nrow; i++) {
			irow = L_SUB(iptr);
                        rhs_work[irow] -= work_col[i]; /* Scatter */
                        work_col[i] = 0.0;
			iptr++;
		    }
		}
#else		
		for (j = 0, bptr = 0; j < nrhs; j++, bptr += ldb) {
		    rhs_work = &Bmat[bptr];
		    slsolve ((int_t)nsupr, (int_t)nsupc, &Lval[luptr], &rhs_work[fsupc]);
		    smatvec ((int_t)nsupr, (int_t)nrow, (int_t)nsupc, &Lval[luptr+nsupc],
			     &rhs_work[fsupc], &work[0] );

		    iptr = istart + nsupc;
		    for (i = 0; i < nrow; i++) {
			irow = L_SUB(iptr);
                        rhs_work[irow] -= work[i];
                        work[i] = 0.0;
			iptr++;
		    }
		}
#endif		    
	    } /* if-else: nsupc == 1 ... */
	} /* for L-solve */

#if ( DEBUGlevel>=2 )
  	printf("After L-solve: y=\n");
	sprint_soln(n, nrhs, Bmat);
#endif

	/*
	 * Back solve Ux=y.
	 */
/*>>	for (k = n-1; k >= 0; k -= nsupc) {
	    ksupno = supno[k];
*/
	for (ksupno = nsuper; ksupno >= 0; --ksupno) {
	    fsupc = L_FST_SUPC(ksupno);
	    istart = L_SUB_START(fsupc);
	    nsupr = L_SUB_END(fsupc) - istart;
	    nsupc = L_LAST_SUPC(ksupno) - fsupc;
	    luptr = L_NZ_START(fsupc);

	    solve_ops += nsupc * (nsupc + 1) * nrhs;

	    /* dense triangular matrix */
	    if ( nsupc == 1 ) {
		rhs_work = &Bmat[0];
		for (j = 0; j < nrhs; j++) {
                    rhs_work[fsupc] /= Lval[luptr];
		    rhs_work += ldb;
		}
	    } else {
#ifdef USE_VENDOR_BLAS
#if ( MACH==CRAY_PVP )
		ftcs1 = _cptofcd("L", strlen("L"));
		ftcs2 = _cptofcd("U", strlen("U"));
		ftcs3 = _cptofcd("N", strlen("N"));
		STRSM(ftcs1, ftcs2, ftcs3, ftcs3, &nsupc, &nrhs, &alpha,
		      &Lval[luptr], &nsupr, &Bmat[fsupc], &ldb);
#else
		strsm_("L", "U", "N", "N", &nsupc, &nrhs, &alpha,
		       &Lval[luptr], &nsupr, &Bmat[fsupc], &ldb);
#endif
#else		
		for (j = 0, bptr = fsupc; j < nrhs; j++, bptr += ldb) {
		    susolve (nsupr, nsupc, &Lval[luptr], &Bmat[bptr]);
		}
#endif		
	    }

	    /* matrix-vector update */
	    for (j = 0, bptr = 0; j < nrhs; ++j, bptr += ldb) {
		rhs_work = &Bmat[bptr];
		for (jcol = fsupc; jcol < fsupc + nsupc; jcol++) {
                    solve_ops += 2*(U_NZ_END(jcol) - U_NZ_START(jcol));
		    for (i = U_NZ_START(jcol); i < U_NZ_END(jcol); i++ ){
			irow = U_SUB(i);
                        rhs_work[irow] -= rhs_work[jcol] * Uval[i];
		    }
		}
	    }
	    
	} /* for U-solve */

#if ( DEBUGlevel>=2 )
  	printf("After U-solve: x=\n");
	sprint_soln(n, nrhs, Bmat);
#endif

	/* Compute the final solution X <= Pc*X. */
	for (i = 0, bptr = 0; i < nrhs; i++, bptr += ldb) {
	    rhs_work = &Bmat[bptr];
	    for (k = 0; k < n; k++) soln[k] = rhs_work[perm_c[k]];
	    for (k = 0; k < n; k++) rhs_work[k] = soln[k];
	}
	
    } else { /* Solve A'*X=B */
	/* Permute right hand sides to form Pc'*B. */
	for (i = 0, bptr = 0; i < nrhs; i++, bptr += ldb) {
	    rhs_work = &Bmat[bptr];
	    for (k = 0; k < n; k++) soln[perm_c[k]] = rhs_work[k];
	    for (k = 0; k < n; k++) rhs_work[k] = soln[k];
	}
	
        for (k = 0; k < nrhs; ++k) {

            /* Multiply by inv(U'). */
            sp_strsv("U", "T", "N", L, U, &Bmat[k*ldb], info);

            /* Multiply by inv(L'). */
            sp_strsv("L", "T", "U", L, U, &Bmat[k*ldb], info);

        }
	/* Compute the final solution X <= Pr'*X (=inv(Pr)*X) */
	for (i = 0, bptr = 0; i < nrhs; i++, bptr += ldb) {
	    rhs_work = &Bmat[bptr];
	    for (k = 0; k < n; k++) soln[k] = rhs_work[perm_r[k]];
	    for (k = 0; k < n; k++) rhs_work[k] = soln[k];
	}

    } /* if-else trans */

    Gstat->ops[TRISOLVE] = solve_ops;
    SUPERLU_FREE(work);
    SUPERLU_FREE(soln);
}

/*
 * Diagnostic print of the solution vector
 */
void
sprint_soln(int_t n, int_t nrhs, float *soln)
{
    int_t i;

    for (i = 0; i < n; i++)
	printf("\t" IFMT ": %.10f\n", i, soln[i]);
}
