
/*
 * -- SuperLU routine (version 3.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * October 15, 2003
 *
 */
/*
 * File name:		zsp_blas2.c
 * Purpose:		Sparse BLAS 2, using some dense BLAS 2 operations.
 */

#include "slu_mt_zdefs.h"


/* 
 * Function prototypes 
 */
extern void zusolve(int_t, int_t, doublecomplex*, doublecomplex*);
extern void zlsolve(int_t, int_t, doublecomplex*, doublecomplex*);
extern void zmatvec(int_t, int_t, int_t, doublecomplex*, doublecomplex*, doublecomplex*);


int_t
sp_ztrsv(char *uplo, char *trans, char *diag, SuperMatrix *L, 
         SuperMatrix *U, doublecomplex *x, int_t *info)
{
/*
 *   Purpose
 *   =======
 *
 *   sp_ztrsv() solves one of the systems of equations   
 *       A*x = b,   or   A'*x = b,
 *   where b and x are n element vectors and A is a sparse unit , or   
 *   non-unit, upper or lower triangular matrix.   
 *   No test for singularity or near-singularity is included in this   
 *   routine. Such tests must be performed before calling this routine.   
 *
 *   Parameters   
 *   ==========   
 *
 *   uplo   - (input) char*
 *            On entry, uplo specifies whether the matrix is an upper or   
 *             lower triangular matrix as follows:   
 *                uplo = 'U' or 'u'   A is an upper triangular matrix.   
 *                uplo = 'L' or 'l'   A is a lower triangular matrix.   
 *
 *   trans  - (input) char*
 *             On entry, trans specifies the equations to be solved as   
 *             follows:   
 *                trans = 'N' or 'n'   A*x = b.   
 *                trans = 'T' or 't'   A'*x = b.
 *                trans = 'C' or 'c'   A^H*x = b.   
 *
 *   diag   - (input) char*
 *             On entry, diag specifies whether or not A is unit   
 *             triangular as follows:   
 *                diag = 'U' or 'u'   A is assumed to be unit triangular.   
 *                diag = 'N' or 'n'   A is not assumed to be unit   
 *                                    triangular.   
 *	     
 *   L       - (input) SuperMatrix*
 *	       The factor L from the factorization Pr*A*Pc=L*U. Use
 *             compressed row subscripts storage for supernodes,
 *             i.e., L has types: Stype = SC, Dtype = _Z, Mtype = TRLU.
 *
 *   U       - (input) SuperMatrix*
 *	        The factor U from the factorization Pr*A*Pc=L*U.
 *	        U has types: Stype = NCP, Dtype = _Z, Mtype = TRU.
 *    
 *   x       - (input/output) doublecomplex*
 *             Before entry, the incremented array X must contain the n   
 *             element right-hand side vector b. On exit, X is overwritten 
 *             with the solution vector x.
 *
 *   info    - (output) int_t*
 *             If *info = -i, the i-th argument had an illegal value.
 *
 */
#if ( MACH==CRAY_PVP )
    _fcd ftcs1, ftcs2, ftcs3;
#endif
    SCPformat *Lstore;
    NCPformat *Ustore;
    doublecomplex   *Lval, *Uval;
    int incx = 1, incy = 1;
    doublecomplex temp;
    doublecomplex alpha = {1.0, 0.0}, beta = {1.0, 0.0};
    doublecomplex comp_zero = {0.0, 0.0};
    register int_t fsupc, luptr, istart, irow, k, iptr, jcol, nsuper;
    int          nsupr, nsupc, nrow, i;
    doublecomplex *work;
    flops_t solve_ops;

    /* Test the input parameters */
    *info = 0;
    if ( !lsame_(uplo,"L") && !lsame_(uplo, "U") ) *info = -1;
    else if ( !lsame_(trans, "N") && !lsame_(trans, "T") ) *info = -2;
    else if ( !lsame_(diag, "U") && !lsame_(diag, "N") ) *info = -3;
    else if ( L->nrow != L->ncol || L->nrow < 0 ) *info = -4;
    else if ( U->nrow != U->ncol || U->nrow < 0 ) *info = -5;
    if ( *info ) {
	i = -(*info);
	xerbla_("sp_ztrsv", &i);
	return 0;
    }

    Lstore = (SCPformat*) L->Store;
    Lval = (doublecomplex*) Lstore->nzval;
    Ustore = (NCPformat*) U->Store;
    Uval = (doublecomplex*) Ustore->nzval;
    nsuper = Lstore->nsuper;
    solve_ops = 0;

    if ( !(work = doublecomplexCalloc(L->nrow)) )
	SUPERLU_ABORT("Malloc fails for work in sp_ztrsv().");
    
    if ( lsame_(trans, "N") ) {	/* Form x := inv(A)*x. */
	
	if ( lsame_(uplo, "L") ) {
	    /* Form x := inv(L)*x */
    	    if ( L->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = 0; k <= nsuper; k++) {
		fsupc = L_FST_SUPC(k);
		istart = L_SUB_START(fsupc);
                nsupr = L_SUB_END(fsupc) - istart;
                nsupc = L_LAST_SUPC(k) - fsupc;
		luptr = L_NZ_START(fsupc);
		nrow = nsupr - nsupc;

                /* 1 z_div costs 10 flops */
	        solve_ops += 4 * nsupc * (nsupc - 1) + 10 * nsupc;
	        solve_ops += 8 * nrow * nsupc;

		if ( nsupc == 1 ) {
		    for (iptr=istart+1; iptr < L_SUB_END(fsupc); ++iptr) {
			irow = L_SUB(iptr);
			++luptr;
			zz_mult(&comp_zero, &x[fsupc], &Lval[luptr]);
			z_sub(&x[irow], &x[irow], &comp_zero);
		    }
		} else {
#ifdef USE_VENDOR_BLAS
#if ( MACH==CRAY_PVP )
                    ftcs1 = _cptofcd("L", strlen("L"));
                    ftcs2 = _cptofcd("N", strlen("N"));
                    ftcs3 = _cptofcd("U", strlen("U"));

		    CTRSV(ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
		       	&x[fsupc], &incx);
		
		    CGEMV(ftcs2, &nrow, &nsupc, &alpha, &Lval[luptr+nsupc], 
		       	&nsupr, &x[fsupc], &incx, &beta, &work[0], &incy);
#else
		    ztrsv_("L", "N", "U", &nsupc, &Lval[luptr], &nsupr,
		       	&x[fsupc], &incx);
		
		    zgemv_("N", &nrow, &nsupc, &alpha, &Lval[luptr+nsupc], 
		       	&nsupr, &x[fsupc], &incx, &beta, &work[0], &incy);
#endif
#else
		    zlsolve (nsupr, nsupc, &Lval[luptr], &x[fsupc]);
		
		    zmatvec (nsupr, nsupr-nsupc, nsupc, &Lval[luptr+nsupc],
                             &x[fsupc], &work[0] );
#endif		
		
		    iptr = istart + nsupc;
		    for (i = 0; i < nrow; ++i, ++iptr) {
			irow = L_SUB(iptr);
			z_sub(&x[irow], &x[irow], &work[i]); /* Scatter */
			work[i] = comp_zero;

		    }
	 	}
	    } /* for k ... */
	    
	} else {
	    /* Form x := inv(U)*x */
	    
	    if ( U->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = nsuper; k >= 0; k--) {
	    	fsupc = L_FST_SUPC(k);
                nsupr = L_SUB_END(fsupc) - L_SUB_START(fsupc);
                nsupc = L_LAST_SUPC(k) - fsupc;
	    	luptr = L_NZ_START(fsupc);
		
                /* 1 z_div costs 10 flops */
    	        solve_ops += 4 * nsupc * (nsupc + 1) + 10 * nsupc;

		if ( nsupc == 1 ) {
		    z_div(&x[fsupc], &x[fsupc], &Lval[luptr]);
		    for (i = U_NZ_START(fsupc); i < U_NZ_END(fsupc); ++i) {
			irow = U_SUB(i);
			zz_mult(&comp_zero, &x[fsupc], &Uval[i]);
			z_sub(&x[irow], &x[irow], &comp_zero);
		    }
		} else {
#ifdef USE_VENDOR_BLAS
#if ( MACH==CRAY_PVP )
                    ftcs1 = _cptofcd("U", strlen("U"));
                    ftcs2 = _cptofcd("N", strlen("N"));
                    ftcs3 = _cptofcd("N", strlen("N"));

		    CTRSV(ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
		       &x[fsupc], &incx);
#else
		    ztrsv_("U", "N", "N", &nsupc, &Lval[luptr], &nsupr,
                           &x[fsupc], &incx);
#endif
#else		
		    zusolve ( nsupr, nsupc, &Lval[luptr], &x[fsupc] );
#endif		

                    for (jcol = fsupc; jcol < fsupc + nsupc; jcol++) {
		        solve_ops += 8*(U_NZ_END(jcol) - U_NZ_START(jcol));
		    	for (i = U_NZ_START(jcol); i < U_NZ_END(jcol); i++) {
			    irow = U_SUB(i);
			zz_mult(&comp_zero, &x[jcol], &Uval[i]);
			z_sub(&x[irow], &x[irow], &comp_zero);
		    	}
                    }
		}
	    } /* for k ... */
	    
	}
    } else if ( lsame_(trans, "T") ) { /* Form x := inv(A')*x */
	
	if ( lsame_(uplo, "L") ) {
	    /* Form x := inv(L')*x */
    	    if ( L->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = Lstore->nsuper; k >= 0; --k) {
	    	fsupc = L_FST_SUPC(k);
	    	istart = L_SUB_START(fsupc);
                nsupr = L_SUB_END(fsupc) - istart;
                nsupc = L_LAST_SUPC(k) - fsupc;
	    	luptr = L_NZ_START(fsupc);

		solve_ops += 8 * (nsupr - nsupc) * nsupc;

		for (jcol = fsupc; jcol < L_LAST_SUPC(k); jcol++) {
		    iptr = istart + nsupc;
		    for (i = L_NZ_START(jcol) + nsupc; 
				i < L_NZ_END(jcol); i++) {
			irow = L_SUB(iptr);
			zz_mult(&comp_zero, &x[irow], &Lval[i]);
		    	z_sub(&x[jcol], &x[jcol], &comp_zero);
			iptr++;
		    }
		}
		
		if ( nsupc > 1 ) {
		    solve_ops += 4 * nsupc * (nsupc - 1);
#ifdef _CRAY
                    ftcs1 = _cptofcd("L", strlen("L"));
                    ftcs2 = _cptofcd("T", strlen("T"));
                    ftcs3 = _cptofcd("U", strlen("U"));
		    CTRSV(ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
			&x[fsupc], &incx);
#else
		    ztrsv_("L", "T", "U", &nsupc, &Lval[luptr], &nsupr,
			&x[fsupc], &incx);
#endif
		}
	    }
	} else {
	    /* Form x := inv(U')*x */
	    if ( U->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = 0; k <= nsuper; k++) {
	    	fsupc = L_FST_SUPC(k);
                nsupr = L_SUB_END(fsupc) - L_SUB_START(fsupc);
                nsupc = L_LAST_SUPC(k) - fsupc;
	    	luptr = L_NZ_START(fsupc);

                for (jcol = fsupc; jcol < fsupc + nsupc; jcol++) {
		    solve_ops += 8*(U_NZ_START(jcol+1) - U_NZ_START(jcol));
                    for (i = U_NZ_START(jcol); i < U_NZ_END(jcol); i++) {
			irow = U_SUB(i);
			zz_mult(&comp_zero, &x[irow], &Uval[i]);
		    	z_sub(&x[jcol], &x[jcol], &comp_zero);
		    }
		}

                /* 1 z_div costs 10 flops */
		solve_ops += 4 * nsupc * (nsupc + 1) + 10 * nsupc;

		if ( nsupc == 1 ) {
		    z_div(&x[fsupc], &x[fsupc], &Lval[luptr]);
		} else {
#ifdef _CRAY
                    ftcs1 = _cptofcd("U", strlen("U"));
                    ftcs2 = _cptofcd("T", strlen("T"));
                    ftcs3 = _cptofcd("N", strlen("N"));
		    CTRSV( ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
			    &x[fsupc], &incx);
#else
		    ztrsv_("U", "T", "N", &nsupc, &Lval[luptr], &nsupr,
			    &x[fsupc], &incx);
#endif
		}
	    } /* for k ... */
	}
    } else { /* Form x := conj(inv(A'))*x */
	
	if ( lsame_(uplo, "L") ) {
	    /* Form x := conj(inv(L'))*x */
    	    if ( L->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = Lstore->nsuper; k >= 0; --k) {
	    	fsupc = L_FST_SUPC(k);
	    	istart = L_SUB_START(fsupc);
	    	nsupr = L_SUB_END(fsupc) - istart;
	    	nsupc = L_LAST_SUPC(k) - fsupc;
	    	luptr = L_NZ_START(fsupc);

		solve_ops += 8 * (nsupr - nsupc) * nsupc;

		for (jcol = fsupc; jcol < L_FST_SUPC(k+1); jcol++) {
		    iptr = istart + nsupc;
		    for (i = L_NZ_START(jcol) + nsupc; 
				i < L_NZ_START(jcol+1); i++) {
			irow = L_SUB(iptr);
                        zz_conj(&temp, &Lval[i]);
			zz_mult(&comp_zero, &x[irow], &temp);
		    	z_sub(&x[jcol], &x[jcol], &comp_zero);
			iptr++;
		    }
 		}
 		
 		if ( nsupc > 1 ) {
		    solve_ops += 4 * nsupc * (nsupc - 1);
#ifdef _CRAY
                    ftcs1 = _cptofcd("L", strlen("L"));
                    ftcs2 = _cptofcd(trans, strlen("T"));
                    ftcs3 = _cptofcd("U", strlen("U"));
		    ZTRSV(ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
			&x[fsupc], &incx);
#else
                    ztrsv_("L", trans, "U", &nsupc, &Lval[luptr], &nsupr,
                           &x[fsupc], &incx);
#endif
		}
	    }
	} else {
	    /* Form x := conj(inv(U'))*x */
	    if ( U->nrow == 0 ) return 0; /* Quick return */
	    
	    for (k = 0; k <= Lstore->nsuper; k++) {
	    	fsupc = L_FST_SUPC(k);
	    	nsupr = L_SUB_START(fsupc+1) - L_SUB_START(fsupc);
	    	nsupc = L_FST_SUPC(k+1) - fsupc;
	    	luptr = L_NZ_START(fsupc);

		for (jcol = fsupc; jcol < L_FST_SUPC(k+1); jcol++) {
		    solve_ops += 8*(U_NZ_START(jcol+1) - U_NZ_START(jcol));
		    for (i = U_NZ_START(jcol); i < U_NZ_START(jcol+1); i++) {
			irow = U_SUB(i);
                        zz_conj(&temp, &Uval[i]);
			zz_mult(&comp_zero, &x[irow], &temp);
		    	z_sub(&x[jcol], &x[jcol], &comp_zero);
		    }
		}

                /* 1 z_div costs 10 flops */
		solve_ops += 4 * nsupc * (nsupc + 1) + 10 * nsupc;
 
		if ( nsupc == 1 ) {
                    zz_conj(&temp, &Lval[luptr]);
		    z_div(&x[fsupc], &x[fsupc], &temp);
		} else {
#ifdef _CRAY
                    ftcs1 = _cptofcd("U", strlen("U"));
                    ftcs2 = _cptofcd(trans, strlen("T"));
                    ftcs3 = _cptofcd("N", strlen("N"));
		    ZTRSV( ftcs1, ftcs2, ftcs3, &nsupc, &Lval[luptr], &nsupr,
			    &x[fsupc], &incx);
#else
                    ztrsv_("U", trans, "N", &nsupc, &Lval[luptr], &nsupr,
                               &x[fsupc], &incx);
#endif
  		}
  	    } /* for k ... */
  	}
    }

    SUPERLU_FREE(work);
    return 0;
}



int_t
sp_zgemv(char *trans, doublecomplex alpha, SuperMatrix *A, doublecomplex *x, 
	 int_t incx, doublecomplex beta, doublecomplex *y, int_t incy)
{
/*  Purpose   
    =======   

    sp_zgemv()  performs one of the matrix-vector operations   
       y := alpha*A*x + beta*y,   or   y := alpha*A'*x + beta*y,   
    where alpha and beta are scalars, x and y are vectors and A is a
    sparse A->nrow by A->ncol matrix.   

    Parameters   
    ==========   

    TRANS  - (input) char*
             On entry, TRANS specifies the operation to be performed as   
             follows:   
                TRANS = 'N' or 'n'   y := alpha*A*x + beta*y.   
                TRANS = 'T' or 't'   y := alpha*A'*x + beta*y.   
                TRANS = 'C' or 'c'   y := alpha*A'*x + beta*y.   

    ALPHA  - (input) doublecomplex
             On entry, ALPHA specifies the scalar alpha.   

    A      - (input) SuperMatrix*
             Before entry, the leading m by n part of the array A must   
             contain the matrix of coefficients.   

    X      - (input) doublecomplex*, array of DIMENSION at least   
             ( 1 + ( n - 1 )*abs( INCX ) ) when TRANS = 'N' or 'n'   
             and at least   
             ( 1 + ( m - 1 )*abs( INCX ) ) otherwise.   
             Before entry, the incremented array X must contain the   
             vector x.   

    INCX   - (input) int
             On entry, INCX specifies the increment for the elements of   
             X. INCX must not be zero.   

    BETA   - (input) doublecomplex
             On entry, BETA specifies the scalar beta. When BETA is   
             supplied as zero then Y need not be set on input.   

    Y      - (output) doublecomplex*,  array of DIMENSION at least   
             ( 1 + ( m - 1 )*abs( INCY ) ) when TRANS = 'N' or 'n'   
             and at least   
             ( 1 + ( n - 1 )*abs( INCY ) ) otherwise.   
             Before entry with BETA non-zero, the incremented array Y   
             must contain the vector y. On exit, Y is overwritten by the 
             updated vector y.
	     
    INCY   - (input) int
             On entry, INCY specifies the increment for the elements of   
             Y. INCY must not be zero.   

    ==== Sparse Level 2 Blas routine.   
*/

    /* Local variables */
    NCformat *Astore;
    doublecomplex   *Aval;
    int info;
    doublecomplex temp, temp1;
    int_t lenx, leny, i, j, irow;
    int_t iy, jx, jy, kx, ky;
    int_t notran;
    doublecomplex comp_zero = {0.0, 0.0};
    doublecomplex comp_one = {1.0, 0.0};

    notran = lsame_(trans, "N");
    Astore = A->Store;
    Aval = Astore->nzval;
    
    /* Test the input parameters */
    info = 0;
    if ( !notran && !lsame_(trans, "T") && !lsame_(trans, "C")) info = 1;
    else if ( A->nrow < 0 || A->ncol < 0 ) info = 3;
    else if (incx == 0) info = 5;
    else if (incy == 0)	info = 8;
    if (info != 0) {
	xerbla_("sp_zgemv ", &info);
	return 0;
    }

    /* Quick return if possible. */
    if (A->nrow == 0 || A->ncol == 0 || 
	z_eq(&alpha, &comp_zero) && 
	z_eq(&beta, &comp_one))
	return 0;


    /* Set  LENX  and  LENY, the lengths of the vectors x and y, and set 
       up the start points in  X  and  Y. */
    if (lsame_(trans, "N")) {
	lenx = A->ncol;
	leny = A->nrow;
    } else {
	lenx = A->nrow;
	leny = A->ncol;
    }
    if (incx > 0) kx = 0;
    else kx =  - (lenx - 1) * incx;
    if (incy > 0) ky = 0;
    else ky =  - (leny - 1) * incy;

    /* Start the operations. In this version the elements of A are   
       accessed sequentially with one pass through A. */
    /* First form  y := beta*y. */
    if ( !z_eq(&beta, &comp_one) ) {
	if (incy == 1) {
	    if ( z_eq(&beta, &comp_zero) )
		for (i = 0; i < leny; ++i) y[i] = comp_zero;
	    else
		for (i = 0; i < leny; ++i) 
		  zz_mult(&y[i], &beta, &y[i]);
	} else {
	    iy = ky;
	    if ( z_eq(&beta, &comp_zero) )
		for (i = 0; i < leny; ++i) {
		    y[iy] = comp_zero;
		    iy += incy;
		}
	    else
		for (i = 0; i < leny; ++i) {
		    zz_mult(&y[iy], &beta, &y[iy]);
		    iy += incy;
		}
	}
    }
    
    if ( z_eq(&alpha, &comp_zero) ) return 0;

    if ( notran ) {
	/* Form  y := alpha*A*x + y. */
	jx = kx;
	if (incy == 1) {
	    for (j = 0; j < A->ncol; ++j) {
		if ( !z_eq(&x[jx], &comp_zero) ) {
		    zz_mult(&temp, &alpha, &x[jx]);
		    for (i = Astore->colptr[j]; i < Astore->colptr[j+1]; ++i) {
			irow = Astore->rowind[i];
			zz_mult(&temp1, &temp,  &Aval[i]);
			z_add(&y[irow], &y[irow], &temp1);
		    }
		}
		jx += incx;
	    }
	} else {
	    SUPERLU_ABORT("Not implemented.");
	}
    } else {
	/* Form  y := alpha*A'*x + y. */
	jy = ky;
	if (incx == 1) {
	    for (j = 0; j < A->ncol; ++j) {
		temp = comp_zero;
		for (i = Astore->colptr[j]; i < Astore->colptr[j+1]; ++i) {
		    irow = Astore->rowind[i];
		    zz_mult(&temp1, &Aval[i], &x[irow]);
		    z_add(&temp, &temp, &temp1);
		}
		zz_mult(&temp1, &alpha, &temp);
		z_add(&y[jy], &y[jy], &temp1);
		jy += incy;
	    }
	} else {
	    SUPERLU_ABORT("Not implemented.");
	}
    }
    return 0;    
} /* sp_zgemv */

