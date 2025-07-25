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
      SUBROUTINE SMUMPS_PROCESS_CONTRIB_TYPE3(BUFR,LBUFR,
     &     LBUFR_BYTES,
     &     root, N, IW, LIW, A, LA,
     &     NBPROCFILS, LRLU, IPTRLU, IWPOS, IWPOSCB,
     &     PTRIST, PTLUST, PTRFAC, PTRAST, STEP, PIMASTER, PAMASTER,
     &     COMP, LRLUS, IPOOL, LPOOL, LEAF,
     &     FILS, DAD, MYID,
     &     LPTRAR, NELT, FRTPTR, FRTELT,
     &     PTRAIW, PTRARW, INTARR, DBLARR,
     &     KEEP, KEEP8, DKEEP, IFLAG, IERROR, COMM, COMM_LOAD,
     &     ITLOC, RHS_MUMPS,
     &     ND,PROCNODE_STEPS, SLAVEF, OPASSW )
      USE SMUMPS_LOAD
      USE SMUMPS_OOC
      USE SMUMPS_STRUC_DEF, ONLY : SMUMPS_ROOT_STRUC
      IMPLICIT NONE
      TYPE (SMUMPS_ROOT_STRUC ) :: root
      INTEGER    :: KEEP( 500 )
      INTEGER(8) :: KEEP8(150)
      REAL       :: DKEEP(230)
      INTEGER(8) :: LA, LRLU, IPTRLU, LRLUS
      INTEGER(8) :: PAMASTER(KEEP(28))
      INTEGER(8) :: PTRAST(KEEP(28))
      INTEGER(8) :: PTRFAC(KEEP(28))
      INTEGER LBUFR, LBUFR_BYTES, N, LIW,
     &        IWPOS, IWPOSCB, COMP, COMM, COMM_LOAD, IFLAG,
     &        IERROR
      INTEGER LPOOL, LEAF
      INTEGER IPOOL( LEAF )
      INTEGER PTRIST(KEEP(28))
      INTEGER PTLUST(KEEP(28))
      INTEGER STEP(N), PIMASTER(KEEP(28)), ITLOC( N+KEEP(253) )
      REAL :: RHS_MUMPS(KEEP(255))
      INTEGER BUFR( LBUFR_BYTES ), NBPROCFILS( KEEP(28) )
      INTEGER IW( LIW )
      INTEGER ND(KEEP(28)), PROCNODE_STEPS(KEEP(28))
      INTEGER SLAVEF
      REAL A( LA )
      INTEGER  MYID
      INTEGER FILS( N ), DAD(KEEP(28))
      INTEGER LPTRAR, NELT
      INTEGER FRTPTR( N+1 ), FRTELT( NELT )
      INTEGER(8), INTENT(IN) :: PTRAIW( LPTRAR ), PTRARW( LPTRAR )
      INTEGER INTARR(KEEP8(27))
      REAL DBLARR(KEEP8(26))
      DOUBLE PRECISION OPASSW
        INCLUDE 'mpif.h'
        INTEGER IERR
        EXTERNAL MUMPS_PROCNODE
        INTEGER MUMPS_PROCNODE
        INTEGER POSITION, LOCAL_M, LOCAL_N, LREQI
        INTEGER(8) :: LREQA, POS_ROOT
        INTEGER NSUBSET_ROW, NSUBSET_COL, IROOT, ISON, NSUBSET_COL_EFF
        INTEGER NSUPCOL_EFF
        INTEGER NBROWS_ALREADY_SENT, NBROWS_PACKET
        INTEGER NSUPROW, NSUPCOL, BBPCBP
        INCLUDE 'mumps_headers.h'
        POSITION = 0
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   ISON, 1, MPI_INTEGER, COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NSUBSET_ROW, 1, MPI_INTEGER, COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NSUPROW, 1, MPI_INTEGER, COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NSUBSET_COL, 1, MPI_INTEGER, COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NSUPCOL, 1, MPI_INTEGER, COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NBROWS_ALREADY_SENT, 1, MPI_INTEGER,
     &                   COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   NBROWS_PACKET, 1, MPI_INTEGER,
     &                   COMM, IERR )
        CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   BBPCBP, 1, MPI_INTEGER,
     &                   COMM, IERR )
        IF (BBPCBP .EQ. 1) THEN
          NSUBSET_COL_EFF = NSUBSET_COL - NSUPCOL
          NSUPCOL_EFF = 0
        ELSE
          NSUBSET_COL_EFF = NSUBSET_COL
          NSUPCOL_EFF = NSUPCOL
        ENDIF
        IROOT = KEEP( 38 )
        IF ( PTRIST( STEP(IROOT) ) .NE. 0 .OR.
     &       PTLUST( STEP(IROOT)) .NE. 0 ) THEN
          IF (NBROWS_ALREADY_SENT + NBROWS_PACKET .EQ. NSUBSET_ROW
     &       - NSUPROW .OR.  NSUBSET_ROW - NSUPROW.EQ.0 .OR.
     &       NSUBSET_COL_EFF .EQ. 0)THEN
            NBPROCFILS(STEP(IROOT)) = NBPROCFILS(STEP(IROOT))-1
#if ! defined(NO_XXNBPR)
            KEEP(121) = KEEP(121) - 1
            CALL CHECK_EQUAL(NBPROCFILS(STEP(IROOT)),KEEP(121))
            IF ( KEEP(121) .eq. 0 ) THEN
#else
            IF ( NBPROCFILS( STEP(IROOT) ) .eq. 0 ) THEN
#endif
              IF (KEEP(201).EQ.1) THEN 
                 CALL SMUMPS_OOC_FORCE_WRT_BUF_PANEL(IERR)
              ELSEIF (KEEP(201).EQ.2) THEN 
                 CALL SMUMPS_FORCE_WRITE_BUF(IERR)              
              ENDIF
              CALL SMUMPS_INSERT_POOL_N( N, IPOOL, LPOOL,
     &             PROCNODE_STEPS, SLAVEF, KEEP(199),
     &             KEEP(28), KEEP(76),
     &             KEEP(80), KEEP(47),
     &             STEP, IROOT + N)
              IF (KEEP(47) .GE. 3) THEN
                 CALL SMUMPS_LOAD_POOL_UPD_NEW_POOL(
     &                IPOOL, LPOOL, 
     &                PROCNODE_STEPS, KEEP,KEEP8, SLAVEF, COMM_LOAD,
     &                MYID, STEP, N, ND, FILS )
              ENDIF
            ENDIF
          ENDIF
        ELSE
           IF (NBROWS_ALREADY_SENT + NBROWS_PACKET .EQ.
     &       NSUBSET_ROW - NSUPROW .OR.
     &        NSUBSET_ROW - NSUPROW.EQ.0 .OR.
     &        NSUBSET_COL_EFF .EQ. 0)THEN
             NBPROCFILS(STEP( IROOT ) ) = -1
#if ! defined(NO_XXNBPR)
             KEEP(121)=-1
#endif
           ENDIF
           CALL SMUMPS_ROOT_ALLOC_STATIC( root, IROOT, N,
     &                IW, LIW, A, LA,
     &                FILS, DAD, MYID, SLAVEF, PROCNODE_STEPS,
     &                LPTRAR, NELT, FRTPTR, FRTELT,
     &                PTRAIW, PTRARW, INTARR, DBLARR,
     &                LRLU, IPTRLU,
     &                IWPOS, IWPOSCB, PTRIST, PTRAST,
     &                STEP, PIMASTER, PAMASTER, ITLOC, RHS_MUMPS,
     &                COMP, LRLUS, IFLAG, KEEP,KEEP8,DKEEP,IERROR )
            IF ( IFLAG .LT. 0 ) RETURN
        END IF
      IF (KEEP(60) .EQ.0) THEN
        IF ( PTRIST(STEP(IROOT)) .GE. 0 ) THEN
          IF ( PTRIST(STEP(IROOT)) .NE. 0 ) THEN
               LOCAL_N  = -IW( PTRIST(STEP( IROOT )) + KEEP(IXSZ)    )
               LOCAL_M  =  IW( PTRIST(STEP( IROOT )) + 1 + KEEP(IXSZ))
               POS_ROOT = PAMASTER(STEP( IROOT ))
          ELSE
               LOCAL_N = IW( PTLUST(STEP( IROOT ) ) + 1 + KEEP(IXSZ))
               LOCAL_M = IW( PTLUST(STEP( IROOT ) ) + 2 + KEEP(IXSZ))
               POS_ROOT = PTRFAC(IW(PTLUST(STEP(IROOT))+4+
     &                    KEEP(IXSZ)))
          END IF
         ENDIF
      ELSE
          LOCAL_M = root%SCHUR_LLD
          LOCAL_N = root%SCHUR_NLOC
      ENDIF
        IF ( (BBPCBP.EQ.1).AND. (NBROWS_ALREADY_SENT.EQ.0).AND.
     &     (min(NSUPROW, NSUPCOL) .GT. 0)
     &     ) THEN
         LREQI = NSUPROW+NSUPCOL
         LREQA = int(NSUPROW,8) * int(NSUPCOL,8)
         IF ( (LREQA.NE.0_8) .AND.
     &       (PTRIST(STEP(IROOT)).LT.0).AND.
     &        KEEP(60)==0) THEN
          WRITE(*,*) ' Error in SMUMPS_PROCESS_CONTRIB_TYPE3'
          CALL MUMPS_ABORT()
         ENDIF
         CALL SMUMPS_ALLOC_CB(.FALSE.,0_8,.FALSE.,.FALSE.,
     &     MYID,N,KEEP,KEEP8,DKEEP,IW,LIW,A, LA,
     &     LRLU, IPTRLU, IWPOS, IWPOSCB, SLAVEF, PROCNODE_STEPS, DAD,
     &     PTRIST, PTRAST, STEP, PIMASTER, PAMASTER,
     &     LREQI, LREQA, -1234, S_NOTFREE, .FALSE.,
     &     COMP, LRLUS, KEEP8(67), IFLAG, IERROR
     &          )
         IF ( IFLAG .LT. 0 ) RETURN
         CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   IW( IWPOSCB + 1 ), LREQI,
     &                   MPI_INTEGER, COMM, IERR )
         CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   A( IPTRLU + 1_8 ), int(LREQA),
     &                   MPI_REAL, COMM, IERR )
         OPASSW = OPASSW + LREQA 
         CALL SMUMPS_ASS_ROOT( root, KEEP(50), NSUPROW, NSUPCOL,
     &                     IW( IWPOSCB + 1 ), 
     &                     IW( IWPOSCB + NSUPROW + 1 ), NSUPCOL,
     &                     A( IPTRLU + 1_8 ),
     &                     A( 1 ), 
     &                     LOCAL_M, LOCAL_N,
     &                  root%RHS_ROOT(1,1), root%RHS_NLOC,
     &                  1)
         IWPOSCB = IWPOSCB + LREQI
         IPTRLU  = IPTRLU  + LREQA
         LRLU    = LRLU    + LREQA
         LRLUS   = LRLUS   + LREQA
         KEEP8(69) = KEEP8(69) - LREQA
         CALL SMUMPS_LOAD_MEM_UPDATE(.FALSE.,.FALSE.,
     &                    LA-LRLUS,0_8,-LREQA,KEEP,KEEP8,LRLUS)
        ENDIF  
        LREQI = NBROWS_PACKET + NSUBSET_COL_EFF
        LREQA = int(NBROWS_PACKET,8) * int(NSUBSET_COL_EFF,8)
        IF ( (LREQA.NE.0_8) .AND.
     &       (PTRIST(STEP(IROOT)).LT.0).AND.
     &       KEEP(60)==0) THEN
         WRITE(*,*) ' Error in SMUMPS_PROCESS_CONTRIB_TYPE3'
         CALL MUMPS_ABORT()
        ENDIF
        IF (LREQA.NE.0_8) THEN
          CALL SMUMPS_ALLOC_CB(.FALSE.,0_8,.FALSE.,.FALSE.,
     &     MYID,N,KEEP,KEEP8,DKEEP,IW,LIW,A, LA,
     &     LRLU, IPTRLU, IWPOS, IWPOSCB, SLAVEF, PROCNODE_STEPS, DAD,
     &     PTRIST, PTRAST, STEP, PIMASTER, PAMASTER,
     &     LREQI, LREQA, -1234, S_NOTFREE, .FALSE.,
     &     COMP, LRLUS, KEEP8(67), IFLAG, IERROR
     &          )
          IF ( IFLAG .LT. 0 ) RETURN
          CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   IW( IWPOSCB + 1 ), LREQI,
     &                   MPI_INTEGER, COMM, IERR )
          CALL MPI_UNPACK( BUFR, LBUFR_BYTES, POSITION,
     &                   A( IPTRLU + 1_8 ), int(LREQA),
     &                   MPI_REAL, COMM, IERR )
          OPASSW = OPASSW + LREQA 
          IF (KEEP(60).EQ.0) THEN
            CALL SMUMPS_ASS_ROOT( root, KEEP(50),
     &                     NBROWS_PACKET, NSUBSET_COL_EFF,
     &                     IW( IWPOSCB + 1 ),
     &                     IW( IWPOSCB + NBROWS_PACKET + 1 ),
     &                     NSUPCOL_EFF,
     &                     A( IPTRLU + 1_8 ),
     &                     A( POS_ROOT ), LOCAL_M, LOCAL_N,
     &                  root%RHS_ROOT(1,1), root%RHS_NLOC,
     &                  0)   
          ELSE
            CALL SMUMPS_ASS_ROOT( root, KEEP(50),
     &                     NBROWS_PACKET, NSUBSET_COL_EFF,
     &                     IW( IWPOSCB + 1 ),
     &                     IW( IWPOSCB + NBROWS_PACKET + 1 ),
     &                     NSUPCOL_EFF,
     &                     A( IPTRLU + 1_8 ),
     &                     root%SCHUR_POINTER(1),
     &                     root%SCHUR_LLD , root%SCHUR_NLOC,
     &                  root%RHS_ROOT(1,1), root%RHS_NLOC,
     &                  0)  
          ENDIF
          IWPOSCB = IWPOSCB + LREQI
          IPTRLU  = IPTRLU  + LREQA
          LRLU    = LRLU    + LREQA
          LRLUS   = LRLUS   + LREQA
          KEEP8(69) = KEEP8(69) - LREQA
          CALL SMUMPS_LOAD_MEM_UPDATE(.FALSE.,.FALSE.,
     &                    LA-LRLUS,0_8,-LREQA,KEEP,KEEP8,LRLUS)
        ENDIF
      RETURN
      END SUBROUTINE SMUMPS_PROCESS_CONTRIB_TYPE3
