C
C  This file is part of MUMPS 5.3.1, released
C  on Fri Apr 10 13:52:30 UTC 2020
C
C
C  Copyright 1991-2020 CERFACS, CNRS, ENS Lyon, INP Toulouse, Inria,
C  Mumps Technologies, University of Bordeaux.
C
C  This version of MUMPS is provided to you free of charge. It is
C  released under the CeCILL-C license:
C  http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
C
      SUBROUTINE SMUMPS_PROCESS_ROOT2SLAVE( TOT_ROOT_SIZE,
     &    TOT_CONT_TO_RECV, root,
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU,
     &    LRLU, LRLUS, N, IW, LIW, A, LA, PTRIST,
     &    PTLUST, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER, NSTK_S, COMP,
     &    IFLAG, IERROR, COMM, COMM_LOAD,
     &    NBPROCFILS,
     &    IPOOL, LPOOL, LEAF,
     &    NBFIN, MYID, SLAVEF,
     &
     &    OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD,
     &    LPTRAR, NELT, FRTPTR, FRTELT, 
     &    PTRARW, PTRAIW,
     &    INTARR, DBLARR, ICNTL, KEEP, KEEP8, DKEEP, ND)
      USE SMUMPS_LOAD
      USE SMUMPS_OOC        
      USE SMUMPS_STRUC_DEF, ONLY : SMUMPS_ROOT_STRUC
      IMPLICIT NONE
      INCLUDE 'mpif.h'
      TYPE (SMUMPS_ROOT_STRUC) :: root
      INTEGER KEEP(500), ICNTL(60)
      INTEGER(8) KEEP8(150)
      REAL DKEEP(230)
      INTEGER TOT_ROOT_SIZE, TOT_CONT_TO_RECV
      INTEGER LBUFR, LBUFR_BYTES
      INTEGER BUFR( LBUFR )
      INTEGER(8) :: IPTRLU, LRLU, LRLUS, LA, POSFAC
      INTEGER(8) :: PTRFAC(KEEP(28)), PTRAST(KEEP(28))
      INTEGER(8) :: PAMASTER(KEEP(28))
      INTEGER IWPOS, IWPOSCB
      INTEGER N, LIW
      INTEGER IW( LIW )
      REAL A( LA )
      INTEGER PTRIST(KEEP(28)), PTLUST(KEEP(28))
      INTEGER STEP(N), PIMASTER(KEEP(28))
      INTEGER COMP
      INTEGER NSTK_S( KEEP(28) ), PROCNODE_STEPS( KEEP(28) )
      INTEGER NBPROCFILS( KEEP(28) ), ND( KEEP(28) )
      INTEGER IFLAG, IERROR, COMM, COMM_LOAD
      INTEGER LPOOL, LEAF
      INTEGER IPOOL( LPOOL )
      INTEGER MYID, SLAVEF, NBFIN
      DOUBLE PRECISION OPASSW, OPELIW
      INTEGER ITLOC(N+KEEP(253)), FILS(N), DAD(KEEP(28))
      INTEGER LPTRAR, NELT
      INTEGER FRTPTR( N+1 ), FRTELT( NELT )
      INTEGER(8), INTENT(IN) :: PTRARW(LPTRAR), PTRAIW(LPTRAR)
      REAL :: RHS_MUMPS(KEEP(255))
      INTEGER INTARR(KEEP8(27))
      REAL DBLARR(KEEP8(26))
      INTEGER ::  allocok
      REAL, DIMENSION(:,:), POINTER :: TMP
      INTEGER NEW_LOCAL_M, NEW_LOCAL_N
      INTEGER OLD_LOCAL_M, OLD_LOCAL_N
      INTEGER I, J
      INTEGER LREQI, IROOT
      INTEGER(8) :: LREQA
      INTEGER POSHEAD, IPOS_SON,IERR
      LOGICAL MASTER_OF_ROOT, NO_OLD_ROOT
      REAL ZERO
      PARAMETER( ZERO = 0.0E0 )
      INCLUDE 'mumps_headers.h'
      INTEGER numroc, MUMPS_PROCNODE
      EXTERNAL numroc, MUMPS_PROCNODE
      IROOT = KEEP( 38 )
      root%TOT_ROOT_SIZE = TOT_ROOT_SIZE
      MASTER_OF_ROOT = ( MYID .EQ. 
     &                   MUMPS_PROCNODE( PROCNODE_STEPS(STEP(IROOT)),
     &                   KEEP(199) ) )
      NEW_LOCAL_M  = numroc( TOT_ROOT_SIZE, root%MBLOCK,
     &               root%MYROW, 0, root%NPROW )
      NEW_LOCAL_M  = max( 1, NEW_LOCAL_M )
      NEW_LOCAL_N  = numroc( TOT_ROOT_SIZE, root%NBLOCK,
     &               root%MYCOL, 0, root%NPCOL )
      IF ( PTRIST(STEP( IROOT )).GT.0) THEN
        OLD_LOCAL_N = -IW( PTRIST(STEP( IROOT )) + KEEP(IXSZ) )
        OLD_LOCAL_M =  IW( PTRIST(STEP( IROOT )) + 1  + KEEP(IXSZ))
      ELSE
        OLD_LOCAL_N = 0
        OLD_LOCAL_M = NEW_LOCAL_M
      ENDIF
      IF (PTRIST(STEP(IROOT)) .EQ.0) THEN
        NO_OLD_ROOT = .TRUE.
      ELSE
        NO_OLD_ROOT =.FALSE.
      ENDIF
      IF (KEEP(60) .NE. 0) THEN
        IF ( MASTER_OF_ROOT ) THEN
          LREQI=6+2*TOT_ROOT_SIZE+KEEP(IXSZ)
          LREQA=0_8
          IF ( IWPOS + LREQI - 1. GT. IWPOSCB ) THEN
           CALL SMUMPS_COMPRE_NEW( N, KEEP(28), IW, LIW, A, LA,
     &           LRLU, IPTRLU,
     &           IWPOS, IWPOSCB, PTRIST, PTRAST,
     &           STEP, PIMASTER, PAMASTER, KEEP(216),LRLUS,
     &           KEEP(IXSZ),COMP,DKEEP(97),
     &           MYID, SLAVEF, KEEP(199), PROCNODE_STEPS, DAD )
           IF ( LRLU .NE. LRLUS ) THEN
                  WRITE(*,*) 'PB1 compress root2slave:LRLU,LRLUS=',
     &            LRLU, LRLUS
                  IFLAG = -9
                  CALL MUMPS_SET_IERROR(LREQA-LRLUS, IERROR)
                  GOTO 700
           END IF
          ENDIF
          IF ( IWPOS + LREQI - 1. GT. IWPOSCB ) THEN
            IFLAG = -8
            IERROR = IWPOS + LREQI - 1 - IWPOSCB
            GOTO 700
          ENDIF
          PTLUST(STEP(IROOT))= IWPOS
          IWPOS = IWPOS + LREQI
          POSHEAD = PTLUST( STEP(IROOT))
          IW( POSHEAD + XXI )=LREQI
          CALL MUMPS_STOREI8( LREQA, IW(POSHEAD + XXR) )
          CALL MUMPS_STOREI8( 0_8, IW(POSHEAD + XXD) )
          IW( POSHEAD + XXS )=-9999
          IW(POSHEAD+XXS+1:POSHEAD+KEEP(IXSZ)-1)=-99999
          IW( POSHEAD +KEEP(IXSZ)) = 0
          IW( POSHEAD + 1 +KEEP(IXSZ)) = -1
          IW( POSHEAD + 2 +KEEP(IXSZ)) = -1
          IW( POSHEAD + 4 +KEEP(IXSZ)) = STEP(IROOT)
          IW( POSHEAD + 5 +KEEP(IXSZ)) = 0
          IW( POSHEAD + 3 +KEEP(IXSZ)) = TOT_ROOT_SIZE
        ELSE 
          PTLUST(STEP(IROOT)) = -4444
        ENDIF
        PTRIST(STEP(IROOT)) = 0
        PTRFAC(STEP(IROOT)) = -4445_8
        IF (root%yes .and. NO_OLD_ROOT) THEN
          IF (NEW_LOCAL_N .GT. 0) THEN
            CALL SMUMPS_SET_TO_ZERO(root%SCHUR_POINTER(1),
     &      root%SCHUR_LLD, root%SCHUR_MLOC, root%SCHUR_NLOC,
     &      KEEP)
            IF (KEEP(55).EQ.0) THEN
              CALL SMUMPS_ASM_ARR_ROOT( N, root, IROOT,
     &        root%SCHUR_POINTER(1), root%SCHUR_LLD, root%SCHUR_MLOC,
     &        root%SCHUR_NLOC, FILS, PTRAIW, PTRARW, INTARR, DBLARR,
     &        KEEP8(27), KEEP8(26), MYID )
            ELSE
              CALL SMUMPS_ASM_ELT_ROOT(N, root,
     &        root%SCHUR_POINTER(1), root%SCHUR_LLD, root%SCHUR_MLOC,
     &        root%SCHUR_NLOC, LPTRAR, NELT, FRTPTR, FRTELT,
     &        PTRAIW, PTRARW, INTARR, DBLARR,
     &        KEEP8(27), KEEP8(26), KEEP, KEEP8, MYID )
            ENDIF
          ENDIF
        ENDIF
      ELSE
        IF ( MASTER_OF_ROOT ) THEN
          LREQI = 6 + 2 * TOT_ROOT_SIZE+KEEP(IXSZ)
        ELSE
          LREQI = 6+KEEP(IXSZ)
        END IF
        LREQA = int(NEW_LOCAL_M, 8) * int(NEW_LOCAL_N, 8)
        CALL SMUMPS_GET_SIZE_NEEDED( 
     &           LREQI , LREQA, .FALSE.,
     &           KEEP(1), KEEP8(1),
     &           N, KEEP(28), IW, LIW, A, LA,
     &           LRLU, IPTRLU,
     &           IWPOS, IWPOSCB, PTRIST, PTRAST,
     &           STEP, PIMASTER, PAMASTER, KEEP(216), LRLUS,
     &           KEEP(IXSZ), COMP, DKEEP(97),
     &           MYID, SLAVEF, PROCNODE_STEPS, DAD, 
     &           IFLAG, IERROR )
        IF (IFLAG.LT.0) GOTO 700
        PTLUST(STEP( IROOT )) = IWPOS
        IWPOS           = IWPOS + LREQI
        IF (LREQA.EQ.0_8) THEN
          PTRAST (STEP(IROOT)) = POSFAC
          PTRFAC (STEP(IROOT)) = POSFAC
        ELSE
          PTRAST (STEP(IROOT)) = POSFAC
          PTRFAC (STEP(IROOT)) = POSFAC
        ENDIF
        POSFAC           = POSFAC + LREQA
        LRLU   = LRLU  - LREQA
        LRLUS  = LRLUS - LREQA
        KEEP8(67) = min(KEEP8(67), LRLUS)
        KEEP8(69) = KEEP8(69) + LREQA
        KEEP8(68) = max(KEEP8(69), KEEP8(68))
        CALL SMUMPS_LOAD_MEM_UPDATE(.FALSE.,.FALSE.,
     &            LA-LRLUS,0_8,LREQA,KEEP,KEEP8,LRLUS)
        POSHEAD = PTLUST( STEP(IROOT))
        IW( POSHEAD + XXI )     = LREQI
        CALL MUMPS_STOREI8( LREQA, IW(POSHEAD + XXR))
        CALL MUMPS_STOREI8( 0_8, IW(POSHEAD + XXD))
        IW( POSHEAD + XXS ) = S_NOTFREE
        IW(POSHEAD+XXS+1:POSHEAD+KEEP(IXSZ)-1)=-99999
        IW( POSHEAD + KEEP(IXSZ) ) = 0
        IW( POSHEAD + 1 + KEEP(IXSZ) ) = NEW_LOCAL_N
        IW( POSHEAD + 2 + KEEP(IXSZ) ) = NEW_LOCAL_M
        IW( POSHEAD + 4 + KEEP(IXSZ) ) = STEP(IROOT)
        IW( POSHEAD + 5 + KEEP(IXSZ) ) = 0
        IF ( MASTER_OF_ROOT ) THEN
          IW( POSHEAD + 3 + KEEP(IXSZ) ) = TOT_ROOT_SIZE
        ELSE
          IW( POSHEAD + 3 + KEEP(IXSZ) ) = 0
        ENDIF
        IF ( PTRIST(STEP(IROOT)) .EQ. 0) THEN
          CALL SMUMPS_SET_TO_ZERO(A(PTRAST(STEP(IROOT))),
     &    NEW_LOCAL_M, NEW_LOCAL_M, NEW_LOCAL_N, KEEP)
          IF (KEEP(55) .EQ.0 ) THEN
            CALL SMUMPS_ASM_ARR_ROOT( N, root, IROOT,
     &      A(PTRAST(STEP(IROOT))),
     &      NEW_LOCAL_M, NEW_LOCAL_M, NEW_LOCAL_N,
     &      FILS, PTRAIW, PTRARW, INTARR, DBLARR,
     &      KEEP8(27), KEEP8(26), MYID )
          ELSE
            CALL SMUMPS_ASM_ELT_ROOT( N, root,
     &      A(PTRAST(STEP(IROOT))),
     &      NEW_LOCAL_M, NEW_LOCAL_M, NEW_LOCAL_N,
     &      LPTRAR, NELT, FRTPTR, FRTELT,
     &      PTRAIW, PTRARW, INTARR, DBLARR,
     &      KEEP8(27), KEEP8(26), KEEP, KEEP8, MYID )
          ENDIF
          PAMASTER(STEP(IROOT)) = 0_8
        ELSE IF ( PTRIST(STEP(IROOT)) .LT. 0 ) THEN
          CALL SMUMPS_SET_TO_ZERO(A(PTRAST(STEP(IROOT))),
     &    NEW_LOCAL_M, NEW_LOCAL_M, NEW_LOCAL_N, KEEP)
        ELSE
          OLD_LOCAL_N = -IW( PTRIST(STEP( IROOT )) + KEEP(IXSZ) )
          OLD_LOCAL_M =  IW( PTRIST(STEP( IROOT )) + 1  + KEEP(IXSZ))
          IF ( TOT_ROOT_SIZE .eq. root%ROOT_SIZE ) THEN
            IF ( LREQA .NE. int(OLD_LOCAL_M,8) * int(OLD_LOCAL_N,8) )
     &            THEN
              write(*,*) 'error 1 in PROCESS_ROOT2SLAVE',
     &        OLD_LOCAL_M, OLD_LOCAL_N
              CALL MUMPS_ABORT()
            END IF
            CALL SMUMPS_COPYI8SIZE(LREQA,
     &                            A( PAMASTER(STEP(IROOT)) ),
     &                            A( PTRAST  (STEP(IROOT)) ) )
          ELSE
            CALL SMUMPS_COPY_ROOT( A( PTRAST(STEP(IROOT))), 
     &          NEW_LOCAL_M,
     &          NEW_LOCAL_N, A( PAMASTER( STEP(IROOT)) ), OLD_LOCAL_M,
     &          OLD_LOCAL_N )
          END IF
          IF ( PTRIST( STEP( IROOT ) ) .GT. 0 ) THEN
             IPOS_SON= PTRIST( STEP(IROOT))
             CALL SMUMPS_FREE_BLOCK_CB_STATIC(.FALSE.,
     &            MYID, N, IPOS_SON,
     &            IW, LIW, LRLU, LRLUS, IPTRLU,
     &            IWPOSCB, LA, KEEP,KEEP8, .FALSE.
     &           )
          END IF
        ENDIF 
        PTRIST(STEP( IROOT ))   = 0
        PAMASTER(STEP( IROOT )) = 0_8
      ENDIF 
      IF ( NO_OLD_ROOT ) THEN
          IF (KEEP(253) .GT.0) THEN
            root%RHS_NLOC = numroc( KEEP(253), root%NBLOCK,
     &                      root%MYCOL, 0, root%NPCOL )
            root%RHS_NLOC = max( root%RHS_NLOC, 1 )
          ELSE
            root%RHS_NLOC = 1
          ENDIF
          IF (associated(root%RHS_ROOT)) DEALLOCATE(root%RHS_ROOT)
          ALLOCATE(root%RHS_ROOT(NEW_LOCAL_M, root%RHS_NLOC),
     &              stat=allocok)
          IF ( allocok.GT.0 ) THEN
             IFLAG = -13
             IERROR = NEW_LOCAL_N * root%RHS_NLOC
            GOTO 700
          ENDIF
          IF (KEEP(253) .NE. 0) THEN
            root%RHS_ROOT=ZERO
            CALL SMUMPS_ASM_RHS_ROOT( N, FILS, root, KEEP, RHS_MUMPS,
     &      IFLAG, IERROR )
          ENDIF
      ELSE IF (NEW_LOCAL_M.GT.OLD_LOCAL_M .AND. KEEP(253) .GT.0) THEN
          TMP => root%RHS_ROOT
          NULLIFY(root%RHS_ROOT)
          ALLOCATE (root%RHS_ROOT(NEW_LOCAL_M, root%RHS_NLOC), 
     &                stat=allocok)
          IF ( allocok.GT.0) THEN
              IFLAG=-13
              IERROR = NEW_LOCAL_M*root%RHS_NLOC
              GOTO 700
          ENDIF
          DO J = 1, root%RHS_NLOC
            DO I = 1, OLD_LOCAL_M
              root%RHS_ROOT(I,J)=TMP(I,J)
            ENDDO
            DO I = OLD_LOCAL_M+1, NEW_LOCAL_M
              root%RHS_ROOT(I,J) = ZERO
            ENDDO
          ENDDO
          DEALLOCATE(TMP)
          NULLIFY(TMP) 
      ENDIF
      NBPROCFILS(STEP(IROOT))=NBPROCFILS(STEP(IROOT)) + TOT_CONT_TO_RECV
#if ! defined(NO_XXNBPR)
      KEEP(121) = KEEP(121) + TOT_CONT_TO_RECV
#endif
#if ! defined(NO_XXNBPR)
      CALL CHECK_EQUAL(NBPROCFILS(STEP(IROOT)), KEEP(121))
      IF ( KEEP(121) .eq. 0 ) THEN
#else
      IF ( NBPROCFILS(STEP(IROOT)) .eq. 0 ) THEN
#endif
         IF (KEEP(201).EQ.1) THEN 
            CALL SMUMPS_OOC_FORCE_WRT_BUF_PANEL(IERR)
         ELSE IF (KEEP(201).EQ.2) THEN 
            CALL SMUMPS_FORCE_WRITE_BUF(IERR)              
         ENDIF
        CALL SMUMPS_INSERT_POOL_N( N, IPOOL, LPOOL, PROCNODE_STEPS,
     &       SLAVEF, KEEP(199), KEEP(28), KEEP(76), KEEP(80), KEEP(47),
     &       STEP, IROOT + N )
        IF (KEEP(47) .GE. 3) THEN
           CALL SMUMPS_LOAD_POOL_UPD_NEW_POOL(
     &          IPOOL, LPOOL, 
     &          PROCNODE_STEPS, KEEP,KEEP8, SLAVEF, COMM_LOAD,
     &          MYID, STEP, N, ND, FILS )
        ENDIF
      END IF
      RETURN
 700  CONTINUE
      CALL SMUMPS_BDC_ERROR( MYID, SLAVEF, COMM, KEEP )
      RETURN
      END SUBROUTINE SMUMPS_PROCESS_ROOT2SLAVE
      SUBROUTINE SMUMPS_COPY_ROOT
     &( NEW, M_NEW, N_NEW,OLD, M_OLD, N_OLD )
      INTEGER M_NEW, N_NEW, M_OLD, N_OLD
      REAL NEW( M_NEW, N_NEW ), OLD( M_OLD, N_OLD )
      INTEGER J
      REAL ZERO
      PARAMETER( ZERO = 0.0E0 )
      DO J = 1, N_OLD
        NEW( 1: M_OLD, J ) = OLD( 1: M_OLD, J )
        NEW( M_OLD + 1: M_NEW, J ) = ZERO
      END DO
      NEW( 1: M_NEW,N_OLD + 1: N_NEW ) = ZERO
      RETURN
      END SUBROUTINE SMUMPS_COPY_ROOT
