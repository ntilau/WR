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
      RECURSIVE SUBROUTINE 
     &    SMUMPS_PROCESS_ROOT2SON( COMM_LOAD, ASS_IRECV, 
     &    INODE, NELIM_ROOT, root, 
     &
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU,
     &    LRLU, LRLUS, N, IW, LIW, A, LA, PTRIST,
     &    PTLUST_S, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER, NSTK_S, COMP,
     &    IFLAG, IERROR, COMM,
     &    NBPROCFILS, IPOOL, LPOOL, LEAF, NBFIN, MYID, SLAVEF,
     &
     &    OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD, PTRARW, PTRAIW,
     &    INTARR,DBLARR,ICNTL,KEEP,KEEP8,DKEEP,ND,FRERE,
     &    LPTRAR, NELT, FRTPTR, FRTELT, 
     &    ISTEP_TO_INIV2, TAB_POS_IN_PERE
     &               , LRGROUPS
     &     )
      USE SMUMPS_STRUC_DEF, ONLY : SMUMPS_ROOT_STRUC
      IMPLICIT NONE
      INCLUDE 'mpif.h'
      TYPE (SMUMPS_ROOT_STRUC) :: root
      INTEGER KEEP(500), ICNTL( 60 )
      INTEGER(8) KEEP8(150)
      REAL       DKEEP(230)
      INTEGER COMM_LOAD, ASS_IRECV
      INTEGER INODE, NELIM_ROOT
      INTEGER LBUFR, LBUFR_BYTES
      INTEGER BUFR( LBUFR )
      INTEGER(8) :: LA, POSFAC, IPTRLU, LRLU, LRLUS
      INTEGER IWPOS, IWPOSCB
      INTEGER N, LIW
      INTEGER IW( LIW )
      REAL A( LA )
      INTEGER, intent(in) :: LRGROUPS(N)
      INTEGER(8) :: PTRAST(KEEP(28))
      INTEGER(8) :: PTRFAC(KEEP(28))
      INTEGER(8) :: PAMASTER(KEEP(28))
      INTEGER PTRIST(KEEP(28)), PTLUST_S(KEEP(28))
      INTEGER STEP(N), PIMASTER(KEEP(28))
      INTEGER COMP
      INTEGER NSTK_S( KEEP(28) ), PROCNODE_STEPS( KEEP(28) )
      INTEGER NBPROCFILS(KEEP(28))
      INTEGER IFLAG, IERROR, COMM
      INTEGER LPOOL, LEAF
      INTEGER IPOOL( LPOOL )
      INTEGER NELT, LPTRAR
      INTEGER FRTPTR( N+1 ), FRTELT( NELT )
      INTEGER MYID, SLAVEF, NBFIN
      DOUBLE PRECISION OPASSW, OPELIW
      INTEGER ITLOC( N + KEEP(253) ), FILS( N ), DAD(KEEP(28))
      REAL :: RHS_MUMPS(KEEP(255))
      INTEGER(8), INTENT(IN) :: PTRARW( LPTRAR ), PTRAIW( LPTRAR )
      INTEGER ND( KEEP(28) ), FRERE( KEEP(28) ) 
      INTEGER INTARR(KEEP8(27))
      REAL DBLARR(KEEP8(26))
      INTEGER ISTEP_TO_INIV2(KEEP(71)), 
     &        TAB_POS_IN_PERE(SLAVEF+2,max(1,KEEP(56)))
      INCLUDE 'mumps_tags.h'
      INCLUDE 'mumps_headers.h'
      INTEGER I, LCONT, NCOL_TO_SEND, LDA
      INTEGER(8) :: SHIFT_VAL_SON, POSELT
      INTEGER FPERE, IOLDPS, NFRONT, NPIV, NASS, NSLAVES,
     &        H_INODE, NELIM, NBCOL, LIST_NELIM_ROW, 
     &        LIST_NELIM_COL, NELIM_LOCAL, TYPE_SON, 
     &        NROW, NCOL, NBROW, SHIFT_LIST_ROW_SON,
     &        SHIFT_LIST_COL_SON, LDAFS, IERR,
     &        ISON, PDEST_MASTER_ISON
      INTEGER :: STATUS(MPI_STATUS_SIZE)
      LOGICAL BLOCKING, SET_IRECV, MESSAGE_RECEIVED
      INTEGER MSGSOU, MSGTAG
      LOGICAL TRANSPOSE_ASM
      INTEGER  MUMPS_PROCNODE, MUMPS_TYPENODE
      EXTERNAL MUMPS_PROCNODE, MUMPS_TYPENODE
      FPERE = KEEP(38)
      TYPE_SON = MUMPS_TYPENODE(PROCNODE_STEPS(STEP(INODE)),KEEP(199))
      IF ( MUMPS_PROCNODE( PROCNODE_STEPS(STEP(INODE)), 
     &     KEEP(199) ).EQ.MYID) THEN
       IOLDPS   = PTLUST_S(STEP(INODE))
       NFRONT   = IW(IOLDPS+KEEP(IXSZ))
       NPIV     = IW(IOLDPS+1+KEEP(IXSZ))
       NASS     = iabs(IW(IOLDPS + 2+KEEP(IXSZ)))
       NSLAVES  =  IW(IOLDPS+5+KEEP(IXSZ))
       H_INODE  = 6 + NSLAVES + KEEP(IXSZ)
       NELIM    = NASS - NPIV
       NBCOL = NFRONT - NPIV
       LIST_NELIM_ROW = IOLDPS + H_INODE + NPIV
       LIST_NELIM_COL = LIST_NELIM_ROW + NFRONT
       IF (NELIM.LE.0) THEN
            write(6,*) ' ERROR 1 in SMUMPS_PROCESS_ROOT2SON ', NELIM
            write(6,*) MYID,':Process root2son: INODE=',INODE,
     & 'Header=',IW(PTLUST_S(STEP(INODE)):PTLUST_S(STEP(INODE))
     &  +5+KEEP(IXSZ))
            CALL MUMPS_ABORT()
           ENDIF
       NELIM_LOCAL = NELIM_ROOT
       DO I=1, NELIM
        root%RG2L_ROW(IW(LIST_NELIM_ROW)) = NELIM_LOCAL
        root%RG2L_COL(IW(LIST_NELIM_COL)) = NELIM_LOCAL
        NELIM_LOCAL = NELIM_LOCAL + 1
        LIST_NELIM_ROW = LIST_NELIM_ROW + 1
        LIST_NELIM_COL = LIST_NELIM_COL + 1
       ENDDO
       NBROW = NFRONT - NPIV
       NROW = NELIM
       IF ( KEEP( 50 ) .eq. 0 ) THEN
         NCOL = NFRONT - NPIV
       ELSE
         NCOL = NELIM
       END IF
       SHIFT_LIST_ROW_SON = H_INODE + NPIV
       SHIFT_LIST_COL_SON = H_INODE + NFRONT + NPIV
       IF ( KEEP(50).eq.0 .OR. TYPE_SON .eq. 1 ) THEN
         LDAFS = NFRONT
       ELSE
         LDAFS = NASS
       END IF
       SHIFT_VAL_SON = int(NPIV,8) * int(LDAFS,8) + int(NPIV,8)
       CALL SMUMPS_BUILD_AND_SEND_CB_ROOT( COMM_LOAD,
     &   ASS_IRECV, 
     &   N, INODE, FPERE,
     &   PTLUST_S(1), PTRAST(1),
     &   root, NROW, NCOL, SHIFT_LIST_ROW_SON,
     &   SHIFT_LIST_COL_SON , SHIFT_VAL_SON, LDAFS,
     &   ROOT_NON_ELIM_CB, MYID, COMM,
     &   BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &   IWPOS, IWPOSCB, IPTRLU, LRLU, LRLUS, IW, LIW, A, LA,
     &   PTRIST, PTLUST_S(1), PTRFAC(1), PTRAST(1),
     &   STEP, PIMASTER, PAMASTER,
     &   NSTK_S, COMP, IFLAG, IERROR, NBPROCFILS,
     &   IPOOL, LPOOL, LEAF, NBFIN, SLAVEF,
     &   OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &   FILS, DAD, PTRARW, PTRAIW,
     &   INTARR,DBLARR,ICNTL,KEEP,KEEP8,DKEEP,.FALSE.,ND,FRERE,
     &   LPTRAR, NELT, FRTPTR, FRTELT, 
     &   ISTEP_TO_INIV2, TAB_POS_IN_PERE
     &               , LRGROUPS
     &    )
       IF (IFLAG.LT.0 ) RETURN
       IF (TYPE_SON.EQ.1) THEN
        NROW = NFRONT - NASS
        NCOL = NELIM
        SHIFT_LIST_ROW_SON = H_INODE + NASS
        SHIFT_LIST_COL_SON = H_INODE + NFRONT + NPIV
        SHIFT_VAL_SON      = int(NASS,8) * int(NFRONT,8) + int(NPIV,8)
        IF ( KEEP( 50 ) .eq. 0 ) THEN
          TRANSPOSE_ASM = .FALSE.
        ELSE
          TRANSPOSE_ASM = .TRUE.
        END IF
        CALL SMUMPS_BUILD_AND_SEND_CB_ROOT( COMM_LOAD, ASS_IRECV,
     &    N, INODE, FPERE,
     &    PTLUST_S, PTRAST,
     &    root, NROW, NCOL, SHIFT_LIST_ROW_SON,
     &    SHIFT_LIST_COL_SON , SHIFT_VAL_SON, NFRONT,
     &    ROOT_NON_ELIM_CB, MYID, COMM,
     &
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU, LRLU, LRLUS, IW, LIW, A, LA,
     &    PTRIST, PTLUST_S, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER,
     &    NSTK_S, COMP, IFLAG, IERROR, NBPROCFILS,
     &    IPOOL, LPOOL, LEAF, NBFIN, SLAVEF,
     &    OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD, PTRARW, PTRAIW,
     &    INTARR,DBLARR,ICNTL,KEEP,KEEP8,DKEEP,
     &    TRANSPOSE_ASM,ND,FRERE,
     &    LPTRAR, NELT, FRTPTR, FRTELT, 
     &    ISTEP_TO_INIV2, TAB_POS_IN_PERE, LRGROUPS )
        IF (IFLAG.LT.0 ) RETURN
       ENDIF
       IOLDPS = PTLUST_S(STEP(INODE))
       POSELT = PTRAST(STEP(INODE))
       IW(IOLDPS + 4+KEEP(IXSZ)) = STEP(INODE)
       PTRFAC(STEP(INODE))=POSELT
       IF ( TYPE_SON .eq. 1 ) THEN
         NBROW = NFRONT - NPIV
       ELSE
         NBROW = NELIM
       END IF
       IF ( TYPE_SON .eq. 1 .OR. KEEP(50).EQ.0) THEN
         LDA = NFRONT
       ELSE
         LDA = NPIV+NBROW
       ENDIF
       CALL SMUMPS_COMPACT_FACTORS(A(POSELT), LDA,
     &          NPIV, NBROW, KEEP(50),
     &          int(LDA,8)*int(NBROW+NPIV,8))
       IW(IOLDPS + KEEP(IXSZ))     = NBCOL
       IW(IOLDPS + 1 +KEEP(IXSZ)) = NASS - NPIV
       IF (TYPE_SON.EQ.2) THEN
        IW(IOLDPS + 2 +KEEP(IXSZ)) = NASS
       ELSE
        IW(IOLDPS + 2 +KEEP(IXSZ)) = NFRONT
       ENDIF
       IW(IOLDPS + 3 +KEEP(IXSZ)) = NPIV
      CALL SMUMPS_COMPRESS_LU(0_8,MYID,N,IOLDPS,TYPE_SON,IW,LIW,
     &    A, LA, POSFAC, LRLU, LRLUS,
     &    IWPOS, PTRAST,PTRFAC,STEP, KEEP,KEEP8, .FALSE.,INODE,IERR
     &    , LRGROUPS, NASS
     &    )
      IF(IERR.LT.0)THEN
         IFLAG=IERR
         IERROR=0
         RETURN
      ENDIF
      ELSE 
        ISON = INODE
        PDEST_MASTER_ISON = 
     &      MUMPS_PROCNODE( PROCNODE_STEPS(STEP(ISON)), KEEP(199) )
        IF ( PTRIST(STEP(ISON)) .EQ. 0) THEN
          CALL SMUMPS_TREAT_DESCBAND( ISON, COMM_LOAD,
     &    ASS_IRECV,
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU,
     &    LRLU, LRLUS, N, IW, LIW, A, LA, PTRIST,
     &    PTLUST_S, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER, NSTK_S, COMP,
     &    IFLAG, IERROR, COMM,
     &    NBPROCFILS, IPOOL, LPOOL, LEAF, NBFIN, MYID, SLAVEF,
     &
     &    root, OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD, PTRARW, PTRAIW,
     &    INTARR, DBLARR,ICNTL,KEEP,KEEP8,DKEEP,ND,FRERE,LPTRAR,
     &    NELT, FRTPTR, FRTELT,
     &    ISTEP_TO_INIV2, TAB_POS_IN_PERE, .TRUE.
     &               , LRGROUPS
     &   )
          IF ( IFLAG .LT. 0 ) RETURN
        ENDIF
        DO WHILE (
     &     ( IW( PTRIST(STEP(ISON)) + 1  +KEEP(IXSZ)) .NE.
     &       IW( PTRIST(STEP(ISON)) + 3  +KEEP(IXSZ)) ) .OR.
     &     ( KEEP(50) .NE. 0 .AND.
     &       IW( PTRIST(STEP(ISON)) + 6  +KEEP(IXSZ)) .NE. 0 ) )
          IF ( KEEP(50).eq.0) THEN
            MSGSOU = PDEST_MASTER_ISON
            MSGTAG = BLOC_FACTO
          ELSE
            IF ( IW( PTRIST(STEP(ISON)) + 1  +KEEP(IXSZ)) .NE.
     &           IW( PTRIST(STEP(ISON)) + 3  +KEEP(IXSZ)) ) THEN
              MSGSOU = PDEST_MASTER_ISON
              MSGTAG = BLOC_FACTO_SYM
            ELSE
              MSGSOU = MPI_ANY_SOURCE
              MSGTAG = BLOC_FACTO_SYM_SLAVE
            END IF
          END IF
          BLOCKING  = .TRUE.
          SET_IRECV = .FALSE.
          MESSAGE_RECEIVED = .FALSE.
          CALL SMUMPS_TRY_RECVTREAT( COMM_LOAD, ASS_IRECV,
     &    BLOCKING, SET_IRECV, MESSAGE_RECEIVED,
     &    MSGSOU, MSGTAG,
     &    STATUS,
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU,
     &    LRLU, LRLUS, N, IW, LIW, A, LA, PTRIST,
     &    PTLUST_S, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER, NSTK_S, COMP,
     &    IFLAG, IERROR, COMM,
     &    NBPROCFILS, IPOOL, LPOOL, LEAF, NBFIN, MYID, SLAVEF,
     &
     &    root, OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD, PTRARW, PTRAIW,
     &    INTARR,DBLARR,ICNTL,KEEP,KEEP8,DKEEP,ND, FRERE, LPTRAR,
     &    NELT, FRTPTR, FRTELT,
     &    ISTEP_TO_INIV2, TAB_POS_IN_PERE, .TRUE.
     &               , LRGROUPS
     &   )
          IF ( IFLAG .LT. 0 ) RETURN
        END DO
       IOLDPS = PTRIST(STEP(INODE))
       LCONT  = IW(IOLDPS+KEEP(IXSZ))
       NROW   = IW(IOLDPS+2+KEEP(IXSZ))
       NPIV   = IW(IOLDPS+3+KEEP(IXSZ))
       NASS   = IW(IOLDPS+4+KEEP(IXSZ))
       NELIM  = NASS-NPIV
       IF (NELIM.LE.0) THEN
         write(6,*) MYID,': INODE,LCONT, NROW, NPIV, NASS, NELIM=',
     &   INODE,LCONT, NROW, NPIV, NASS, NELIM
         write(6,*) MYID,': IOLDPS=',IOLDPS
         write(6,*) MYID,': ERROR 2 in SMUMPS_PROCESS_ROOT2SON '
         CALL MUMPS_ABORT()
       ENDIF
       NSLAVES= IW(IOLDPS+5+KEEP(IXSZ))
       H_INODE = 6 + NSLAVES + KEEP(IXSZ)
       LIST_NELIM_COL = IOLDPS + H_INODE + NROW + NPIV
       NELIM_LOCAL = NELIM_ROOT
       DO I = 1, NELIM
        root%RG2L_COL(IW(LIST_NELIM_COL)) = NELIM_LOCAL
        root%RG2L_ROW(IW(LIST_NELIM_COL)) = NELIM_LOCAL
        NELIM_LOCAL = NELIM_LOCAL + 1
        LIST_NELIM_COL = LIST_NELIM_COL + 1
       ENDDO
       SHIFT_LIST_ROW_SON = 6 + IW(IOLDPS+5+KEEP(IXSZ)) + KEEP(IXSZ)
       SHIFT_LIST_COL_SON = SHIFT_LIST_ROW_SON + NROW + NPIV
       NCOL_TO_SEND       = NELIM
       LDA = -9999
       SHIFT_VAL_SON = -9999_8
       IF ( KEEP( 50 ) .eq. 0 ) THEN
         TRANSPOSE_ASM = .FALSE.
       ELSE
         TRANSPOSE_ASM = .TRUE.
       END IF
       CALL SMUMPS_BUILD_AND_SEND_CB_ROOT( COMM_LOAD, ASS_IRECV, 
     &    N, INODE, FPERE,
     &    PTRIST, PTRAST,
     &    root, NROW, NCOL_TO_SEND, SHIFT_LIST_ROW_SON,
     &    SHIFT_LIST_COL_SON , SHIFT_VAL_SON, LDA,
     &    ROOT_NON_ELIM_CB, MYID, COMM,
     &
     &    BUFR, LBUFR, LBUFR_BYTES, PROCNODE_STEPS, POSFAC,
     &    IWPOS, IWPOSCB, IPTRLU, LRLU, LRLUS, IW, LIW, A, LA,
     &    PTRIST, PTLUST_S, PTRFAC,
     &    PTRAST, STEP, PIMASTER, PAMASTER,
     &    NSTK_S, COMP, IFLAG, IERROR, NBPROCFILS,
     &    IPOOL, LPOOL, LEAF, NBFIN, SLAVEF,
     &    OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &    FILS, DAD, PTRARW, PTRAIW,
     &    INTARR,DBLARR,ICNTL,KEEP,KEEP8,DKEEP,TRANSPOSE_ASM,
     &    ND, FRERE, LPTRAR, NELT, FRTPTR, FRTELT, 
     &   ISTEP_TO_INIV2, TAB_POS_IN_PERE, LRGROUPS)
        IF (IFLAG.LT.0 ) RETURN
       IF (KEEP(214).EQ.2) THEN
        CALL SMUMPS_STACK_BAND( N, INODE,
     &      PTRIST, PTRAST, PTLUST_S, PTRFAC, IW, LIW, A, LA,
     &      LRLU, LRLUS, IWPOS, IWPOSCB, POSFAC, COMP,
     &      IPTRLU, OPELIW, STEP, PIMASTER, PAMASTER,
     &      IFLAG, IERROR, SLAVEF, PROCNODE_STEPS, DAD, MYID, COMM,
     &      KEEP, KEEP8, DKEEP,TYPE_SON
     &      )
       ENDIF
        IF (IFLAG.LT.0) THEN
           CALL SMUMPS_BDC_ERROR( MYID, SLAVEF, COMM, KEEP )
        ENDIF
      ENDIF
      RETURN
      END SUBROUTINE SMUMPS_PROCESS_ROOT2SON
