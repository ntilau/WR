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
      SUBROUTINE CMUMPS_FAC_B( N, S_IS_POINTERS, LA, LIW,
     & SYM_PERM, NA, LNA,
     & NE_STEPS, NFSIZ, FILS,
     & STEP, FRERE, DAD, CAND, 
     & ISTEP_TO_INIV2, TAB_POS_IN_PERE, 
     & PTRAR, LDPTRAR,
     & PTRIST, PTLUST_S, PTRFAC, IW1, IW2, ITLOC, RHS_MUMPS,
     & POOL, LPOOL,  
     & CNTL1, ICNTL, INFO, RINFO, KEEP,KEEP8,PROCNODE_STEPS,
     & SLAVEF,
     & COMM_NODES, MYID, MYID_NODES,
     & BUFR, LBUFR, LBUFR_BYTES, CMUMPS_LBUF, INTARR, DBLARR,
     & root, NELT, FRTPTR, FRTELT, COMM_LOAD,
     & ASS_IRECV, SEUIL, SEUIL_LDLT_NIV2,
     & MEM_DISTRIB,
     & DKEEP,PIVNUL_LIST,LPN_LIST
     &       ,LRGROUPS
     &     )
      USE CMUMPS_DYNAMIC_MEMORY_M, ONLY: CMUMPS_DM_FAC_UPD_DYN_MEMCNTS
      USE CMUMPS_LOAD 
      USE CMUMPS_BUF, ONLY : CMUMPS_BUF_ALLOC_CB, CMUMPS_BUF_DEALL_CB
      USE CMUMPS_FAC_S_IS_POINTERS_M, ONLY : S_IS_POINTERS_T
      USE CMUMPS_STRUC_DEF, ONLY : CMUMPS_ROOT_STRUC
      IMPLICIT NONE
      INCLUDE 'mpif.h'
      TYPE (CMUMPS_ROOT_STRUC) :: root
      INTEGER(8) :: LA
      INTEGER N,LIW,LPOOL,SLAVEF,COMM_NODES
      INTEGER MYID, MYID_NODES,LNA
      TYPE (S_IS_POINTERS_T) :: S_IS_POINTERS
      REAL RINFO(40)
      INTEGER, INTENT( IN ) :: LBUFR, LBUFR_BYTES
      INTEGER :: BUFR( LBUFR )
      INTEGER, INTENT( IN ) :: CMUMPS_LBUF
      INTEGER, DIMENSION(0:SLAVEF - 1) :: MEM_DISTRIB
      INTEGER NELT, LDPTRAR
      INTEGER FRTPTR(*), FRTELT(*)
      INTEGER LRGROUPS(N)
      REAL CNTL1
      INTEGER   ICNTL(60)
      INTEGER   INFO(80), KEEP(500)
      INTEGER(8) KEEP8(150)
      INTEGER   SYM_PERM(N), NA(LNA),
     &          NE_STEPS(KEEP(28)), FILS(N),
     &          FRERE(KEEP(28)), NFSIZ(KEEP(28)), 
     &          DAD(KEEP(28))
      INTEGER   CAND(SLAVEF+1, max(1,KEEP(56)))
      INTEGER   STEP(N)
      INTEGER(8), INTENT(IN) :: PTRAR(LDPTRAR,2)
      INTEGER(8) :: PTRFAC(KEEP(28))
      INTEGER   PTRIST(KEEP(28)), PTLUST_S(KEEP(28))
      INTEGER   IW1(3*KEEP(28)), ITLOC(N+KEEP(253)), POOL(LPOOL)
      COMPLEX :: RHS_MUMPS(KEEP(255))
      INTEGER(8) :: IW2(2*KEEP(28))
      INTEGER   PROCNODE_STEPS(KEEP(28))
      INTEGER   COMM_LOAD, ASS_IRECV
      INTEGER   ISTEP_TO_INIV2(KEEP(71)), 
     &          TAB_POS_IN_PERE(SLAVEF+2,max(1,KEEP(56)))
      COMPLEX   DBLARR(KEEP8(26))
      INTEGER   INTARR(KEEP8(27))
      REAL SEUIL, SEUIL_LDLT_NIV2
      INTEGER LPN_LIST
      INTEGER PIVNUL_LIST(LPN_LIST)
      REAL DKEEP(230)
       INTEGER, EXTERNAL :: MUMPS_PROCNODE
      INTEGER allocok
      REAL UULOC
      INTEGER IERR
      INTEGER LP, MPRINT
      LOGICAL LPOK
      INTEGER NSTK,PTRAST, NBPROCFILS
      INTEGER PIMASTER, PAMASTER
      LOGICAL PROK
      REAL ZERO, ONE
      DATA ZERO /0.0E0/
      DATA ONE /1.0E0/
      INTEGER :: NSTEPSDONE
      DOUBLE PRECISION :: OPASS, OPELI
      INTEGER :: NELVA, COMP
      INTEGER :: MAXFRT, NTOTPV, NMAXNPIV, NOFFNEGPV
      INTEGER :: NB22T1, NB22T2, NBTINY, DET_EXP, DET_SIGN
      COMPLEX :: DET_MANT
      INTEGER :: NTOTPVTOT
      INTEGER(8) :: POSFAC, LRLU, IPTRLU, LRLUS
      INTEGER IWPOS, LEAF, NBROOT, NROOT
      INTEGER    :: LIW_ARG_FAC_PAR
      INTEGER(8) :: LA_ARG_FAC_PAR
      COMPLEX, TARGET:: CDUMMY(1)
      INTEGER, TARGET :: IDUMMY(1)
      LOGICAL :: IW_DUMMY, A_DUMMY
      KEEP(41)=0
      KEEP(42)=0
      LP     = ICNTL(1)
      LPOK   = (LP.GT.0) .AND. (ICNTL(4).GE.1)
      MPRINT = ICNTL(2)
      PROK   = (MPRINT.GT.0) .AND. (ICNTL(4).GE.2)
      UULOC = CNTL1
      IF (UULOC.GT.ONE)   UULOC=ONE
      IF (UULOC.LT.ZERO)  UULOC=ZERO
      IF (KEEP(50).NE.0.AND.UULOC.GT.0.5E0) THEN
        UULOC = 0.5E0
      ENDIF
      PIMASTER   = 1
      NSTK       = PIMASTER + KEEP(28)
      NBPROCFILS = NSTK + KEEP(28)
      PTRAST = 1
      PAMASTER = 1 + KEEP(28)
      IF (KEEP(4).LE.0) KEEP(4)=32
      IF (KEEP(5).LE.0) KEEP(5)=16
      IF (KEEP(5).GT.KEEP(4)) KEEP(5) = KEEP(4)
      IF (KEEP(6).LE.0) KEEP(6)=24
      IF (KEEP(3).LE.KEEP(4)) KEEP(3)=KEEP(4)*2
      IF (KEEP(6).GT.KEEP(3)) KEEP(6) = KEEP(3)
      POSFAC = 1_8
      IWPOS  = 1
      LRLU = LA
      LRLUS = LRLU
      KEEP8(63) = 0_8
      KEEP8(64) = 0_8
      KEEP8(65) = 0_8
      KEEP8(66) = 0_8
      KEEP8(68) = 0_8
      KEEP8(69) = 0_8
      KEEP8(70) = 0_8
      KEEP8(71) = 0_8
      KEEP8(73) = 0_8
      KEEP8(74) = 0_8
      IPTRLU = LRLU
      NSTEPSDONE = 0
      OPASS      = 0.0D0
      OPELI      = 0.0D0
      NELVA      = 0
      COMP       = 0
      MAXFRT     = 0  
      NMAXNPIV   = 0
      NTOTPV     = 0
      NOFFNEGPV  = 0
      NB22T1     = 0  
      NB22T2     = 0  
      NBTINY     = 0  
      DET_EXP    = 0
      DET_SIGN   = 1
      DET_MANT   = cmplx(1.0E0,0.0E0, kind=kind(1.0E0))
      IW1(NSTK:NSTK+KEEP(28)-1) = NE_STEPS(1:KEEP(28))
      CALL MUMPS_INIT_NROOT_DIST(N, NBROOT, NROOT,
     &                     MYID_NODES,
     &                     SLAVEF, NA, LNA,
     &                     KEEP, STEP,
     &                     PROCNODE_STEPS)
        CALL MUMPS_INIT_POOL_DIST(N, LEAF,
     &                     MYID_NODES,
     &                     SLAVEF, NA, LNA,
     &                     KEEP,KEEP8, STEP,
     &                     PROCNODE_STEPS,
     &                     POOL, LPOOL)
        CALL CMUMPS_INIT_POOL_LAST3(POOL, LPOOL, LEAF)     
      CALL CMUMPS_LOAD_INIT_SBTR_STRUCT(POOL, LPOOL,KEEP,KEEP8)
      IF ( KEEP( 38 ) .NE. 0 ) THEN
        NBROOT = NBROOT + root%NPROW * root%NPCOL - 1
      END IF
      IF ( root%yes )  THEN 
         IF ( MUMPS_PROCNODE( PROCNODE_STEPS(STEP(KEEP(38))),
     &                                       KEEP(199) )
     &         .NE. MYID_NODES ) THEN
             NROOT = NROOT + 1
         END IF
      END IF
      PTRIST(1:KEEP(28))=0
      PTLUST_S(1:KEEP(28))=0
      PTRFAC(1:KEEP(28))=-99999_8
      IW2(PTRAST:PTRAST+KEEP(28)-1)=0_8
      IW1(PIMASTER:PIMASTER+KEEP(28)-1)=-99999_8
      KEEP(405) = 0
      IF (associated(S_IS_POINTERS%IW)) THEN
        WRITE(*,*) " Internal error CMUMPS_FAC_B IW"
        CALL MUMPS_ABORT()
      ENDIF
      IF (INFO(1) .GE. 0 ) THEN
        ALLOCATE(S_IS_POINTERS%IW(LIW), stat=allocok)
        IF (allocok .GT.0) THEN
          INFO(1) = -13
          INFO(2) = LIW
          IF (LPOK) THEN
            WRITE(LP,*) 
     &     'Allocation error for id%IS(',LIW,') on worker',
     &      MYID_NODES
          ENDIF
        ENDIF
      ENDIF
      IF (INFO(1) .GE. 0) THEN
        IF (.NOT. associated(S_IS_POINTERS%A)) THEN
          ALLOCATE(S_IS_POINTERS%A(LA), stat=allocok)
          IF (allocok .GT. 0) THEN
            INFO(1) = -13
            CALL MUMPS_SETI8TOI4(LA, INFO(2))
            DEALLOCATE(S_IS_POINTERS%IW); NULLIFY(S_IS_POINTERS%IW)
            KEEP8(23)=0_8
          ELSE
            KEEP8(23)=LA
          ENDIF
        ENDIF
      ENDIF
      IF (INFO(1) .GE. 0) THEN
        CALL CMUMPS_BUF_ALLOC_CB( CMUMPS_LBUF, IERR )
        IF ( IERR .NE. 0 ) THEN
          INFO(1)= -13
          INFO(2)= (CMUMPS_LBUF+KEEP(34)-1)/KEEP(34)
          IF (LPOK) THEN
            WRITE(LP,*) 
     &     'Allocation error in CMUMPS_BUF_ALLOC_CB'
     &     ,INFO(2), ' on worker', MYID_NODES
          ENDIF
          DEALLOCATE(S_IS_POINTERS%IW); NULLIFY(S_IS_POINTERS%IW)
          DEALLOCATE(S_IS_POINTERS%A); NULLIFY(S_IS_POINTERS%A)
        END IF
      ENDIF
      IW_DUMMY = .FALSE.
      A_DUMMY = .FALSE.
      IF (INFO(1) .GE. 0) THEN
        LIW_ARG_FAC_PAR = LIW
        LA_ARG_FAC_PAR  = LA
      ELSE
        LIW_ARG_FAC_PAR = 1
        LA_ARG_FAC_PAR  = 1_8
        IF (.NOT. associated(S_IS_POINTERS%IW)) THEN
          S_IS_POINTERS%IW => IDUMMY
          IW_DUMMY = .TRUE.
        ENDIF
        IF (.NOT. associated(S_IS_POINTERS%A)) THEN
          S_IS_POINTERS%A  => CDUMMY
          A_DUMMY = .TRUE.
        ENDIF
      ENDIF
      IF ( INFO(1) .LT. 0 ) THEN
        CALL CMUMPS_BDC_ERROR( MYID_NODES, SLAVEF, COMM_NODES, KEEP )
      ENDIF
      CALL CMUMPS_FAC_PAR_I(N,S_IS_POINTERS%IW(1),LIW_ARG_FAC_PAR,
     &     S_IS_POINTERS%A(1),LA_ARG_FAC_PAR,IW1(NSTK),IW1(NBPROCFILS),
     &     NFSIZ,FILS,STEP,FRERE, DAD, CAND,
     &     ISTEP_TO_INIV2, TAB_POS_IN_PERE,
     &     NSTEPSDONE, OPASS, OPELI, NELVA, COMP,
     &     MAXFRT, NMAXNPIV, NTOTPV, NOFFNEGPV,
     &     NB22T1, NB22T2, NBTINY, DET_EXP, DET_MANT, DET_SIGN,
     &     PTRIST, IW2(PTRAST),
     &     IW1(PIMASTER), IW2(PAMASTER), PTRAR(1,2), 
     &     PTRAR(1,1),          
     &     ITLOC, RHS_MUMPS,
     &     POOL, LPOOL,
     &     RINFO, POSFAC,IWPOS,LRLU,IPTRLU, 
     &     LRLUS, LEAF, NROOT, NBROOT,
     &     UULOC,ICNTL,PTLUST_S,PTRFAC,INFO,
     &     KEEP,KEEP8, PROCNODE_STEPS,SLAVEF,MYID,COMM_NODES,
     &     MYID_NODES, BUFR, LBUFR, LBUFR_BYTES,
     &     INTARR, DBLARR, root, SYM_PERM,
     &     NELT, FRTPTR, FRTELT, LDPTRAR, 
     &     COMM_LOAD, ASS_IRECV, SEUIL, SEUIL_LDLT_NIV2,
     &     MEM_DISTRIB,NE_STEPS,
     &     DKEEP(1),PIVNUL_LIST(1),LPN_LIST
     &     ,LRGROUPS(1)
     &     )
      IF (IW_DUMMY) THEN
        NULLIFY( S_IS_POINTERS%IW )
      ENDIF
      IF (A_DUMMY) THEN
        NULLIFY( S_IS_POINTERS%A )
      ENDIF
      CALL CMUMPS_BUF_DEALL_CB( IERR )
      RINFO(2)  = real(OPASS)
      RINFO(3)  = real(OPELI)
      INFO(13)  = NELVA
      INFO(14)  = COMP
      KEEP(33)  = MAXFRT; INFO(11)  = MAXFRT
      KEEP(246) = NMAXNPIV
      KEEP(89)  = NTOTPV; INFO(23)  = NTOTPV
      INFO(12)  = NOFFNEGPV
      KEEP(103) = NB22T1
      KEEP(105) = NB22T2
      KEEP(98)  = NBTINY
      KEEP(260) = KEEP(260) * DET_SIGN
      KEEP(259) = KEEP(259) + DET_EXP
      CALL CMUMPS_UPDATEDETER( DET_MANT, DKEEP(6), KEEP(259) )
      POSFAC = POSFAC -1_8
      IWPOS = IWPOS -1
      IF (KEEP(201).LE.0) THEN
        IF (KEEP(201) .EQ. -1 .AND. INFO(1) .LT. 0) THEN
          POSFAC = 0_8
        ENDIF
        KEEP8(31) = POSFAC 
         RINFO(6)  = ZERO
      ELSE
         RINFO(6)  = real(KEEP8(31)*int(KEEP(35),8))/1E6
      ENDIF
      KEEP8(48) = KEEP8(31)+KEEP8(71)+KEEP8(64)
      KEEP(32) = IWPOS
      CALL MUMPS_SETI8TOI4(KEEP8(48), INFO(9))
      INFO(10) = KEEP(32)
      KEEP8(67) = LA - KEEP8(67)
      CALL MPI_ALLREDUCE(NTOTPV, NTOTPVTOT, 1, MPI_INTEGER, MPI_SUM, 
     &                COMM_NODES, IERR)
      IF ( ( (INFO(1).EQ.-10 .OR. INFO(1).EQ.-40)
     &       .AND. (NTOTPVTOT.EQ.N) )
     &     .OR. ( NTOTPVTOT.GT.N ) ) THEN
       write(*,*) ' Error 1 NTOTPVTOT=', NTOTPVTOT,N
       CALL MUMPS_ABORT()
      ENDIF
      IF ( (KEEP(19).NE.0 ) .AND. (NTOTPVTOT.NE.N) .AND. 
     &     (INFO(1).GE.0) )  THEN
       write(*,*) ' Error 2 NTOTPVTOT=', NTOTPVTOT 
       CALL MUMPS_ABORT()
      ENDIF
      IF ( (INFO(1) .GE. 0 ) 
     &      .AND. (NTOTPVTOT.NE.N) ) THEN
         INFO(1) = -10
      ENDIF
      IF (INFO(1).EQ.-10) THEN
         INFO(2) = NTOTPVTOT
      ENDIF
      IF (PROK) THEN
        WRITE (MPRINT,99980) INFO(1), INFO(2),
     &       KEEP(28), KEEP8(48), INFO(10), INFO(11)
        IF(KEEP(50) .EQ. 0) THEN
          WRITE(MPRINT,99982) INFO(12)
        ENDIF
        WRITE (MPRINT, 99986)
     &       INFO(13), INFO(14), INFO(25), RINFO(2), RINFO(3)
      ENDIF
      RETURN
99980 FORMAT (/' LEAVING FACTORIZATION PHASE WITH ...'/
     &      ' INFO (1)                                      =',I15/
     &      '  --- (2)                                      =',I15/
     &      '           Number of nodes in the tree         =',I15/
     &      ' INFO (9)  Real space for factors              =',I15/
     &      '  --- (10) Integer space for factors           =',I15/
     &      '  --- (11) Maximum size of frontal matrices    =',I15)
99982 FORMAT ('  --- (12) Number of off diagonal pivots       =',I15)
99986 FORMAT ('  --- (13) Number of delayed pivots            =',I15/
     &      '  --- (14) Number of memory compresses         =',I15/
     &      '  --- (25) Number of entries in factors        =',I15/
     &  ' RINFO(2)  Operations during node assembly     =',1PD10.3/
     &  ' -----(3)  Operations during node elimination  =',1PD10.3)
      END SUBROUTINE CMUMPS_FAC_B
      SUBROUTINE CMUMPS_FAC_PAR_I(N,IW,LIW,A,LA,
     &             NSTK_STEPS, NBPROCFILS,ND,FILS,STEP,
     &             FRERE, DAD, CAND,
     &             ISTEP_TO_INIV2, TAB_POS_IN_PERE,
     &             NSTEPSDONE, OPASS, OPELI, NELVA, COMP,
     &             MAXFRT, NMAXNPIV, NTOTPV, NOFFNEGPV,
     &             NB22T1, NB22T2, NBTINY, DET_EXP, DET_MANT, DET_SIGN,
     &             PTRIST, PTRAST,
     &             PIMASTER, PAMASTER, PTRARW, PTRAIW,
     &             ITLOC, RHS_MUMPS, IPOOL, LPOOL,
     &             RINFO, POSFAC ,IWPOS, LRLU, IPTRLU,
     &             LRLUS, LEAF, NBROOT, NBRTOT,
     &             UU, ICNTL, PTLUST, PTRFAC, INFO,
     &             KEEP,KEEP8,
     &             PROCNODE_STEPS,SLAVEF,MYID, COMM_NODES,
     &             MYID_NODES,
     &             BUFR,LBUFR,LBUFR_BYTES,INTARR,DBLARR,root,
     &             PERM, NELT, FRTPTR, FRTELT, LPTRAR,
     &             COMM_LOAD, ASS_IRECV, SEUIL, SEUIL_LDLT_NIV2,
     &             MEM_DISTRIB, NE, DKEEP,PIVNUL_LIST,LPN_LIST
     &             ,LRGROUPS
     &             )
      USE CMUMPS_LOAD
      USE CMUMPS_OOC
      USE CMUMPS_FAC_ASM_MASTER_M
      USE CMUMPS_FAC_ASM_MASTER_ELT_M
      USE CMUMPS_FAC1_LDLT_M
      USE CMUMPS_FAC2_LDLT_M
      USE CMUMPS_FAC1_LU_M
      USE CMUMPS_FAC2_LU_M
      USE CMUMPS_STRUC_DEF, ONLY : CMUMPS_ROOT_STRUC
      USE CMUMPS_FAC_PAR_M, ONLY : CMUMPS_FAC_PAR
      IMPLICIT NONE
      TYPE (CMUMPS_ROOT_STRUC) :: root
      INTEGER N, LIW, LPTRAR, NSTEPSDONE, INFO(80)
      DOUBLE PRECISION, INTENT(INOUT) :: OPASS, OPELI
      INTEGER, INTENT(INOUT) :: NELVA, COMP
      INTEGER, INTENT(INOUT) :: MAXFRT, NTOTPV, NMAXNPIV, NOFFNEGPV
      INTEGER, INTENT(INOUT) :: NB22T1, NB22T2, NBTINY
      INTEGER, INTENT(INOUT) :: DET_SIGN, DET_EXP
      COMPLEX, INTENT(INOUT) :: DET_MANT
      INTEGER(8) :: LA
      COMPLEX :: A(LA)
      INTEGER SLAVEF, COMM_NODES, MYID, MYID_NODES
      INTEGER, DIMENSION(0: SLAVEF - 1) :: MEM_DISTRIB
      INTEGER KEEP(500), ICNTL(60)
      INTEGER(8) KEEP8(150)
      INTEGER LPOOL
      INTEGER PROCNODE_STEPS(KEEP(28))
      INTEGER ITLOC(N+KEEP(253))
      COMPLEX :: RHS_MUMPS(KEEP(255))
      INTEGER IW(LIW), NSTK_STEPS(KEEP(28)), NBPROCFILS(KEEP(28))
      INTEGER(8), INTENT(IN) :: PTRARW(LPTRAR), PTRAIW(LPTRAR)
      INTEGER ND(KEEP(28))
      INTEGER FILS(N),PTRIST(KEEP(28))
      INTEGER STEP(N), FRERE(KEEP(28)), DAD(KEEP(28))
      INTEGER PIMASTER(KEEP(28))
      INTEGER PTLUST(KEEP(28)), PERM(N)
      INTEGER CAND(SLAVEF+1,max(1,KEEP(56)))
      INTEGER   ISTEP_TO_INIV2(KEEP(71)),
     &          TAB_POS_IN_PERE(SLAVEF+2,max(1,KEEP(56)))
      INTEGER IPOOL(LPOOL)
      INTEGER NE(KEEP(28))
      REAL RINFO(40)
      INTEGER(8) :: PAMASTER(KEEP(28)), PTRAST(KEEP(28))
      INTEGER(8) :: PTRFAC(KEEP(28))
      INTEGER(8) :: POSFAC, LRLU, LRLUS, IPTRLU
      INTEGER IWPOS, LEAF, NBROOT, NBRTOT
      INTEGER COMM_LOAD, ASS_IRECV
      REAL UU, SEUIL, SEUIL_LDLT_NIV2
      INTEGER NELT
      INTEGER FRTPTR( N+1 ), FRTELT( NELT )
      INTEGER LBUFR, LBUFR_BYTES
      INTEGER BUFR( LBUFR )
      COMPLEX DBLARR( KEEP8(26) )
      INTEGER INTARR( KEEP8(27) )
      INTEGER LPN_LIST
      INTEGER PIVNUL_LIST(LPN_LIST)
      REAL DKEEP(230)
      INTEGER LRGROUPS(N)
      CALL CMUMPS_FAC_PAR(N,IW,LIW,A,LA,
     &             NSTK_STEPS, NBPROCFILS,ND,FILS,STEP,
     &             FRERE, DAD, CAND,
     &             ISTEP_TO_INIV2, TAB_POS_IN_PERE,
     &             NSTEPSDONE, OPASS, OPELI, NELVA, COMP,
     &             MAXFRT, NMAXNPIV, NTOTPV, NOFFNEGPV,
     &             NB22T1, NB22T2, NBTINY, DET_EXP, DET_MANT, DET_SIGN,
     &             PTRIST, PTRAST,
     &             PIMASTER, PAMASTER, PTRARW, PTRAIW,
     &             ITLOC, RHS_MUMPS, IPOOL, LPOOL,
     &             RINFO, POSFAC ,IWPOS, LRLU, IPTRLU,
     &             LRLUS, LEAF, NBROOT, NBRTOT,
     &             UU, ICNTL, PTLUST, PTRFAC, INFO,
     &             KEEP,KEEP8,
     &             PROCNODE_STEPS,SLAVEF,MYID, COMM_NODES,
     &             MYID_NODES,
     &             BUFR,LBUFR,LBUFR_BYTES,INTARR,DBLARR,root,
     &             PERM, NELT, FRTPTR, FRTELT, LPTRAR,
     &             COMM_LOAD, ASS_IRECV, SEUIL, SEUIL_LDLT_NIV2,
     &             MEM_DISTRIB, NE, DKEEP,PIVNUL_LIST,LPN_LIST
     &             ,LRGROUPS
     &             )
      RETURN
      END SUBROUTINE CMUMPS_FAC_PAR_I
