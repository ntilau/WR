
#include <stdio.h>
#include <stdlib.h>
#include "slu_mt_zdefs.h"

void zlsolve(int_t, int_t, doublecomplex *, doublecomplex *);
void zmatvec(int_t, int_t, int_t, doublecomplex *, doublecomplex *, doublecomplex *);

void
pzgstrf_bmod1D(
	       const int_t pnum,  /* process number */
	       const int_t m,     /* number of rows in the matrix */
	       const int_t w,     /* current panel width */
	       const int_t jcol,  /* leading column of the current panel */
	       const int_t fsupc, /* leading column of the updating supernode */ 
	       const int_t krep,  /* last column of the updating supernode */ 
	       const int_t nsupc, /* number of columns in the updating s-node */ 
	       int_t nsupr, /* number of rows in the updating supernode */  
	       int_t nrow,  /* number of rows below the diagonal block of
			     the updating supernode */ 
	       int_t *repfnz,     /* in */
	       int_t *panel_lsub, /* modified */
	       int_t *w_lsub_end, /* modified */
	       int_t *spa_marker, /* modified; size n-by-w */
	       doublecomplex *dense,   /* modified */
	       doublecomplex *tempv,   /* working array - zeros on entry/exit */
	       GlobalLU_t *Glu, /* modified */
	       Gstat_t *Gstat   /* modified */
	       )
{
/*
 * -- SuperLU MT routine (version 2.0) --
 * Lawrence Berkeley National Lab,  Univ. of California Berkeley,
 * and Xerox Palo Alto Research Center.
 * September 10, 2007
 *
 * Purpose
 * =======
 *
 *    Performs numeric block updates (sup-panel) in topological order.
 *    It features: col-col, 2cols-col, 3cols-col, and sup-col updates.
 *    Results are returned in SPA dense[*,w].
 *
 */
#if ( MACH==CRAY_PVP )
    _fcd ftcs1 = _cptofcd("L", strlen("L")),
         ftcs2 = _cptofcd("N", strlen("N")),
         ftcs3 = _cptofcd("U", strlen("U"));
#endif
#ifdef USE_VENDOR_BLAS
    int          incx = 1, incy = 1;
    doublecomplex       alpha, beta;
#endif

    doublecomplex       ukj, ukj1, ukj2;
    int_t          luptr, luptr1, luptr2;
    int            segsze, nrow32 = nrow, nsupr32 = nsupr;
    register int_t lptr; /* start of row subscripts of the updating supernode */
    register int_t i, krep_ind, kfnz, isub, irow, no_zeros;
    register int_t jj;	      /* index through each column in the panel */
    int_t          *repfnz_col; /* repfnz[] for a column in the panel */
    doublecomplex       *dense_col;  /* dense[] for a column in the panel */
    doublecomplex      *tempv1;     /* used to store matrix-vector result */
    int_t          *col_marker; /* each column of the spa_marker[*,w] */
    int_t          *col_lsub;   /* each column of the panel_lsub[*,w] */
    int_t          *lsub, *xlsub_end;
    doublecomplex       *lusup;
    int_t          *xlusup;
    register float flopcnt;

    doublecomplex      zero = {0.0, 0.0};
    doublecomplex      one = {1.0, 0.0};
    doublecomplex      comp_temp, comp_temp1;
    
#ifdef TIMING
    double *utime = Gstat->utime;
    double f_time;
#endif    
    
    lsub      = Glu->lsub;
    xlsub_end = Glu->xlsub_end;
    lusup     = Glu->lusup;
    xlusup    = Glu->xlusup;
    lptr      = Glu->xlsub[fsupc];
    krep_ind  = lptr + nsupc - 1;

    /* Pointers to each column of the w-wide arrays. */
    repfnz_col= repfnz;
    dense_col = dense;
    col_marker= spa_marker;
    col_lsub  = panel_lsub;

#if ( DEBUGlevel>=2 )
if (jcol == BADPAN && krep == BADREP) {
    printf("(%d) pzgstrf_bmod1D[1] jcol %d, fsupc %d, krep %d, nsupc %d, nsupr %d, nrow %d\n",
	   pnum, jcol, fsupc, krep, nsupc, nsupr, nrow);
    PrintInt10("lsub[xlsub[2774]]", nsupr, &lsub[lptr]);
}    
#endif
    
    /*
     * Sequence through each column in the panel ...
     */
    for (jj = jcol; jj < jcol + w; ++jj, col_marker += m, col_lsub += m,
	 repfnz_col += m, dense_col += m) {

	kfnz = repfnz_col[krep];
	if ( kfnz == EMPTY ) continue;	/* Skip any zero segment */

	segsze = krep - kfnz + 1;
	luptr = xlusup[fsupc];

	/* Calculate flops: tri-solve + mat-vector */
        flopcnt = 4 * segsze * (segsze - 1) + 8 * nrow * segsze;
	Gstat->procstat[pnum].fcops += flopcnt;

	/* Case 1: Update U-segment of size 1 -- col-col update */
	if ( segsze == 1 ) {
#ifdef TIMING
	    f_time = SuperLU_timer_();
#endif	    
	    ukj = dense_col[lsub[krep_ind]];
	    luptr += nsupr*(nsupc-1) + nsupc;
#if ( DEBUGlevel>=2 )
if (krep == BADCOL && jj == -1) {
    printf("(%d) pzgstrf_bmod1D[segsze=1]: k %d, j %d, ukj %.10e\n",
	   pnum, lsub[krep_ind], jj, ukj);
    PrintInt10("segsze=1", nsupr, &lsub[lptr]);
}
#endif	    
	    for (i = lptr + nsupc; i < xlsub_end[fsupc]; i++) {
		irow = lsub[i];
                        zz_mult(&comp_temp, &ukj, &lusup[luptr]);
                        z_sub(&dense_col[irow], &dense_col[irow], &comp_temp);
		++luptr;
#ifdef SCATTER_FOUND		
		if ( col_marker[irow] != jj ) {
		    col_marker[irow] = jj;
		    col_lsub[w_lsub_end[jj-jcol]++] = irow;
		}
#endif		
	    }
#ifdef TIMING
	    utime[FLOAT] += SuperLU_timer_() - f_time;
#endif	    
	} else if ( segsze <= 3 ) {
#ifdef TIMING
	    f_time = SuperLU_timer_();
#endif	    
	    ukj = dense_col[lsub[krep_ind]];
	    luptr += nsupr*(nsupc-1) + nsupc-1;
	    ukj1 = dense_col[lsub[krep_ind - 1]];
	    luptr1 = luptr - nsupr;
	    if ( segsze == 2 ) {
                zz_mult(&comp_temp, &ukj1, &lusup[luptr1]);
                z_sub(&ukj, &ukj, &comp_temp);
		dense_col[lsub[krep_ind]] = ukj;
		for (i = lptr + nsupc; i < xlsub_end[fsupc]; ++i) {
		    irow = lsub[i];
		    ++luptr;  ++luptr1;
                            zz_mult(&comp_temp, &ukj, &lusup[luptr]);
                            zz_mult(&comp_temp1, &ukj1, &lusup[luptr1]);
                            z_add(&comp_temp, &comp_temp, &comp_temp1);
                            z_sub(&dense_col[irow], &dense_col[irow], &comp_temp);
#ifdef SCATTER_FOUND		
		    if ( col_marker[irow] != jj ) {
			col_marker[irow] = jj;
			col_lsub[w_lsub_end[jj-jcol]++] = irow;
		    }
#endif		
		}
	    } else {
		ukj2 = dense_col[lsub[krep_ind - 2]];
		luptr2 = luptr1 - nsupr;
                zz_mult(&comp_temp, &ukj2, &lusup[luptr2-1]);
                z_sub(&ukj1, &ukj1, &comp_temp);

                zz_mult(&comp_temp, &ukj1, &lusup[luptr1]);
                zz_mult(&comp_temp1, &ukj2, &lusup[luptr2]);
                z_add(&comp_temp, &comp_temp, &comp_temp1);
                z_sub(&ukj, &ukj, &comp_temp);
		dense_col[lsub[krep_ind]] = ukj;
		dense_col[lsub[krep_ind-1]] = ukj1;
		for (i = lptr + nsupc; i < xlsub_end[fsupc]; ++i) {
		    irow = lsub[i];
		    ++luptr; ++luptr1; ++luptr2;
                    zz_mult(&comp_temp, &ukj, &lusup[luptr]);
                    zz_mult(&comp_temp1, &ukj1, &lusup[luptr1]);
                    z_add(&comp_temp, &comp_temp, &comp_temp1);
                    zz_mult(&comp_temp1, &ukj2, &lusup[luptr2]);
                    z_add(&comp_temp, &comp_temp, &comp_temp1);
                    z_sub(&dense_col[irow], &dense_col[irow], &comp_temp);
#ifdef SCATTER_FOUND		
		    if ( col_marker[irow] != jj ) {
			col_marker[irow] = jj;
			col_lsub[w_lsub_end[jj-jcol]++] = irow;
		    }
#endif		
		}
	    }
#ifdef TIMING
	    utime[FLOAT] += SuperLU_timer_() - f_time;
#endif	    
	} else { /* segsze >= 4 */
	    /* 
	     * Perform a triangular solve and matrix-vector update,
	     * then scatter the result of sup-col update to dense[*].
	     */
	    no_zeros = kfnz - fsupc;

	    /* Gather U[*,j] segment from dense[*] to tempv[*]: 
	     *   The result of triangular solve is in tempv[*];
	     *   The result of matrix vector update is in dense_col[*]
	     */
	    isub = lptr + no_zeros;
/*#pragma ivdep*/
	    for (i = 0; i < segsze; ++i) {
		irow = lsub[isub];
		tempv[i] = dense_col[irow]; /* Gather */
		++isub;
	    }

	    /* start effective triangle */
	    luptr += nsupr * no_zeros + no_zeros;
#ifdef TIMING
	    f_time = SuperLU_timer_();
#endif
		
#ifdef USE_VENDOR_BLAS
#if ( MACH==CRAY_PVP )
	    CTRSV( ftcs1, ftcs2, ftcs3, &segsze, &lusup[luptr], 
		  &nsupr, tempv, &incx );
#else
	    ztrsv_( "L", "N", "U", &segsze, &lusup[luptr], 
		   &nsupr32, tempv, &incx );
#endif
		
	    luptr += segsze;	/* Dense matrix-vector */
	    tempv1 = &tempv[segsze];

            alpha = one;
            beta = zero;
#if ( MACH==CRAY_PVP )
	    CGEMV( ftcs2, &nrow, &segsze, &alpha, &lusup[luptr], 
		  &nsupr, tempv, &incx, &beta, tempv1, &incy );
#else
	    zgemv_( "N", &nrow32, &segsze, &alpha, &lusup[luptr], 
		   &nsupr32, tempv, &incx, &beta, tempv1, &incy );
#endif /* _CRAY_PVP */
#else
	    zlsolve ( nsupr, segsze, &lusup[luptr], tempv );
	    
	    luptr += segsze;        /* Dense matrix-vector */
	    tempv1 = &tempv[segsze];
	    zmatvec (nsupr, nrow, segsze, &lusup[luptr], tempv, tempv1);
#endif
		
#ifdef TIMING
	    utime[FLOAT] += SuperLU_timer_() - f_time;
#endif	    

	    /* Scatter tempv[*] into SPA dense[*] temporarily, 
	     * such that tempv[*] can be used for the triangular solve of
	     * the next column of the panel. They will be copied into 
	     * ucol[*] after the whole panel has been finished.
	     */
	    isub = lptr + no_zeros;
/*#pragma ivdep*/
	    for (i = 0; i < segsze; i++) {
		irow = lsub[isub];
		dense_col[irow] = tempv[i]; /* Scatter */
		tempv[i] = zero;
		isub++;
#if ( DEBUGlevel>=2 )
	if (jj == -1 && krep == 3423)
	    printf("(%d) pzgstrf_bmod1D[scatter] jj %d, dense_col[%d] %e\n",
		   pnum, jj, irow, dense_col[irow]);
#endif
	    }
		
	    /* Scatter the update from tempv1[*] into SPA dense[*] */
/*#pragma ivdep*/
	    for (i = 0; i < nrow; i++) {
		irow = lsub[isub];
                z_sub(&dense_col[irow], &dense_col[irow],
                              &tempv1[i]); /* Scatter-add */
#ifdef SCATTER_FOUND		
		if ( col_marker[irow] != jj ) {
		    col_marker[irow] = jj;
		    col_lsub[w_lsub_end[jj-jcol]++] = irow;
		}
#endif		
		tempv1[i] = zero;
		isub++;
	    }
		
	} /* else segsze >= 4 ... */
	
    } /* for jj ... */

}
