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
      SUBROUTINE ZMUMPS_ASM_SLAVE_MASTER(N, INODE, IW, LIW, A, LA, 
     &    ISON, NBROWS, NBCOLS, ROWLIST,
     &    VALSON, PTLUST_S, PTRAST, STEP, PIMASTER,
     &    OPASSW, IWPOSCB, MYID, KEEP,KEEP8, IS_ofType5or6,
     &    LDA_VALSON )
      USE ZMUMPS_LOAD
      IMPLICIT NONE
      INTEGER KEEP(500)
      INTEGER(8) KEEP8(150)
      INTEGER(8) :: LA
      INTEGER N,LIW,MYID
      INTEGER INODE,ISON, IWPOSCB
      INTEGER NBROWS, NBCOLS, LDA_VALSON
      INTEGER(8) :: PTRAST(KEEP(28))
      INTEGER IW(LIW), STEP(N), PIMASTER(KEEP(28)),
     &        PTLUST_S(KEEP(28)), ROWLIST(NBROWS)
      COMPLEX(kind=8) A(LA), VALSON(LDA_VALSON,NBROWS)
      DOUBLE PRECISION OPASSW
      LOGICAL, INTENT(IN) :: IS_ofType5or6
      INTEGER(8) :: POSELT, POSEL1, APOS, JJ2
      INTEGER HF,HS, NSLAVES, NFRONT, NASS1,
     &        IOLDPS, ISTCHK, LSTK, NSLSON,NELIM,
     &        NPIVS,NCOLS,J1,JJ,JJ1,NROWS,
     &        LDAFS_PERE, IBEG, DIAG
      INCLUDE 'mumps_headers.h'
      LOGICAL SAME_PROC
      INTRINSIC real
      IOLDPS = PTLUST_S(STEP(INODE))
      POSELT = PTRAST(STEP(INODE))
      NFRONT = IW(IOLDPS+KEEP(IXSZ))
      NASS1  = iabs(IW(IOLDPS + 2+KEEP(IXSZ)))
      NSLAVES= IW(IOLDPS+5+KEEP(IXSZ))
      IF (KEEP(50).EQ.0) THEN
        LDAFS_PERE = NFRONT
      ELSE
        IF ( NSLAVES .eq. 0 ) THEN
          LDAFS_PERE = NFRONT
        ELSE
          LDAFS_PERE = NASS1
        ENDIF
      ENDIF
      HF      = 6 + NSLAVES + KEEP(IXSZ)
      POSEL1 = POSELT - int(LDAFS_PERE,8)
      ISTCHK = PIMASTER(STEP(ISON))
      LSTK = IW(ISTCHK+KEEP(IXSZ))
      NSLSON  = IW(ISTCHK + 5+KEEP(IXSZ))
      HS      = 6 + NSLSON + KEEP(IXSZ)
      OPASSW = OPASSW + dble(NBROWS*NBCOLS)
      NELIM = IW(ISTCHK + 1+KEEP(IXSZ))
      NPIVS = IW(ISTCHK + 3+KEEP(IXSZ))
      IF (NPIVS.LT.0) NPIVS = 0
      NCOLS = NPIVS + LSTK
      SAME_PROC = (ISTCHK.LT.IWPOSCB)
      IF (SAME_PROC) THEN
       NROWS = NCOLS
      ELSE
       NROWS = IW(ISTCHK+2+KEEP(IXSZ))
      ENDIF
      J1 = ISTCHK + NROWS + HS + NPIVS
      IF (KEEP(50).EQ.0) THEN
       IF (IS_ofType5or6) THEN
         APOS = POSEL1 + int(ROWLIST(1),8) * int(LDAFS_PERE,8)
         DO JJ = 1, NBROWS
           DO JJ1 = 1, NBCOLS
             JJ2 = APOS + int(JJ1-1,8)
             A(JJ2)=A(JJ2)+VALSON(JJ1,JJ)
           ENDDO
           APOS = APOS + int(LDAFS_PERE,8)
         ENDDO
       ELSE
        DO 170 JJ = 1, NBROWS
         APOS = POSEL1 + int(ROWLIST(JJ),8) * int(LDAFS_PERE,8)
         DO 160 JJ1 = 1, NBCOLS
           JJ2 = APOS + int(IW(J1 + JJ1 - 1) - 1,8)
           A(JJ2) = A(JJ2) + VALSON(JJ1,JJ) 
  160    CONTINUE
  170   CONTINUE
       ENDIF
      ELSE
       IF (IS_ofType5or6) THEN
        APOS = POSEL1 + int(ROWLIST(1),8) * int(LDAFS_PERE,8)
        DIAG = ROWLIST(1)
        DO JJ = 1, NBROWS
          DO JJ1 = 1, DIAG
            JJ2 = APOS+int(JJ1-1,8)
            A(JJ2) = A(JJ2) + VALSON(JJ1,JJ)
          ENDDO
          DIAG = DIAG+1
          APOS = APOS + int(LDAFS_PERE,8)
        ENDDO
       ELSE
        DO JJ = 1, NBROWS
         IF (ROWLIST(JJ).LE.NASS1.and..NOT.IS_ofType5or6) THEN
          APOS = POSEL1 + int(ROWLIST(JJ) - 1,8)
          DO JJ1 = 1, NELIM
           JJ2 = APOS + int(IW(J1+JJ1-1),8)*int(LDAFS_PERE,8)
            A(JJ2) = A(JJ2) + VALSON(JJ1,JJ)
          ENDDO
          IBEG = NELIM+1
         ELSE
          IBEG = 1
         ENDIF
         APOS = POSEL1 + int(ROWLIST(JJ),8) * int(LDAFS_PERE,8)
         DO JJ1 = IBEG, NBCOLS
          IF (ROWLIST(JJ).LT.IW(J1 + JJ1 - 1)) EXIT
          JJ2 = APOS + int(IW(J1 + JJ1 - 1) - 1,8)
          A(JJ2) = A(JJ2) + VALSON(JJ1,JJ)
         ENDDO
        ENDDO
       ENDIF  
      ENDIF   
      RETURN
      END SUBROUTINE ZMUMPS_ASM_SLAVE_MASTER
      SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE_INIT
     &    (N, INODE, IW, LIW, A, LA, 
     &    NBROWS, NBCOLS,
     &    OPASSW, OPELIW, STEP, PTRIST, PTRAST, ITLOC,
     &    RHS_MUMPS, FILS, PTRARW, PTRAIW, INTARR, DBLARR, 
     &    ICNTL, KEEP,KEEP8, MYID, LRGROUPS)
      USE ZMUMPS_DYNAMIC_MEMORY_M, ONLY : ZMUMPS_DM_SET_DYNPTR
      IMPLICIT NONE
      INTEGER N,LIW
      INTEGER(8) :: LA
      INTEGER KEEP(500), ICNTL(60)
      INTEGER(8) KEEP8(150)
      INTEGER INODE, MYID
      INTEGER NBROWS, NBCOLS 
      INTEGER(8) :: PTRAST(KEEP(28))
      INTEGER IW(LIW), ITLOC(N+KEEP(253)), STEP(N),
     &        PTRIST(KEEP(28)), FILS(N)
      INTEGER(8), INTENT(IN) :: PTRARW(N), PTRAIW(N)
      COMPLEX(kind=8) :: RHS_MUMPS(KEEP(255))
      COMPLEX(kind=8) :: A(LA)
      INTEGER :: INTARR(KEEP8(27))
      COMPLEX(kind=8) :: DBLARR(KEEP8(26))
      DOUBLE PRECISION OPASSW, OPELIW
      INTEGER, INTENT(IN)    :: LRGROUPS(N)
      INTEGER(8) :: POSELT
      COMPLEX(kind=8), DIMENSION(:), POINTER :: A_PTR
      INTEGER(8) :: LA_PTR
      INTEGER IOLDPS, NBCOLF, NBROWF, NSLAVES, HF,
     &        K1,K2,K,J,JPOS,NASS
      COMPLEX(kind=8) ZERO
      PARAMETER( ZERO = (0.0D0,0.0D0) )
      INCLUDE 'mumps_headers.h'
      IOLDPS  = PTRIST(STEP(INODE))
      CALL ZMUMPS_DM_SET_DYNPTR( IW(IOLDPS+XXS), A, LA,
     &     PTRAST(STEP(INODE)), IW(IOLDPS+XXD), IW(IOLDPS+XXR),
     &     A_PTR, POSELT, LA_PTR )
      NBCOLF  = IW(IOLDPS+KEEP(IXSZ))
      NBROWF  = IW(IOLDPS+2+KEEP(IXSZ))
      NASS    = IW(IOLDPS+1+KEEP(IXSZ))
      NSLAVES = IW(IOLDPS+5+KEEP(IXSZ))
      HF      = 6 + NSLAVES + KEEP(IXSZ)
      IF (NASS.LT.0) THEN
          NASS         = -NASS
          IW(IOLDPS+1+KEEP(IXSZ)) = NASS
          CALL ZMUMPS_ASM_SLAVE_ARROWHEADS(INODE, N, IW, LIW,
     &           IOLDPS, A_PTR(POSELT), LA_PTR, 1_8, KEEP, KEEP8,
     &           ITLOC, FILS, PTRAIW, PTRARW, INTARR, DBLARR,
     &           KEEP8(27), KEEP8(26),
     &           RHS_MUMPS, LRGROUPS)
      ENDIF
      IF (NBROWS.GT.0) THEN
          K1 = IOLDPS + HF + NBROWF
          K2 = K1 + NBCOLF - 1
          JPOS = 1
          DO K = K1, K2
           J        = IW(K)
           ITLOC(J) = JPOS
           JPOS     = JPOS + 1
          ENDDO
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE_INIT
      SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE_END
     & (N, INODE, IW, LIW, NBROWS, STEP, PTRIST,
     & ITLOC, RHS_MUMPS, KEEP,KEEP8)
      IMPLICIT NONE
      INTEGER N, LIW
      INTEGER KEEP(500)
      INTEGER(8) KEEP8(150)
      INTEGER INODE
      INTEGER NBROWS
      INTEGER IW(LIW), ITLOC(N+KEEP(253)), STEP(N),
     &        PTRIST(KEEP(28))
      COMPLEX(kind=8) :: RHS_MUMPS(KEEP(255))
      INCLUDE 'mumps_headers.h'
      INTEGER IOLDPS, NBCOLF, NBROWF, NSLAVES, HF,
     &        K1,K2,K,J
      IOLDPS  = PTRIST(STEP(INODE))
      NBCOLF  = IW(IOLDPS+KEEP(IXSZ))
      NBROWF  = IW(IOLDPS+2+KEEP(IXSZ))
      NSLAVES = IW(IOLDPS+5+KEEP(IXSZ))
      HF      = 6 + NSLAVES+KEEP(IXSZ)
      IF (NBROWS.GT.0) THEN
          K1 = IOLDPS + HF + NBROWF
          K2 = K1 + NBCOLF - 1
          DO K = K1, K2
           J        = IW(K)
           ITLOC(J) = 0
          ENDDO
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE_END
      SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE(N, INODE, IW, LIW, A, LA, 
     &    NBROWS, NBCOLS, ROWLIST, COLLIST, VALSON, 
     &    OPASSW, OPELIW, STEP, PTRIST, PTRAST, ITLOC,
     &    RHS_MUMPS, FILS,
     &    ICNTL, KEEP,KEEP8, MYID, IS_ofType5or6, LDA_VALSON)
      USE ZMUMPS_DYNAMIC_MEMORY_M, ONLY: ZMUMPS_DM_SET_DYNPTR
      IMPLICIT NONE
      INTEGER N,LIW
      INTEGER(8) :: LA
      INTEGER KEEP(500), ICNTL(60)
      INTEGER(8) KEEP8(150)
      INTEGER INODE, MYID
      LOGICAL, intent(in) :: IS_ofType5or6
      INTEGER NBROWS, NBCOLS, LDA_VALSON
      INTEGER ROWLIST(NBROWS), COLLIST(NBCOLS)
      INTEGER IW(LIW), ITLOC(N+KEEP(253)), STEP(N),
     &        PTRIST(KEEP(28)), FILS(N)
      COMPLEX(kind=8) :: RHS_MUMPS(KEEP(255))
      INTEGER(8) :: PTRAST(KEEP(28))
      COMPLEX(kind=8) A(LA), VALSON(LDA_VALSON,NBROWS)
      DOUBLE PRECISION OPASSW, OPELIW
      INTEGER(8) :: POSEL1, POSELT, APOS, K8
      INTEGER IOLDPS, NBCOLF, NBROWF, NSLAVES, HF,
     &        I,J,NASS,IDIAG
      COMPLEX(kind=8), POINTER, DIMENSION(:) :: A_PTR
      INTEGER(8) :: LA_PTR
      INCLUDE 'mumps_headers.h'
      IOLDPS  = PTRIST(STEP(INODE))
      CALL ZMUMPS_DM_SET_DYNPTR( IW(IOLDPS+XXS), A, LA,
     &     PTRAST(STEP(INODE)), IW(IOLDPS+XXD), IW(IOLDPS+XXR),
     &     A_PTR, POSELT, LA_PTR )
      NBCOLF  = IW(IOLDPS+KEEP(IXSZ))
      NBROWF  = IW(IOLDPS+2+KEEP(IXSZ))
      NASS    = IW(IOLDPS+1+KEEP(IXSZ))
      IF ( NBROWS .GT. NBROWF ) THEN
          WRITE(*,*) ' ERR: ERROR : NBROWS > NBROWF'
          WRITE(*,*) ' ERR: INODE =', INODE
          WRITE(*,*) ' ERR: NBROW=',NBROWS,'NBROWF=',NBROWF
          WRITE(*,*) ' ERR: ROW_LIST=', ROWLIST
          WRITE(*,*) ' ERR: NBCOLF/NASS=', NBCOLF, NASS
          CALL MUMPS_ABORT()
       END IF
      NSLAVES = IW(IOLDPS+5+KEEP(IXSZ))
      HF      = 6 + NSLAVES+KEEP(IXSZ)
      IF (NBROWS.GT.0) THEN
          POSEL1 = POSELT - int(NBCOLF,8)
          IF (KEEP(50).EQ.0) THEN
           IF (IS_ofType5or6) THEN
            APOS = POSEL1 + int(ROWLIST(1),8) * int(NBCOLF,8)
            DO I=1, NBROWS
                DO J = 1, NBCOLS
                  A_PTR(APOS+int(J-1,8)) = A_PTR( APOS+int(J-1,8)) +
     &                                     VALSON(J,I)
                ENDDO
                APOS = APOS + int(NBCOLF,8)
            END DO
           ELSE
            DO I=1,NBROWS
             APOS = POSEL1 + int(ROWLIST(I),8) * int(NBCOLF,8)
             DO J=1,NBCOLS
              K8 = APOS + int(ITLOC(COLLIST(J)),8) - 1_8
              A_PTR(K8) = A_PTR(K8) + VALSON(J,I)
             ENDDO
            ENDDO
           ENDIF
          ELSE
           IF (IS_ofType5or6) THEN
            APOS = POSEL1 + int(ROWLIST(1),8) * int(NBCOLF,8)
     &              + int((NBROWS-1),8)*int(NBCOLF,8)
            IDIAG = 0
            DO I=NBROWS,1,-1
             A_PTR(APOS:APOS+int(NBCOLS-IDIAG-1,8))= 
     &         A_PTR(APOS:APOS+int(NBCOLS-IDIAG-1,8)) +
     &         VALSON(1:NBCOLS-IDIAG,I)
             APOS = APOS - int(NBCOLF,8)
             IDIAG = IDIAG + 1
            ENDDO
           ELSE
            DO I=1,NBROWS
             APOS = POSEL1 + int(ROWLIST(I),8) * int(NBCOLF,8)
             DO J=1,NBCOLS
              IF (ITLOC(COLLIST(J)) .EQ. 0) THEN 
                  EXIT
              ENDIF
              K8 = APOS + int(ITLOC(COLLIST(J)),8) - 1_8
              A_PTR(K8) = A_PTR(K8) + VALSON(J,I)
             ENDDO
            ENDDO
           ENDIF
          ENDIF
          OPASSW = OPASSW + dble(NBROWS*NBCOLS)
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_ASM_SLAVE_TO_SLAVE
      SUBROUTINE ZMUMPS_LDLT_ASM_NIV12_IP( A, LA,
     &             IAFATH, NFRONT, NASS1,
     &             IACB, NCOLS, LCB,
     &             IW, NROWS, NELIM, ETATASS,
     &             CB_IS_COMPRESSED )
      IMPLICIT NONE
      INTEGER NFRONT, NASS1
      INTEGER(8) :: LA
      INTEGER NCOLS, NROWS, NELIM
      INTEGER(8) :: LCB
      COMPLEX(kind=8) A( LA )
      INTEGER(8) :: IAFATH, IACB
      INTEGER IW( NCOLS )
      INTEGER ETATASS
      LOGICAL CB_IS_COMPRESSED
      COMPLEX(kind=8) ZERO
      PARAMETER( ZERO = (0.0D0,0.0D0) )
      INTEGER I, J
      INTEGER(8) :: APOS, POSELT
      INTEGER(8) :: IPOSCB, IBEGCBROW, IENDFRONT
      LOGICAL RESET_TO_ZERO, RISK_OF_SAME_POS,
     &        RISK_OF_SAME_POS_THIS_LINE
      IENDFRONT =  IAFATH+int(NFRONT,8)*int(NFRONT,8)-1_8
      IPOSCB=1_8
      RESET_TO_ZERO    = IACB .LT. IENDFRONT + 1_8
      RISK_OF_SAME_POS = IACB + LCB .EQ. IENDFRONT + 1_8
      RISK_OF_SAME_POS_THIS_LINE = .FALSE.
      DO I=1, NROWS
          POSELT = int(IW(I)-1,8) * int(NFRONT,8)
          IF (.NOT. CB_IS_COMPRESSED ) THEN
            IPOSCB = 1_8 + int(I - 1,8) * int(NCOLS,8)
            IF (IACB+IPOSCB-1_8 .GE. IENDFRONT + 1_8) THEN
              RESET_TO_ZERO = .FALSE.
            ENDIF
          ENDIF
          IF ( RISK_OF_SAME_POS ) THEN
            IF (I.EQ.NROWS .OR. .NOT. CB_IS_COMPRESSED) THEN
              IF ( IAFATH + POSELT + int(IW(I)-1,8) .EQ.
     &             IACB+IPOSCB+int(I-1-1,8)) THEN
                 RISK_OF_SAME_POS_THIS_LINE = .TRUE.
              ENDIF
            ENDIF
          ENDIF
          IF (RESET_TO_ZERO) THEN
            IF ( RISK_OF_SAME_POS_THIS_LINE ) THEN
              DO J=1, I
                APOS = POSELT + int(IW( J ),8)
                IF (IAFATH + APOS - 1_8.NE. IACB+IPOSCB-1_8) THEN
                  A(IAFATH+ APOS -1_8) = A(IACB+IPOSCB-1_8)
                  A(IACB+IPOSCB-1_8) = ZERO
                ENDIF
                IPOSCB = IPOSCB + 1_8
              ENDDO
            ELSE
              DO J=1, I
                APOS = POSELT + int(IW( J ),8)
                A(IAFATH+ APOS -1_8) = A(IACB+IPOSCB-1_8)
                A(IACB+IPOSCB-1_8) = ZERO
                IPOSCB = IPOSCB + 1_8
              ENDDO
            ENDIF
          ELSE
            DO J=1, I
              APOS = POSELT + int(IW( J ),8)
              A(IAFATH+ APOS -1_8) = A(IACB+IPOSCB-1_8)
              IPOSCB = IPOSCB + 1_8
            ENDDO
          ENDIF
          IF (.NOT. CB_IS_COMPRESSED ) THEN
            IBEGCBROW = IACB+IPOSCB-1_8
            IF ( IBEGCBROW .LE. IENDFRONT ) THEN
              A(IBEGCBROW:IBEGCBROW+int(NCOLS-I,8)-1_8)=ZERO
            ENDIF
          ENDIF
          IF (IACB+IPOSCB-1_8 .GE. IENDFRONT + 1_8) THEN
            RESET_TO_ZERO = .FALSE.
          ENDIF
      ENDDO
      RETURN
      END SUBROUTINE ZMUMPS_LDLT_ASM_NIV12_IP
      SUBROUTINE ZMUMPS_LDLT_ASM_NIV12( A, LA, SON_A,
     &             IAFATH, NFRONT, NASS1,
     &             NCOLS, LCB,
     &             IW, NROWS, NELIM, ETATASS,
     &             CB_IS_COMPRESSED
!$   &             , K360
     &             )
      IMPLICIT NONE
      INTEGER NFRONT, NASS1
      INTEGER(8) :: LA
      INTEGER NCOLS, NROWS, NELIM
      INTEGER(8) :: LCB
      COMPLEX(kind=8) A( LA )
      COMPLEX(kind=8) SON_A( LCB )
      INTEGER(8) :: IAFATH
      INTEGER IW( NCOLS )
      INTEGER ETATASS
      LOGICAL CB_IS_COMPRESSED
!$    INTEGER, INTENT(in):: K360
      COMPLEX(kind=8) ZERO
      PARAMETER( ZERO = (0.0D0,0.0D0) )
      INTEGER I, J
      INTEGER(8) :: APOS, POSELT
      INTEGER(8) :: IPOSCB
!$    LOGICAL    :: OMP_FLAG
      IF ((ETATASS.EQ.0) .OR. (ETATASS.EQ.1)) THEN
        IPOSCB = 1_8
        DO I = 1, NELIM
          POSELT = int( IW( I ) - 1, 8 ) * int(NFRONT, 8)
          IF (.NOT. CB_IS_COMPRESSED) THEN
            IPOSCB = 1_8 + int( I - 1, 8 ) * int(NCOLS,8)
          ENDIF
          DO J = 1, I
            APOS = POSELT + int(IW( J ),8)
            A(IAFATH+ APOS -1_8) = A(IAFATH+ APOS -1_8)
     &                           + SON_A(IPOSCB)
            IPOSCB = IPOSCB + 1_8
          END DO
        END DO
      ENDIF
      IF ((ETATASS.EQ.0).OR.(ETATASS.EQ.1)) THEN
!$      OMP_FLAG = (NROWS-NELIM).GE.K360
!$OMP PARALLEL DO PRIVATE(IPOSCB, POSELT, J, APOS) IF (OMP_FLAG)
       DO I = NELIM + 1, NROWS
          IF (CB_IS_COMPRESSED) THEN
            IPOSCB = (int(I,8) * int(I-1,8)) / 2_8 + 1_8
          ELSE
            IPOSCB = int(I-1,8) * int(NCOLS,8) + 1_8
          ENDIF
          POSELT = int(IW( I ),8)
          IF (POSELT.LE. int(NASS1,8)) THEN 
            DO J = 1, NELIM
              APOS = POSELT + int( IW( J ) - 1, 8 ) * int(NFRONT,8)
              A(IAFATH+APOS-1_8) = A(IAFATH+APOS-1_8) +
     &                             SON_A(IPOSCB)
              IPOSCB = IPOSCB + 1_8
            END DO
          ELSE
            POSELT = int( IW( I ) - 1, 8 ) * int(NFRONT, 8)
            DO J = 1, NELIM
             APOS = POSELT + int(IW( J ), 8)
             A(IAFATH+APOS-1_8) = A(IAFATH+APOS-1_8)
     &                          + SON_A(IPOSCB)
             IPOSCB = IPOSCB + 1_8
            END DO
          ENDIF
          IF (ETATASS.EQ.1) THEN
            POSELT = int( IW( I ) - 1, 8 ) * int(NFRONT, 8)
            DO J = NELIM + 1, I
                 IF (IW(J).GT.NASS1) EXIT
                 APOS = POSELT + int(IW( J ), 8)
                 A(IAFATH+APOS-1_8) = A(IAFATH+APOS-1_8)
     &                              + SON_A(IPOSCB)
                 IPOSCB = IPOSCB +1_8
            END DO
          ELSE
            POSELT = int( IW( I ) - 1, 8 ) * int(NFRONT, 8)
            DO J = NELIM + 1, I
             APOS = POSELT + int(IW( J ), 8)
             A(IAFATH+APOS-1_8) = A(IAFATH+APOS-1_8)
     &                          + SON_A(IPOSCB)
             IPOSCB = IPOSCB + 1_8
            END DO
          ENDIF
        END DO
!$OMP END PARALLEL DO
      ELSE  
        DO I= NROWS, NELIM+1, -1
          IF (CB_IS_COMPRESSED) THEN
            IPOSCB = (int(I,8)*int(I+1,8))/2_8 
          ELSE
            IPOSCB = int(I-1,8) * int(NCOLS,8) + int(I,8)
          ENDIF
          POSELT = int(IW( I ),8)
          IF (POSELT.LE.int(NASS1,8)) EXIT
          POSELT = int( IW( I ) - 1, 8 ) * int(NFRONT, 8)
          DO J=I,NELIM+1, -1
            IF (IW(J).LE.NASS1) EXIT
            APOS = POSELT + int(IW( J ), 8)
            A(IAFATH+APOS-1_8) = A(IAFATH+APOS-1_8)
     &                         + SON_A(IPOSCB)
            IPOSCB = IPOSCB - 1_8
          ENDDO
        ENDDO
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_LDLT_ASM_NIV12
      SUBROUTINE ZMUMPS_RESTORE_INDICES(N, ISON, INODE, IWPOSCB,
     &           PIMASTER, PTLUST_S, IW, LIW, STEP, KEEP,KEEP8)
      IMPLICIT NONE
      INTEGER N, ISON, INODE, IWPOSCB
      INTEGER KEEP(500), STEP(N)
      INTEGER(8) KEEP8(150)
      INTEGER PIMASTER(KEEP(28)), PTLUST_S(KEEP(28))
      INTEGER LIW
      INTEGER IW(LIW)
      INTEGER ISTCHK, LSTK, NSLSON, HS, NROWS, NCOLS, NPIVS, NELIM
      INTEGER IOLDPS, NFRONT, NSLAVES, ICT11, HF
      INTEGER J1, J2, J3, JJ, JPOS
      LOGICAL SAME_PROC
      INCLUDE 'mumps_headers.h'
      ISTCHK = PIMASTER(STEP(ISON))
      LSTK   = IW(ISTCHK+KEEP(IXSZ))
      NSLSON = IW(ISTCHK+5+KEEP(IXSZ))
      HS     = 6 + NSLSON + KEEP(IXSZ)
      NELIM  = IW(ISTCHK + 1+KEEP(IXSZ))
      NPIVS  = IW(ISTCHK + 3+KEEP(IXSZ))
      NCOLS  = NPIVS + LSTK
      IF ( NPIVS < 0 ) NPIVS = 0
      SAME_PROC = ISTCHK < IWPOSCB
      IF (SAME_PROC) THEN
       NROWS = NCOLS
      ELSE
       NROWS = IW(ISTCHK+2+KEEP(IXSZ))
      ENDIF
      J1 = ISTCHK + NROWS + HS + NPIVS
      IF (KEEP(50).NE.0) THEN
          J2 = J1 +  LSTK - 1
          DO JJ = J1, J2
            IW(JJ) = IW(JJ - NROWS)
          ENDDO
      ELSE
            J2 = J1 + LSTK - 1
            J3 = J1 + NELIM
            DO JJ = J3, J2
             IW(JJ) = IW(JJ - NROWS)
            ENDDO
            IF (NELIM .NE. 0) THEN
              IOLDPS = PTLUST_S(STEP(INODE))
              NFRONT = IW(IOLDPS+KEEP(IXSZ))
              NSLAVES= IW(IOLDPS+5+KEEP(IXSZ))
              HF     = 6 + NSLAVES+KEEP(IXSZ)
              ICT11 = IOLDPS + HF - 1 + NFRONT
              J3 = J3 - 1
              DO 190 JJ = J1, J3
               JPOS = IW(JJ) + ICT11
               IW(JJ) = IW(JPOS)
  190         CONTINUE
            ENDIF
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_RESTORE_INDICES
      SUBROUTINE ZMUMPS_ASM_MAX(
     &     N, INODE, IW, LIW, A, LA, 
     &     ISON, NBCOLS,
     &     VALSON, PTLUST_S, PTRAST, STEP, PIMASTER,
     &     OPASSW, IWPOSCB,MYID, KEEP,KEEP8 )
      USE ZMUMPS_LOAD
      IMPLICIT NONE
      INTEGER KEEP(500)
      INTEGER(8) KEEP8(150)
      INTEGER(8) :: LA
      INTEGER N,LIW,MYID
      INTEGER INODE,ISON,IWPOSCB
      INTEGER NBCOLS
      INTEGER IW(LIW), STEP(N), 
     &     PIMASTER(KEEP(28)),
     &     PTLUST_S(KEEP(28))
      INTEGER(8) PTRAST(KEEP(28))
      COMPLEX(kind=8) A(LA)
      DOUBLE PRECISION VALSON(NBCOLS)
      DOUBLE PRECISION OPASSW
      INTEGER HF,HS, NSLAVES, NASS1,
     &     IOLDPS, ISTCHK,
     &     LSTK, NSLSON,NELIM,NPIVS,NCOLS, J1,
     &     JJ1,NROWS
      INTEGER(8) POSELT, APOS, JJ2
      INCLUDE 'mumps_headers.h'
      LOGICAL SAME_PROC
      INTRINSIC real
      IOLDPS = PTLUST_S(STEP(INODE))
      POSELT = PTRAST(STEP(INODE))
      NASS1  = iabs(IW(IOLDPS + 2 + KEEP(IXSZ)))
      NSLAVES= IW(IOLDPS+5 + KEEP(IXSZ))
      HF      = 6 + NSLAVES + KEEP(IXSZ)
      ISTCHK = PIMASTER(STEP(ISON))
      LSTK = IW(ISTCHK + KEEP(IXSZ))
      NSLSON  = IW(ISTCHK + 5 + KEEP(IXSZ))
      HS      = 6 + NSLSON + KEEP(IXSZ)
      NELIM = IW(ISTCHK + 1 + KEEP(IXSZ))
      NPIVS = IW(ISTCHK + 3 + KEEP(IXSZ))
      IF (NPIVS.LT.0) NPIVS = 0
      NCOLS = NPIVS + LSTK
      SAME_PROC = (ISTCHK.LT.IWPOSCB)
      IF (SAME_PROC) THEN
       NROWS = NCOLS
      ELSE
       NROWS = IW(ISTCHK+2 + KEEP(IXSZ))
      ENDIF
      J1 = ISTCHK + NROWS + HS + NPIVS
      APOS = POSELT + int(NASS1,8)*int(NASS1,8) - 1_8
      DO JJ1 = 1, NBCOLS
         JJ2 = APOS+int(IW(J1 + JJ1 - 1),8)
         IF(dble(A(JJ2)) .LT. VALSON(JJ1)) THEN
              A(JJ2) = cmplx(VALSON(JJ1),kind=kind(A))
         ENDIF
      ENDDO
      RETURN
      END SUBROUTINE ZMUMPS_ASM_MAX
      SUBROUTINE ZMUMPS_ASM_SLAVE_ARROWHEADS(INODE, N, IW, LIW, IOLDPS,
     &           A, LA, POSELT, KEEP, KEEP8,
     &           ITLOC, FILS, PTRAIW, PTRARW, INTARR, DBLARR,
     &           LINTARR, LDBLARR, RHS_MUMPS, LRGROUPS)
!$    USE OMP_LIB
      USE ZMUMPS_ANA_LR,    ONLY : GET_CUT
      USE ZMUMPS_LR_CORE,   ONLY : MAX_CLUSTER
      USE MUMPS_LR_COMMON,  ONLY : COMPUTE_BLR_VCS
      IMPLICIT NONE
      INTEGER, intent(in)    :: N, LIW, IOLDPS, INODE
      INTEGER(8), intent(in) :: LA, POSELT
      INTEGER(8), intent(in) :: LINTARR, LDBLARR
      INTEGER, intent(in)    :: IW(LIW)
      INTEGER, intent(in)    :: KEEP(500)
      INTEGER(8), intent(in) :: KEEP8(150)
      INTEGER, intent(inout) :: ITLOC(N+KEEP(253))
      COMPLEX(kind=8), intent(inout) :: A(LA)
      COMPLEX(kind=8), intent(in)    :: RHS_MUMPS(KEEP(255))
      COMPLEX(kind=8), intent(in)    :: DBLARR(LDBLARR)
      INTEGER, intent(in)    :: INTARR(LINTARR)
      INTEGER, intent(in)    :: FILS(N)
      INTEGER(8), intent(in) :: PTRAIW(N), PTRARW(N)
      INTEGER, INTENT(IN)    :: LRGROUPS(N)
!$    INTEGER :: NOMP
!$    INTEGER(8) :: CHUNK8  
!$    INTEGER    :: CHUNK
      INCLUDE 'mumps_headers.h'
      INTEGER    :: HF, NBROWF, NBCOLF, NASS, NSLAVES
      INTEGER    :: ILOC, J, K, K1, K2, JPOS, IJROW
      INTEGER    :: IN
      INTEGER(8) :: J18, J28, JJ8, JK8
      INTEGER(8) :: APOS, ICT12
      INTEGER(8) :: AINPUT8
      INTEGER, POINTER, DIMENSION(:) :: BEGS_BLR_LS
      INTEGER :: NB_BLR_LS, NPARTSCB, NPARTSASS, MAXI_CLUSTER, 
     &           IBCKSZ2, MINSIZE, TOPDIAG
      INTEGER(8) :: JJ3
      INTEGER    :: K1RHS, K2RHS, JFirstRHS
      COMPLEX(kind=8) ZERO
      PARAMETER( ZERO = (0.0D0,0.0D0) )
      NBCOLF  = IW(IOLDPS+KEEP(IXSZ))
      NBROWF  = IW(IOLDPS+2+KEEP(IXSZ))
      NASS    = IW(IOLDPS+1+KEEP(IXSZ))
      NSLAVES= IW(IOLDPS+5 + KEEP(IXSZ))
      HF      = 6 + NSLAVES + KEEP(IXSZ)
!$    NOMP = OMP_GET_MAX_THREADS()
      IF (KEEP(50) .EQ. 0 .OR. NBROWF .LT. KEEP(63)) THEN
!$      CHUNK8 = int(KEEP(361),8)
!$OMP   PARALLEL DO PRIVATE(JJ8) SCHEDULE(STATIC, CHUNK8)
!$OMP&  IF (int(NBROWF,8)*int(NBCOLF,8) > int(KEEP(361),8)
!$OMP&    .AND. NOMP .GT. 1)
        DO JJ8=POSELT, POSELT+int(NBROWF,8)*int(NBCOLF,8)-1_8
          A(JJ8) = ZERO
        ENDDO
!$OMP   END PARALLEL DO
      ELSE
        TOPDIAG = 0
        IF (IW(IOLDPS+XXLR).GE.1) THEN
          CALL GET_CUT(IW(IOLDPS+HF:IOLDPS+HF+NBROWF-1), 0,
     &                    NBROWF, LRGROUPS, NPARTSCB, 
     &                    NPARTSASS, BEGS_BLR_LS)
          NB_BLR_LS = NPARTSCB
          call MAX_CLUSTER(BEGS_BLR_LS,NB_BLR_LS+1,MAXI_CLUSTER)
          DEALLOCATE(BEGS_BLR_LS)
          CALL COMPUTE_BLR_VCS(KEEP(472), IBCKSZ2, KEEP(488), NASS)
          MINSIZE = int(IBCKSZ2 / 2)
          TOPDIAG = max(2*MINSIZE + MAXI_CLUSTER-1, TOPDIAG)
        ENDIF
!$      CHUNK = max( KEEP(360)/2,
!$   &               ((NBROWF+NOMP-1)/NOMP +2) / 3 )
!$OMP   PARALLEL DO PRIVATE(APOS,JJ3,JJ8) SCHEDULE(STATIC,CHUNK)
!$OMP&  IF (NBROWF .GT. KEEP(360) .AND. NOMP .GT. 1)
        DO JJ8 = 0_8, int(NBROWF-1,8)
          APOS = POSELT+ JJ8*int(NBCOLF,8)
          JJ3  = min( int(NBCOLF,8)  - 1_8, 
     &           JJ8 + int(NBCOLF-NBROWF,8) + TOPDIAG )
          A(APOS: APOS+JJ3) = ZERO
        ENDDO
!$OMP   END PARALLEL DO
      ENDIF
      K1 = IOLDPS + HF + NBROWF
      K2 = K1 + NASS - 1
      JPOS = 1
      DO K = K1, K2
         J        = IW(K)
         ITLOC(J) = -JPOS
         JPOS     = JPOS + 1
      ENDDO
      K1 = IOLDPS + HF 
      K2 = K1 + NBROWF - 1
      JPOS = 1
      IF ((KEEP(253).GT.0).AND.(KEEP(50).NE.0)) THEN
           K1RHS = 0
           K2RHS = -1
           DO K = K1, K2
            J        = IW(K)
            ITLOC(J) = JPOS
            IF ((K1RHS.EQ.0).AND.(J.GT.N)) THEN
             K1RHS = K
             JFirstRHS=J-N 
            ENDIF
            JPOS     = JPOS + 1
           ENDDO
           IF (K1RHS.GT.0) K2RHS=K2
           IF ( K2RHS.GE.K1RHS ) THEN
             IN = INODE
             DO WHILE (IN.GT.0) 
               IJROW = -ITLOC(IN)  
               DO K = K1RHS, K2RHS
                J    = IW(K)       
                ILOC = ITLOC(J)    
                APOS = POSELT+int(ILOC-1,8)*int(NBCOLF,8) + 
     &                 int(IJROW-1,8) 
                A(APOS) = A(APOS) + RHS_MUMPS(
     &                    (JFirstRHS+(K-K1RHS)-1)*KEEP(254)+IN)
              ENDDO
              IN = FILS(IN)
             ENDDO
            ENDIF
          ELSE  
           DO K = K1, K2
            J        = IW(K)
            ITLOC(J) = JPOS
            JPOS     = JPOS + 1
           ENDDO
      ENDIF
      IN = INODE
      DO WHILE (IN.GT.0) 
           AINPUT8 = PTRARW(IN)
           JK8     = PTRAIW(IN)
           JJ8     = JK8 + 1_8
           J18     = JJ8 + 1_8
           J28 = J18 + INTARR(JK8)
           IJROW = -ITLOC(INTARR(J18))
           ICT12 = POSELT +int(- NBCOLF + IJROW - 1,8)
           DO JJ8= J18,J28
            ILOC = ITLOC(INTARR(JJ8))
            IF (ILOC.GT.0) THEN
              APOS = ICT12 + int(ILOC,8)*int(NBCOLF,8)
              A(APOS) = A(APOS) + DBLARR(AINPUT8)
            ENDIF
            AINPUT8 = AINPUT8 + 1_8
           ENDDO
           IN = FILS(IN)
      ENDDO
      K1 = IOLDPS + HF
      K2 = K1 + NBROWF + NASS - 1
      DO K = K1, K2
           J = IW(K)
           ITLOC(J) = 0
      ENDDO
      RETURN
      END SUBROUTINE ZMUMPS_ASM_SLAVE_ARROWHEADS
