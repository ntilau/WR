
/*
 * -- SuperLU MT routine (version 2.2) --
 * Lawrence Berkeley National Lab, Univ. of California Berkeley,
 * and Xerox Palo Alto Research Center.
 * September 10, 2007
 * 
 * Last modified: 
 * -- 8/29/2013: added lock to access Stack memory supplied by user
 *   
 */
#include "slu_mt_sdefs.h"

/* ------------------
   Constants & Macros
   ------------------ */
#define EXPAND      1.5
#define NO_MEMTYPE  4      /* 0: lusup;
			      1: ucol;
			      2: lsub;
			      3: usub */
#define GluIntArray(n)   (9 * (n) + 5)

/* -------------------
   Internal prototypes
   ------------------- */
void    *psgstrf_expand (int_t *, MemType,int_t, int_t, GlobalLU_t *);
void    copy_mem_float (int_t, void *, void *);
void    psgstrf_StackCompress(GlobalLU_t *);
void    psgstrf_SetupSpace (void *, int_t);
void    *suser_malloc   (int_t, int_t);
void    suser_free      (int_t, int_t);

/* ----------------------------------------------
   External prototypes (in memory.c - prec-indep)
   ---------------------------------------------- */
extern void    copy_mem_int    (int_t, void *, void *);
extern void    user_bcopy      (char *, char *, int_t);

typedef struct {
    int_t  size;
    int_t  used;
    int_t  top1;  /* grow upward, relative to &array[0] */
    int_t  top2;  /* grow downward */
    void *array;
#if ( MACH==PTHREAD )
    pthread_mutex_t lock;;
#endif
} LU_stack_t;

typedef enum {HEAD, TAIL}   stack_end_t;
typedef enum {SYSTEM, USER} LU_space_t;

ExpHeader *sexpanders = 0; /* Array of pointers to 4 types of memory */
static LU_stack_t stack;
static int_t        no_expand;
static int_t        ndim;
static LU_space_t whichspace; /* 0 - system malloc'd; 1 - user provided */

/* Macros to manipulate stack */
#define StackFull(x)         ( x + stack.used >= stack.size )
#define NotDoubleAlign(addr) ( (long long int)addr & 7 )
#define DoubleAlign(addr)    ( ((long long int)addr + 7) & ~7L )

#define Reduce(alpha)        ((alpha + 1) / 2)     /* i.e. (alpha-1)/2 + 1 */

/* temporary space used by BLAS calls */
#define NUM_TEMPV(n,w,t,b)  (SUPERLU_MAX( 2*n, (t + b)*w ))

/*
 * Setup the memory model to be used for factorization.
 *    lwork = 0: use system malloc;
 *    lwork > 0: use user-supplied work[] space.
 */
void psgstrf_SetupSpace(void *work, int_t lwork)
{
    if ( lwork == 0 ) {
        whichspace = SYSTEM; /* malloc/free */
    } else if ( lwork > 0 ) {
        whichspace = USER;   /* user provided space */
        stack.size = lwork;
        stack.used = 0;
        stack.top1 = 0;
        stack.top2 = lwork;
        stack.array = (void *) work;
    }
#if ( MACH==PTHREAD )
    pthread_mutex_init ( &stack.lock, NULL);
#endif
}

/*
 * Destroy the lock used for user stack memory.
 */
void psgstrf_StackFree()
{
#if ( MACH==PTHREAD ) /* Use pthread ... */
     if ( whichspace == USER ) 
         pthread_mutex_destroy( &stack.lock );
#endif
} 

void *suser_malloc(int_t bytes, int_t which_end)
{
    void *buf;

#if ( MACH==PTHREAD ) /* Use pthread ... */
    pthread_mutex_lock( &stack.lock );
#elif ( MACH==OPENMP ) /* Use openMP ... */
#pragma omp critical ( STACK_LOCK )
#endif
    {    
        if ( StackFull(bytes) ) {
            buf = NULL;
            goto end;
        }

        if ( which_end == HEAD ) {
	    buf = (char*) stack.array + stack.top1;
	    stack.top1 += bytes;
        } else {
	    stack.top2 -= bytes;
	    buf = (char*) stack.array + stack.top2;
        }
        stack.used += bytes;
        
     end: ;
    } /* ---- end critical section ---- */

#if ( MACH==PTHREAD ) /* Use pthread ... */
    pthread_mutex_unlock( &stack.lock );
#endif
    return buf;
}


void suser_free(int_t bytes, int_t which_end)
{

#if ( MACH==PTHREAD ) /* Use pthread ... */
    pthread_mutex_lock( &stack.lock );
#elif ( MACH==OPENMP ) /* Use openMP ... */
#pragma omp critical ( STACK_LOCK )
#endif

    {
        if ( which_end == HEAD ) stack.top1 -= bytes;
        else stack.top2 += bytes;
        stack.used -= bytes;
    }

#if ( MACH==PTHREAD ) /* Use pthread ... */
    pthread_mutex_unlock( &stack.lock );
#endif

}


/* Returns the working storage used during factorization */
int_t superlu_sTempSpace(int_t n, int_t w, int_t p)
{
    register float tmp, ptmp;
    register int_t iword = sizeof(int_t), dword = sizeof(float);
    int_t    maxsuper = sp_ienv(3), rowblk = sp_ienv(4);

    /* globally shared */
    tmp = 14 * n * iword;

    /* local to each processor */
    ptmp = (2 * w + 5 + NO_MARKER) * n * iword;
    ptmp += (n * w + NUM_TEMPV(n,w,maxsuper,rowblk)) * dword;
#if ( PRNTlevel>=1 )
    printf("Per-processor work[] %.0f MB\n", ptmp/1024/1024);
#endif
    ptmp *= p;

    return (tmp + ptmp);
}

/*
 * superlu_memusage consists of the following fields:
 *    o for_lu (float)
 *      The amount of space used in bytes for L\U data structures.
 *    o total_needed (float)
 *      The amount of space needed in bytes to perform factorization.
 *    o expansions (int)
 *      The number of memory expansions during the LU factorization.
 */
int_t superlu_sQuerySpace(int_t P, SuperMatrix *L, SuperMatrix *U, int_t panel_size,
                       superlu_memusage_t *superlu_memusage)
{
    SCPformat *Lstore;
    NCPformat *Ustore;
    register int_t n, iword, dword, lwork;

    Lstore = L->Store;
    Ustore = U->Store;
    n = L->ncol;
    iword = sizeof(int_t);
    dword = sizeof(float);

    /* L supernodes of type SCP */
    superlu_memusage->for_lu = (float) (7*n + 3) * iword
                             + (float) Lstore->nzval_colend[n-1] * dword
                             + (float) Lstore->rowind_colend[n-1] * iword;

    /* U columns of type NCP */
    superlu_memusage->for_lu += (2*n + 1) * iword
        + (float) Ustore->colend[n-1] * (dword + iword);

    /* Working storage to support factorization */
    lwork = superlu_sTempSpace(n, panel_size, P);
    superlu_memusage->total_needed = superlu_memusage->for_lu + lwork;

    superlu_memusage->expansions = --no_expand;

    return 0;
}

float psgstrf_memory_use(const int_t nzlmax, const int_t nzumax, const int_t nzlumax)
{
    register float iword, dword, t;

    iword   = sizeof(int_t);
    dword   = sizeof(float);

    t = 10. * ndim * iword + nzlmax * iword + nzumax * (iword + dword)
	+ nzlumax * dword;
    return t;
}


/*
 * Allocate storage for the data structures common to all factor routines.
 * For those unpredictable size, make a guess as FILL * nnz(A).
 * Return value:
 *     If lwork = -1, return the estimated amount of space required;
 *     otherwise, return the amount of space actually allocated when
 *     memory allocation failure occurred.
 */
float
psgstrf_MemInit(int_t n, int_t annz, superlumt_options_t *superlumt_options,
		SuperMatrix *L, SuperMatrix *U, GlobalLU_t *Glu)
{
    register int_t nprocs = superlumt_options->nprocs;
    yes_no_t refact = superlumt_options->refact;
    register int_t panel_size = superlumt_options->panel_size;
    register int_t lwork = superlumt_options->lwork;
    void     *work = superlumt_options->work;
    int_t      iword, dword, retries = 0;
    SCPformat *Lstore;
    NCPformat *Ustore;
    int_t      *xsup, *xsup_end, *supno;
    int_t      *lsub, *xlsub, *xlsub_end;
    float   *lusup;
    int_t      *xlusup, *xlusup_end;
    float   *ucol;
    int_t      *usub, *xusub, *xusub_end;
    int_t      nzlmax, nzumax, nzlumax;
    int_t      FILL_LUSUP = sp_ienv(6); /* Guess the fill-in growth for LUSUP */
    int_t      FILL_UCOL = sp_ienv(7); /* Guess the fill-in growth for UCOL */
    int_t      FILL_LSUB = sp_ienv(8); /* Guess the fill-in growth for LSUB */
    
    no_expand = 0;
    ndim      = n;
    iword     = sizeof(int_t);
    dword     = sizeof(float);

    if ( !sexpanders )
      sexpanders = (ExpHeader *) SUPERLU_MALLOC(NO_MEMTYPE * sizeof(ExpHeader));

    if ( refact == NO ) {

	/* Guess amount of storage needed by L\U factors. */
        if ( FILL_UCOL < 0 ) nzumax = -FILL_UCOL * annz;
	else nzumax = FILL_UCOL;
	if ( FILL_LSUB < 0 ) nzlmax = -FILL_LSUB * annz;
	else nzlmax = FILL_LSUB;

	if ( Glu->dynamic_snode_bound == YES ) {
	    if ( FILL_LUSUP < 0 ) nzlumax = -FILL_LUSUP * annz;
	    else nzlumax = FILL_LUSUP; /* estimate an upper bound */
	} else {
	    nzlumax = Glu->nzlumax; /* preset as static upper bound */
	}

	if ( lwork == -1 ) {
	    return (GluIntArray(n) * iword + 
		    superlu_sTempSpace(n, panel_size, nprocs)
		    + (nzlmax+nzumax)*iword + (nzlumax+nzumax)*dword);
        } else {
	    psgstrf_SetupSpace(work, lwork);
	}
	
	/* Integer pointers for L\U factors */
	if ( whichspace == SYSTEM ) {
	    xsup       = intMalloc(n+1);
	    xsup_end   = intMalloc(n);
	    supno      = intMalloc(n+1);
	    xlsub      = intMalloc(n+1);
	    xlsub_end  = intMalloc(n);
	    xlusup     = intMalloc(n+1);
	    xlusup_end = intMalloc(n);
	    xusub      = intMalloc(n+1);
	    xusub_end  = intMalloc(n);
	} else {
	    xsup       = (int_t *)suser_malloc((n+1) * iword, HEAD);
	    xsup_end   = (int_t *)suser_malloc((n) * iword, HEAD);
	    supno      = (int_t *)suser_malloc((n+1) * iword, HEAD);
	    xlsub      = (int_t *)suser_malloc((n+1) * iword, HEAD);
	    xlsub_end  = (int_t *)suser_malloc((n) * iword, HEAD);
	    xlusup     = (int_t *)suser_malloc((n+1) * iword, HEAD);
	    xlusup_end = (int_t *)suser_malloc((n) * iword, HEAD);
	    xusub      = (int_t *)suser_malloc((n+1) * iword, HEAD);
	    xusub_end  = (int_t *)suser_malloc((n) * iword, HEAD);
	}

	lusup = (float *) psgstrf_expand( &nzlumax, LUSUP, 0, 0, Glu );
	ucol  = (float *) psgstrf_expand( &nzumax, UCOL, 0, 0, Glu );
	lsub  = (int_t *)    psgstrf_expand( &nzlmax, LSUB, 0, 0, Glu );
	usub  = (int_t *)    psgstrf_expand( &nzumax, USUB, 0, 1, Glu );

	while ( !ucol || !lsub || !usub ) {
	    /*SUPERLU_ABORT("Not enough core in LUMemInit()");*/
#if (PRNTlevel==1)
	    printf(".. psgstrf_MemInit(): #retries " IFMT "\n", ++retries);
#endif
	    if ( whichspace == SYSTEM ) {
		SUPERLU_FREE(ucol);
		SUPERLU_FREE(lsub);
		SUPERLU_FREE(usub);
	    } else {
		suser_free(nzumax*dword+(nzlmax+nzumax)*iword, HEAD);
	    }
	    nzumax /= 2;    /* reduce request */
	    nzlmax /= 2;
	    if ( nzumax < annz/2 ) {
		printf("Not enough memory to perform factorization.\n");
		return (psgstrf_memory_use(nzlmax, nzumax, nzlumax) + n);
	    }
	    ucol  = (float *) psgstrf_expand( &nzumax, UCOL, 0, 0, Glu );
	    lsub  = (int_t *)  psgstrf_expand( &nzlmax, LSUB, 0, 0, Glu );
	    usub  = (int_t *)  psgstrf_expand( &nzumax, USUB, 0, 1, Glu );
	}
	
	if ( !lusup )  {
	    float t = psgstrf_memory_use(nzlmax, nzumax, nzlumax) + n;
	    printf("Not enough memory to perform factorization .. "
		   "need %.1f GBytes\n", t*1e-9);
	    fflush(stdout);
	    return (t);
	}
	
    } else { /* refact == YES */
	Lstore   = L->Store;
	Ustore   = U->Store;
	xsup     = Lstore->sup_to_colbeg;
	xsup_end = Lstore->sup_to_colend;
	supno    = Lstore->col_to_sup;
	xlsub    = Lstore->rowind_colbeg;
	xlsub_end= Lstore->rowind_colend;
	xlusup   = Lstore->nzval_colbeg;
	xlusup_end= Lstore->nzval_colend;
	xusub    = Ustore->colbeg;
	xusub_end= Ustore->colend;
	nzlmax   = Glu->nzlmax;    /* max from previous factorization */
	nzumax   = Glu->nzumax;
	nzlumax  = Glu->nzlumax;
	
	if ( lwork == -1 ) {
	    return (GluIntArray(n) * iword + superlu_sTempSpace(n, panel_size, nprocs)
		    + (nzlmax+nzumax)*iword + (nzlumax+nzumax)*dword);
        } else if ( lwork == 0 ) {
	    whichspace = SYSTEM;
	} else {
	    whichspace = USER;
	    stack.size = lwork;
	    stack.top2 = lwork;
	}
	
	lsub  = sexpanders[LSUB].mem  = Lstore->rowind;
	lusup = sexpanders[LUSUP].mem = Lstore->nzval;
	usub  = sexpanders[USUB].mem  = Ustore->rowind;
	ucol  = sexpanders[UCOL].mem  = Ustore->nzval;;

	sexpanders[LSUB].size         = nzlmax;
	sexpanders[LUSUP].size        = nzlumax;
	sexpanders[USUB].size         = nzumax;
	sexpanders[UCOL].size         = nzumax;	
    }

    Glu->xsup       = xsup;
    Glu->xsup_end   = xsup_end;
    Glu->supno      = supno;
    Glu->lsub       = lsub;
    Glu->xlsub      = xlsub;
    Glu->xlsub_end  = xlsub_end;
    Glu->lusup      = lusup;
    Glu->xlusup     = xlusup;
    Glu->xlusup_end = xlusup_end;
    Glu->ucol       = ucol;
    Glu->usub       = usub;
    Glu->xusub      = xusub;
    Glu->xusub_end  = xusub_end;
    Glu->nzlmax     = nzlmax;
    Glu->nzumax     = nzumax;
    Glu->nzlumax    = nzlumax;
    ++no_expand;

#if ( PRNTlevel>=1 )
    printf(".. psgstrf_MemInit() refact %d, whichspace %d, nzlumax " IFMT ", nzumax " IFMT ", nzlmax " IFMT "\n",
	refact, whichspace, nzlumax, nzumax, nzlmax);
    printf(".. psgstrf_MemInit() FILL_LUSUP " IFMT ", FILL_UCOL " IFMT ", FILL_LSUB " IFMT "\n",
	FILL_LUSUP, FILL_UCOL, FILL_LSUB);
    fflush(stdout);
#endif

    return 0;
    
} /* psgstrf_MemInit */

/* 
 * Allocate known working storage. Returns 0 if success, otherwise
 * returns the number of bytes allocated so far when failure occurred.
 */
int_t
psgstrf_WorkInit(int_t n, int_t panel_size, int_t **iworkptr, float **dworkptr)
{
    int_t  isize, dsize, extra;
    float *old_ptr;
    int_t    maxsuper = sp_ienv(3),
           rowblk   = sp_ienv(4);

    isize = (2*panel_size + 5 + NO_MARKER) * n * sizeof(int_t);
    dsize = (n * panel_size +
	     NUM_TEMPV(n,panel_size,maxsuper,rowblk)) * sizeof(float);
    
    if ( whichspace == SYSTEM ) 
	*iworkptr = (int_t *) intCalloc(isize/sizeof(int_t));
    else
	*iworkptr = (int_t *) suser_malloc(isize, TAIL);
    if ( ! *iworkptr ) {
	fprintf(stderr, "psgstrf_WorkInit: malloc fails for local iworkptr[]\n");
	return (isize + n);
    }

    if ( whichspace == SYSTEM )
	*dworkptr = (float *) SUPERLU_MALLOC((size_t) dsize);
    else {
	    *dworkptr = (float *) suser_malloc(dsize, TAIL);
	    if ( NotDoubleAlign(*dworkptr) ) {
	        old_ptr = *dworkptr;
	        *dworkptr = (float*) DoubleAlign(*dworkptr);
	        *dworkptr = (float*) ((double*)*dworkptr - 1);
	        extra = (char*)old_ptr - (char*)*dworkptr;
#if ( DEBUGlevel>=1 )
	        printf("psgstrf_WorkInit: not aligned, extra" IFMT "\n", extra);
#endif	    
#if ( MACH==PTHREAD ) /* Use pthread ... */
        pthread_mutex_lock( &stack.lock );
#elif ( MACH==OPENMP ) /* Use openMP ... */
#pragma omp critical ( STACK_LOCK )
#endif
              {
	        stack.top2 -= extra;
	        stack.used += extra;
	      }
#if ( MACH==PTHREAD ) /* Use pthread ... */
        pthread_mutex_unlock( &stack.lock );
#endif
	    }
    } /* else */
    if ( ! *dworkptr ) {
	printf("malloc fails for local dworkptr[] ... dsize " IFMT "\n", dsize);
	return (isize + dsize + n);
    }
	
    return 0;
}


/*
 * Set up pointers for real working arrays.
 */
void
psgstrf_SetRWork(int_t n, int_t panel_size, float *dworkptr,
		 float **dense, float **tempv)
{
    float zero = 0.0;

    int_t maxsuper = sp_ienv(3);
    int_t rowblk   = sp_ienv(4);
    *dense = dworkptr;
    *tempv = *dense + panel_size*n;
    sfill (*dense, n * panel_size, zero);
    sfill (*tempv, NUM_TEMPV(n,panel_size,maxsuper,rowblk), zero);     
}
	
/*
 * Free the working storage used by factor routines.
 */
void psgstrf_WorkFree(int_t *iwork, float *dwork, GlobalLU_t *Glu)
{
    if ( whichspace == SYSTEM ) {
	SUPERLU_FREE (iwork);
	SUPERLU_FREE (dwork);
    } else {
#if ( MACH==PTHREAD ) /* Use pthread ... */
        pthread_mutex_lock( &stack.lock );
#elif ( MACH==OPENMP ) /* Use openMP ... */
#pragma omp critical ( STACK_LOCK )
#endif
        {
	    stack.used -= (stack.size - stack.top2);
	    stack.top2 = stack.size;
	    
	    /*	psgstrf_StackCompress(Glu);  */
        }
#if ( MACH==PTHREAD ) /* Use pthread ... */
        pthread_mutex_unlock( &stack.lock );
#endif
    }
}

/* 
 * Expand the data structures for L and U during the factorization.
 * Return value:   0 - successful return
 *               > 0 - number of bytes allocated when run out of space
 *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * !! Warning: Not Implemented in SuperLU_MT !!
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */
int_t
psgstrf_MemXpand(
		 int_t jcol,
		 int_t next, /* number of elements currently in the factors */
		 MemType mem_type,/* which type of memory to expand  */
		 int_t *maxlen, /* modified - max. length of a data structure */
		 GlobalLU_t *Glu /* modified - global LU data structures */
		 )
{
    void   *new_mem;
    
#ifdef CHK_EXPAND    
    printf("psgstrf_MemXpand(): jcol " IFMT ", next " IFMT ", maxlen " IFMT ", MemType " IFMT "\n",
	   jcol, next, *maxlen, mem_type);
#endif    

    if (mem_type == USUB) 
    	new_mem = psgstrf_expand(maxlen, mem_type, next, 1, Glu);
    else
	new_mem = psgstrf_expand(maxlen, mem_type, next, 0, Glu);
    
    if ( !new_mem ) {
	int_t    nzlmax  = Glu->nzlmax;
	int_t    nzumax  = Glu->nzumax;
	int_t    nzlumax = Glu->nzlumax;
    	fprintf(stderr, "Can't expand MemType %d : jcol " IFMT "\n",
                mem_type, jcol);
    	return (psgstrf_memory_use(nzlmax, nzumax, nzlumax) + ndim);
    }

    switch ( mem_type ) {
      case LUSUP:
	Glu->lusup   = (float *) new_mem;
	Glu->nzlumax = *maxlen;
	break;
      case UCOL:
	Glu->ucol   = (float *) new_mem;
	Glu->nzumax = *maxlen;
	break;
      case LSUB:
	Glu->lsub   = (int_t *) new_mem;
	Glu->nzlmax = *maxlen;
	break;
      case USUB:
	Glu->usub   = (int_t *) new_mem;
	Glu->nzumax = *maxlen;
	break;
    }
    
    return 0;
    
}


void
copy_mem_float(int_t howmany, void *old, void *new)
{
    register int_t i;
    float *dold = old;
    float *dnew = new;
    for (i = 0; i < howmany; i++) dnew[i] = dold[i];
}


/*
 * Expand the existing storage to accommodate more fill-ins.
 */
void
*psgstrf_expand(
                int_t *prev_len,   /* length used from previous call */
                MemType type,    /* which part of the memory to expand */
                int_t len_to_copy, /* size of memory to be copied to new store */
                int_t keep_prev,   /* = 1: use prev_len;
                                    = 0: compute new_len to expand */
                GlobalLU_t *Glu  /* modified - global LU data structures */
                )
{
    double   alpha = EXPAND;
    void     *new_mem, *old_mem;
    int_t      new_len, tries, lword, extra, bytes_to_copy;
    void     *ret = NULL;

    if ( no_expand == 0 || keep_prev ) /* First time allocate requested */
        new_len = *prev_len;
    else {
        new_len = alpha * *prev_len;
    }

    if ( type == LSUB || type == USUB ) lword = sizeof(int_t);
    else lword = sizeof(float);

    if ( whichspace == SYSTEM ) {
        new_mem = (void *) SUPERLU_MALLOC( (size_t) new_len * lword );

        if ( no_expand != 0 ) {
            tries = 0;
            if ( keep_prev ) {
                if ( !new_mem ) return (NULL);
            } else {
                while ( !new_mem ) {
                    if ( ++tries > 10 ) return (NULL);
                    alpha = Reduce(alpha);
                    new_len = alpha * *prev_len;
                    new_mem = (void *) SUPERLU_MALLOC((size_t) new_len * lword);
                }
            }
            if ( type == LSUB || type == USUB ) {
                copy_mem_int(len_to_copy, sexpanders[type].mem, new_mem);
            } else {
                copy_mem_float(len_to_copy, sexpanders[type].mem, new_mem);
            }
            SUPERLU_FREE (sexpanders[type].mem);
        }
        sexpanders[type].mem = (void *) new_mem;

    } else { /* whichspace == USER */
        if ( no_expand == 0 ) {
            new_mem = suser_malloc(new_len * lword, HEAD);
            if ( NotDoubleAlign(new_mem) &&
                (type == LUSUP || type == UCOL) ) {
                old_mem = new_mem;
                new_mem = (void *)DoubleAlign(new_mem);
                extra = (char*)new_mem - (char*)old_mem;
#ifdef CHK_EXPAND
                printf("expand(): not aligned, extra " IFMT "\n", extra);
#endif
#if ( MACH==PTHREAD ) /* Use pthread ... */
      pthread_mutex_lock( &stack.lock );
#elif ( MACH==OPENMP ) /* Use openMP ... */
#pragma omp critical ( STACK_LOCK )
#endif
              {
                stack.top1 += extra;
                stack.used += extra;
              }
#if ( MACH==PTHREAD ) /* Use pthread ... */
      pthread_mutex_unlock( &stack.lock );
#endif
            }
            sexpanders[type].mem = (void *) new_mem;
        } else {
            tries = 0;
            extra = (new_len - *prev_len) * lword;
            if ( keep_prev ) {
                if ( StackFull(extra) ) {
                    new_len = 0;
                    sexpanders[type].mem = NULL;
		    return NULL;
                }
            } else {
                while ( StackFull(extra) ) {
                    if ( ++tries > 10 ) {
                        new_len = 0;
                        sexpanders[type].mem = NULL;
			return NULL;
		    }
                    alpha = Reduce(alpha);
                    new_len = alpha * *prev_len;
                    extra = (new_len - *prev_len) * lword;
                }
            }

            if ( type != USUB ) {
                new_mem = (void*)((char*)sexpanders[type + 1].mem + extra);
                bytes_to_copy = (char*)stack.array + stack.top1
                    - (char*)sexpanders[type + 1].mem;
                user_bcopy(sexpanders[type+1].mem, new_mem, bytes_to_copy);

                if ( type < USUB ) {
                    Glu->usub = sexpanders[USUB].mem =
                        (void*)((char*)sexpanders[USUB].mem + extra);
                }
                if ( type < LSUB ) {
                    Glu->lsub = sexpanders[LSUB].mem =
                        (void*)((char*)sexpanders[LSUB].mem + extra);
                }
                if ( type < UCOL ) {
                    Glu->ucol = sexpanders[UCOL].mem =
                        (void*)((char*)sexpanders[UCOL].mem + extra);
                }
                stack.top1 += extra;
                stack.used += extra;
                if ( type == UCOL ) {
                    stack.top1 += extra;   /* Add same amount for USUB */
                    stack.used += extra;
                }

            } /* if ... */
        } /* else ... */

    } /* else, whichspace == USER */

#ifdef DEBUG
    printf("psgstrf_expand[type " IFMT "]\n", type);
#endif
    sexpanders[type].size = new_len;
    *prev_len = new_len;
    if ( no_expand ) ++no_expand;

    return (void *) sexpanders[type].mem;
  
} /* expand */


/*
 * Compress the work[] array to remove fragmentation.
 */
void
psgstrf_StackCompress(GlobalLU_t *Glu)
{
    register int_t iword, dword;
    char     *last, *fragment;
    int_t      *ifrom, *ito;
    float   *dfrom, *dto;
    int_t      *xlsub, *lsub, *xusub_end, *usub, *xlusup;
    float   *ucol, *lusup;
    
    iword = sizeof(int_t);
    dword = sizeof(float);

    xlsub  = Glu->xlsub;
    lsub   = Glu->lsub;
    xusub_end  = Glu->xusub_end;
    usub   = Glu->usub;
    xlusup = Glu->xlusup;
    ucol   = Glu->ucol;
    lusup  = Glu->lusup;
    
    dfrom = ucol;
    dto = (float *)((char*)lusup + xlusup[ndim] * dword);
    copy_mem_float(xusub_end[ndim-1], dfrom, dto);
    ucol = dto;

    ifrom = lsub;
    ito = (int_t *) ((char*)ucol + xusub_end[ndim-1] * iword);
    copy_mem_int(xlsub[ndim], ifrom, ito);
    lsub = ito;
    
    ifrom = usub;
    ito = (int_t *) ((char*)lsub + xlsub[ndim] * iword);
    copy_mem_int(xusub_end[ndim-1], ifrom, ito);
    usub = ito;
    
    last = (char*)usub + xusub_end[ndim-1] * iword;
    fragment = (char*) ((char*)stack.array + stack.top1 - last);
    stack.used -= (long long int) fragment;
    stack.top1 -= (long long int) fragment;

    Glu->ucol = ucol;
    Glu->lsub = lsub;
    Glu->usub = usub;
    
#ifdef CHK_EXPAND
    printf("psgstrf_StackCompress: fragment " IFMT "\n", fragment);
    /* PrintStack("After compress", Glu);
    for (last = 0; last < ndim; ++last)
	print_lu_col("After compress:", last, 0);*/
#endif    
    
}


/*
 * Allocate storage for original matrix A
 */
void
sallocateA(int_t n, int_t nnz, float **a, int_t **asub, int_t **xa)
{
    *a    = (float *) floatMalloc(nnz);
    *asub = (int_t *) intMalloc(nnz);
    *xa   = (int_t *) intMalloc(n+1);
}

float *floatMalloc(int_t n)
{
    float *buf;
    buf = (float *) SUPERLU_MALLOC( (size_t) n * sizeof(float) ); 
    if ( !buf ) {
	fprintf(stderr, "SUPERLU_MALLOC failed for buf in floatMalloc()");
	exit (1);
    }
    return (buf);
}

float *floatCalloc(int_t n)
{
    float *buf;
    register int_t i;
    float zero = 0.0;
    buf = (float *) SUPERLU_MALLOC( (size_t) n * sizeof(float) );
    if ( !buf ) {
	fprintf(stderr, "SUPERLU_MALLOC failed for buf in floatCalloc()");
	exit (1);
    }
    for (i = 0; i < n; ++i) buf[i] = zero;
    return (buf);
}

/*
 * Set up memory image in lusup[*], using the supernode boundaries in 
 * the Householder matrix.
 * 
 * In both static and dynamic scheme, the relaxed supernodes (leaves) 
 * are stored in the beginning of lusup[*]. In the static scheme, the
 * memory is also set aside for the internal supernodes using upper
 * bound information from H. In the dynamic scheme, however, the memory
 * for the internal supernodes is not allocated by this routine.
 *
 * Return value
 *   o Static scheme: number of nonzeros of all the supernodes in H.
 *   o Dynamic scheme: number of nonzeros of the relaxed supernodes. 
 */
int_t
sPresetMap(
	  const int_t n,
	  SuperMatrix *A, /* original matrix permuted by columns */
	  pxgstrf_relax_t *pxgstrf_relax, /* relaxed supernodes */
	  superlumt_options_t *superlumt_options, /* input */
	  GlobalLU_t *Glu /* modified */
	  )
{
    register int_t i, j, k, w, rs, rs_lastcol, krow, kmark, maxsup, nextpos;
    register int_t rs_nrow; /* number of nonzero rows in a relaxed supernode */
    int_t          *marker, *asub, *xa_begin, *xa_end;
    NCPformat    *Astore;
    int_t *map_in_sup; /* memory mapping function; values irrelevant on entry. */
    int_t *colcnt;     /* column count of Lc or H */
    int_t *super_bnd;  /* supernodes partition in H */
    char *snode_env, *getenv();

    snode_env = getenv("SuperLU_DYNAMIC_SNODE_STORE");
    if ( snode_env != NULL ) {
	Glu->dynamic_snode_bound = YES;
#if ( PRNTlevel>=1 )
	printf(".. Use dynamic alg. to allocate storage for L supernodes.\n");
#endif
    } else  Glu->dynamic_snode_bound = NO;

    Astore   = A->Store;
    asub     = Astore->rowind;
    xa_begin = Astore->colbeg;
    xa_end   = Astore->colend;
    rs       = 1;
    marker   = intMalloc(n);
    ifill(marker, n, EMPTY);
    map_in_sup = Glu->map_in_sup = intCalloc(n+1);
    colcnt = superlumt_options->colcnt_h;
    super_bnd = superlumt_options->part_super_h;
    nextpos = 0;

    /* Split large supernode into smaller pieces */
    maxsup = sp_ienv(3);
    for (j = 0; j < n; ) {
	w = super_bnd[j];
	k = j + w;
	if ( w > maxsup ) {
	    w = w % maxsup;
	    if ( w == 0 ) w = maxsup;
	    while ( j < k ) {
		super_bnd[j] = w;
		j += w;
		w = maxsup;
	    }
	}
	j = k;
    }
    
    for (j = 0; j < n; j += w) {
        if ( Glu->dynamic_snode_bound == NO ) map_in_sup[j] = nextpos;

	if ( pxgstrf_relax[rs].fcol == j ) {
	    /* Column j starts a relaxed supernode. */
	    map_in_sup[j] = nextpos;
	    rs_nrow = 0;
	    w = pxgstrf_relax[rs++].size;
	    rs_lastcol = j + w;
	    for (i = j; i < rs_lastcol; ++i) {
		/* for each nonzero in A[*,i] */
		for (k = xa_begin[i]; k < xa_end[i]; k++) {	
		    krow = asub[k];
		    kmark = marker[krow];
		    if ( kmark != j ) { /* first time visit krow */
			marker[krow] = j;
			++rs_nrow;
		    }
		}
	    }
	    nextpos += w * rs_nrow;
	    
	    /* Find the next H-supernode, with leading column i, which is
	       outside the relaxed supernode, rs. */
	    for (i = j; i < rs_lastcol; k = i, i += super_bnd[i]);
	    if ( i > rs_lastcol ) {
		/* The w columns [rs_lastcol, i) may join in the
		   preceeding relaxed supernode; make sure we leave
		   enough room for the combined supernode. */
		w = i - rs_lastcol;
		nextpos += w * SUPERLU_MAX( rs_nrow, colcnt[k] );
	    }
	    w = i - j;
	} else { /* Column j starts a supernode in H */
	    w = super_bnd[j];
	    if ( Glu->dynamic_snode_bound == NO ) nextpos += w * colcnt[j];
	}

	/* Set up the offset (negative) to the leading column j of a
	   supernode in H */ 
	for (i = 1; i < w; ++i) map_in_sup[j + i] = -i;
	
    } /* for j ... */

    if ( Glu->dynamic_snode_bound == YES ) Glu->nextlu = nextpos;
    else map_in_sup[n] = nextpos;

#if ( PRNTlevel>=1 )
    printf("** PresetMap() allocates " IFMT " reals to lusup[*]....\n", nextpos);
#endif

    free (marker);
    return nextpos;
}



