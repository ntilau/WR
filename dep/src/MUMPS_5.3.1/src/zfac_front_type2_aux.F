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
      MODULE ZMUMPS_FAC_FRONT_TYPE2_AUX_M
      CONTAINS
      SUBROUTINE ZMUMPS_FAC_I_LDLT_NIV2( 
     &      DIAG_ORIG, SIZEDIAG_ORIG, GW_FACTCUMUL,
     &      NFRONT, NASS, IBEG_BLOCK_TO_SEND, IBEG_BLOCK, IEND_BLOCK,
     &      NASS2, TIPIV,
     &      N, INODE, IW, LIW, A, LA, NNEGW, NB22T2W, NBTINYW,
     &      DET_EXPW, DET_MANTW, DET_SIGNW,
     &                   INOPV, IFLAG,
     &                   IOLDPS, POSELT, UU, 
     &                   SEUIL,KEEP,KEEP8,PIVSIZ,
     &                   DKEEP,PIVNUL_LIST,LPN_LIST,
     &                   PP_FIRST2SWAP_L, PP_LastPanelonDisk,
     &                   PP_LastPIVRPTRIndexFilled, 
     &                   PIVOT_OPTION,   
     &                   Inextpiv, IEND_BLR, 
     &                   OOCWRITE_COMPATIBLE_WITH_BLR)
      USE MUMPS_OOC_COMMON, ONLY : TYPEF_L   
      USE ZMUMPS_FAC_FRONT_AUX_M
      IMPLICIT NONE
      INTEGER SIZEDIAG_ORIG
      DOUBLE PRECISION    DIAG_ORIG(SIZEDIAG_ORIG)
      DOUBLE PRECISION    GW_FACTCUMUL
      INTEGER NFRONT,NASS,N,LIW,INODE,IFLAG,INOPV
      INTEGER NASS2, IBEG_BLOCK_TO_SEND, IBEG_BLOCK, IEND_BLOCK
      INTEGER, intent(inout) :: NNEGW, NB22T2W, NBTINYW
      INTEGER, intent(inout) :: DET_EXPW, DET_SIGNW
      COMPLEX(kind=8), intent(inout) :: DET_MANTW
      INTEGER TIPIV( NASS2 )
      INTEGER PIVSIZ,LPIV
      INTEGER, intent(in)    :: PIVOT_OPTION, IEND_BLR
      INTEGER, intent(inout) :: Inextpiv
      LOGICAL, intent(in)    :: OOCWRITE_COMPATIBLE_WITH_BLR
      INTEGER(8) :: LA
      COMPLEX(kind=8) A(LA) 
      DOUBLE PRECISION UU, UULOC, SEUIL
      COMPLEX(kind=8) CSEUIL
      INTEGER IW(LIW) 
      INTEGER   IOLDPS
      INTEGER(8) :: POSELT
      INTEGER KEEP(500)
      INTEGER(8) KEEP8(150)
      INTEGER LPN_LIST
      INTEGER PIVNUL_LIST(LPN_LIST)
      DOUBLE PRECISION DKEEP(230)
      INTEGER PP_FIRST2SWAP_L, PP_LastPanelonDisk
      INTEGER PP_LastPIVRPTRIndexFilled
      include 'mpif.h'
      INTEGER(8) :: POSPV1,POSPV2,OFFDAG,APOSJ
      INTEGER JMAX
      INTEGER :: IPIVNUL, HF 
      DOUBLE PRECISION RMAX,AMAX,TMAX,RMAX_NORELAX
      DOUBLE PRECISION MAXPIV, ABS_PIVOT
      DOUBLE PRECISION RMAX_NOSLAVE, TMAX_NOSLAVE
      COMPLEX(kind=8) PIVOT,DETPIV
      DOUBLE PRECISION    ABSDETPIV
      INCLUDE 'mumps_headers.h'
      INTEGER(8) :: APOSMAX
      INTEGER(8) :: APOS
      INTEGER(8) :: J1, J2, JJ, KK
      DOUBLE PRECISION       :: GROWTH, RSWOP
      DOUBLE PRECISION       :: UULOCM1
      INTEGER    :: LDAFS
      INTEGER(8) :: LDAFS8
      DOUBLE PRECISION, PARAMETER :: RZERO = 0.0D0
      DOUBLE PRECISION, PARAMETER :: RONE  = 1.0D0
      COMPLEX(kind=8) ZERO, ONE
      PARAMETER( ZERO = (0.0D0,0.0D0) )
      PARAMETER( ONE = (1.0D0,0.0D0) )
      DOUBLE PRECISION PIVNUL, VALTMP
      COMPLEX(kind=8) FIXA
      INTEGER NPIV,IPIV,K219
      INTEGER NPIVP1,ILOC,K,J
      INTEGER ISHIFT, K206, IPIV_END, IPIV_SHIFT
      INTRINSIC max
      INTEGER I_PIVRPTR, I_PIVR, NBPANELS_L
      DOUBLE PRECISION GW_FACT 
      GW_FACT = RONE
      AMAX = RZERO
      RMAX = RZERO
      TMAX = RZERO
      RMAX_NOSLAVE = RZERO
      PIVOT = ONE
      HF = 6 + IW(IOLDPS+5+KEEP(IXSZ)) + KEEP(IXSZ)
      K206  = KEEP(206)
      PIVNUL = DKEEP(1)
      FIXA   = cmplx(DKEEP(2),kind=kind(FIXA))
      CSEUIL = cmplx(SEUIL,kind=kind(CSEUIL))
      LDAFS  = NASS
      LDAFS8 = int(LDAFS,8)
      IF (KEEP(201).EQ.1 .AND. KEEP(50).NE.1 .AND.
     &    OOCWRITE_COMPATIBLE_WITH_BLR) THEN
             CALL ZMUMPS_GET_OOC_PERM_PTR(TYPEF_L, NBPANELS_L, 
     &       I_PIVRPTR, I_PIVR, 
     &       IOLDPS+2*NFRONT+6+IW(IOLDPS+5+KEEP(IXSZ))
     &              +KEEP(IXSZ),
     &       IW, LIW)
      ENDIF
        UULOC = UU
        K219   = KEEP(219)  
        IF (UULOC.GT.RZERO) THEN 
          UULOCM1 = RONE/UULOC
        ELSE
          K219=0
          UULOCM1 = RONE
        ENDIF
        IF (K219.LT.2) GW_FACTCUMUL = RONE
        PIVSIZ = 1
        NPIV    = IW(IOLDPS+1+KEEP(IXSZ))
        NPIVP1  = NPIV + 1
        ILOC = NPIVP1 - IBEG_BLOCK_TO_SEND + 1
        TIPIV( ILOC ) = ILOC
        APOSMAX = POSELT+LDAFS8*LDAFS8-1_8
        IF(INOPV .EQ. -1) THEN
           APOS = POSELT + LDAFS8*int(NPIVP1-1,8) + int(NPIV,8)
           POSPV1 = APOS
           CALL ZMUMPS_UPDATE_MINMAX_PIVOT 
     &           ( abs(A(APOS)),
     &             DKEEP, KEEP, .TRUE.)
           IF(abs(A(APOS)).LT.SEUIL) THEN
              IF(dble(A(APOS)) .GE. RZERO) THEN
                 A(APOS) = CSEUIL
              ELSE
                 A(APOS) = -CSEUIL
              ENDIF
              NBTINYW = NBTINYW + 1
           ELSE IF (KEEP(258) .NE.0 ) THEN
             CALL ZMUMPS_UPDATEDETER( A(APOS), DET_MANTW, DET_EXPW )
           ENDIF
           IF (KEEP(201).EQ.1.AND.KEEP(50).NE.1.AND.
     &         OOCWRITE_COMPATIBLE_WITH_BLR) THEN
             CALL ZMUMPS_STORE_PERMINFO( IW(I_PIVRPTR), NBPANELS_L,
     &               IW(I_PIVR), NASS, NPIVP1, NPIVP1, 
     &               PP_LastPanelonDisk,
     &               PP_LastPIVRPTRIndexFilled)
           ENDIF
           GO TO 420
        ENDIF
        INOPV   = 0
      IF ((K219.GE.2).AND.(NPIVP1.EQ.1)) THEN 
        GW_FACTCUMUL = RONE
        IF (K219.EQ.3) THEN
         DO IPIV=1,NASS
            DIAG_ORIG (IPIV)  = abs(A(POSELT +
     &                              (LDAFS8+1_8)*int(IPIV-1,8)))
         ENDDO
        ELSE IF (K219.GE.4) THEN
         DIAG_ORIG  = RZERO
         DO IPIV=1,NASS
          APOS = POSELT + LDAFS8*int(IPIV-1,8) 
          POSPV1 = APOS + int(IPIV - 1,8)
          DIAG_ORIG(IPIV) = max( abs(A(POSPV1)), DIAG_ORIG(IPIV) ) 
          DO J=IPIV+1,NASS
           DIAG_ORIG(IPIV) = max( abs(A(POSPV1)), DIAG_ORIG(IPIV) ) 
           DIAG_ORIG(IPIV+J-IPIV) = max( abs(A(POSPV1)), 
     &                                   DIAG_ORIG(IPIV+J-IPIV) ) 
           POSPV1 = POSPV1 + LDAFS8
          ENDDO
         ENDDO
        ENDIF
      ENDIF
      ISHIFT = 0              
      IPIV_END = IEND_BLOCK   
      IF (K206.GE.1) THEN
        IF (Inextpiv.GT.NPIVP1.AND.Inextpiv.LE.IEND_BLOCK) THEN
          ISHIFT = Inextpiv - NPIVP1
        ENDIF
        IF ( K206.EQ.1
     &      .OR.  (K206 .GT.1 .AND. IEND_BLOCK.EQ.IEND_BLR) ) THEN
          IPIV_END = IEND_BLOCK + ISHIFT
        ENDIF
      ENDIF  
       DO 460 IPIV_SHIFT = NPIVP1+ISHIFT, IPIV_END
            IF (IPIV_SHIFT .LE. IEND_BLOCK) THEN
              IPIV=IPIV_SHIFT
            ELSE
              IPIV = IPIV_SHIFT-IEND_BLOCK-1+NPIVP1
              IF (IBEG_BLOCK.EQ.NPIVP1) THEN
                EXIT
              ENDIF
            ENDIF
            APOS = POSELT + LDAFS8*int(IPIV-1,8) + int(NPIV,8)
            POSPV1 = APOS + int(IPIV - NPIVP1,8)
            PIVOT     = A(POSPV1)
            ABS_PIVOT = abs(PIVOT)
            IF (UULOC.EQ.RZERO.OR.PIVOT_OPTION.EQ.0) THEN 
              IF (ABS_PIVOT.EQ.RZERO) GO TO 630 
              CALL ZMUMPS_UPDATE_MINMAX_PIVOT 
     &           ( abs(A(APOS)), DKEEP, KEEP, .FALSE.)
              IF (KEEP(258) .NE. 0) THEN
                CALL ZMUMPS_UPDATEDETER(A(APOS), DET_MANTW, DET_EXPW )
              ENDIF
              GO TO 420
            ENDIF
            AMAX = -RONE
            JMAX = 0
            J1 = APOS
            J2 = POSPV1 - 1_8
            DO JJ=J1,J2
               IF(abs(A(JJ)) .GT. AMAX) THEN
                  AMAX = abs(A(JJ))
                  JMAX = IPIV - int(POSPV1-JJ)
               ENDIF
            ENDDO
            J1 = POSPV1 + LDAFS8
            DO J=1, IEND_BLOCK - IPIV
               IF(abs(A(J1)) .GT. AMAX) THEN
                  AMAX = abs(A(J1))
                  JMAX = IPIV + J
               ENDIF
               J1 = J1 + LDAFS8
            ENDDO
            RMAX_NOSLAVE = RZERO
            IF (PIVOT_OPTION.EQ.2) THEN
              DO J=1,NASS - IEND_BLOCK
                RMAX_NOSLAVE = max(abs(A(J1+LDAFS8*int(J-1,8))),
     &                             RMAX_NOSLAVE)
              ENDDO
            ENDIF
            IF (K219.NE.0) THEN
             RMAX_NORELAX = dble(A(APOSMAX+int(IPIV,8)))
             RMAX         = RMAX_NORELAX
             IF (K219.GE.2) THEN
              IF (ABS_PIVOT.NE.RZERO.AND.
     &            ABS_PIVOT.GE.UULOC*max(RMAX,RMAX_NOSLAVE,AMAX)) 
     &            THEN
               GROWTH = RONE
               IF (K219.EQ.3) THEN
                IF (DIAG_ORIG(IPIV).EQ.RZERO) THEN
                 DIAG_ORIG(IPIV) = ABS_PIVOT
                ELSE
                 GROWTH =  ABS_PIVOT / DIAG_ORIG(IPIV)
                ENDIF
               ELSE IF (K219.GE.4) THEN
                IF (DIAG_ORIG(IPIV).EQ.RZERO) THEN
                 DIAG_ORIG(IPIV) = max(AMAX,RMAX_NOSLAVE)
                ELSE
                 GROWTH = max(ABS_PIVOT,AMAX,RMAX_NOSLAVE)/
     &                         DIAG_ORIG(IPIV)
                ENDIF
               ENDIF
               RMAX = RMAX*max(GROWTH,GW_FACTCUMUL)
              ENDIF
             ENDIF   
            ELSE     
             RMAX         = RZERO
             RMAX_NORELAX = RZERO
            ENDIF
            RMAX_NOSLAVE = max(RMAX_NORELAX,RMAX_NOSLAVE)
            RMAX         = max(RMAX,RMAX_NOSLAVE)
            IF (max(AMAX,RMAX,ABS_PIVOT).LE.PIVNUL) THEN
               CALL ZMUMPS_UPDATE_MINMAX_PIVOT 
     &           ( abs(A(POSPV1)), DKEEP, KEEP, .TRUE.)
               KEEP(109) = KEEP(109) + 1
               IPIVNUL = KEEP(109)
               PIVNUL_LIST(IPIVNUL) = IW( IOLDPS+HF+NPIV+IPIV-NPIVP1 )
               IF (dble(FIXA).GT.RZERO) THEN
                 IF(dble(PIVOT) .GE. RZERO) THEN
                   A(POSPV1) = FIXA
                 ELSE
                   A(POSPV1) = -FIXA
                 ENDIF
               ELSE
                 J1 = APOS
                 J2 = POSPV1 - 1_8
                 DO JJ=J1,J2
                    A(JJ) = ZERO
                 ENDDO
                 DO J=1, NASS-IPIV
                   A(POSPV1+int(J,8)*LDAFS8) = ZERO
                 ENDDO
                 VALTMP = max(1.0D10*RMAX, sqrt(huge(RMAX))/1.0D8)
                 A(POSPV1) = cmplx(VALTMP,kind=kind(A))
               ENDIF
               PIVOT = A(POSPV1)
               ABS_PIVOT = abs(PIVOT)
               GO TO 415
         ENDIF
        IF (ABS_PIVOT.GE.UULOC*max(RMAX,AMAX)
     &      .AND. ABS_PIVOT .GT. max(SEUIL, tiny(RMAX))) THEN
          CALL ZMUMPS_UPDATE_MINMAX_PIVOT 
     &       ( ABS_PIVOT, DKEEP, KEEP, .FALSE.)
          IF (KEEP(258) .NE.0 ) THEN
            CALL ZMUMPS_UPDATEDETER(PIVOT, DET_MANTW, DET_EXPW )
          ENDIF
          GO TO 415
        END IF
         IF (NPIVP1.EQ.IEND_BLOCK) THEN
           GOTO 460
         ELSE IF (JMAX .EQ.0) THEN
           GOTO 460
         ENDIF
         IF (max(abs(PIVOT),RMAX,AMAX).LE.tiny(RMAX)) THEN
           GOTO 460
         ENDIF
            IF (RMAX_NOSLAVE.LT.AMAX) THEN
               J1 = APOS
               J2 = POSPV1 - 1_8
               DO JJ=J1,J2
                  IF(int(POSPV1-JJ) .NE. IPIV-JMAX) THEN
                     RMAX_NOSLAVE = max(RMAX_NOSLAVE,abs(A(JJ)))
                  ENDIF
               ENDDO
               DO J=1,NASS-IPIV
                  IF(IPIV+J .NE. JMAX) THEN
                     RMAX_NOSLAVE = max(abs(A(POSPV1+LDAFS8*int(J,8))),
     &                                  RMAX_NOSLAVE)
                  ENDIF
               ENDDO
               RMAX = max(RMAX, RMAX_NOSLAVE)
            ENDIF            
            APOSJ = POSELT + int(JMAX-1,8)*LDAFS8 + int(NPIV,8)
            POSPV2 = APOSJ + int(JMAX - NPIVP1,8)
            IF (IPIV.LT.JMAX) THEN
               OFFDAG = APOSJ + int(IPIV - NPIVP1,8)
            ELSE
               OFFDAG = APOS + int(JMAX - NPIVP1,8)
            END IF
            TMAX_NOSLAVE = RZERO
            IF(JMAX .LT. IPIV) THEN
               JJ = POSPV2
               DO K = 1, NASS-JMAX
                  JJ = JJ+LDAFS8    
                  IF (JMAX+K.NE.IPIV) THEN
                     TMAX_NOSLAVE=max(TMAX_NOSLAVE,abs(A(JJ)))
                  ENDIF
               ENDDO
               DO KK =  APOSJ, POSPV2-1_8
                  TMAX_NOSLAVE = max(TMAX_NOSLAVE,abs(A(KK)))
               ENDDO
              ELSE
               JJ = POSPV2
               DO K = 1, NASS-JMAX
                  JJ = JJ+LDAFS8 
                  TMAX_NOSLAVE=max(TMAX_NOSLAVE,abs(A(JJ)))
               ENDDO
               DO KK =  APOSJ, POSPV2 - 1_8
                  IF (KK.NE.OFFDAG) THEN
                     TMAX_NOSLAVE = max(TMAX_NOSLAVE,abs(A(KK)))
                  ENDIF
               ENDDO
            ENDIF 
            IF (K219.NE.0) THEN
             TMAX = max(SEUIL*UULOCM1,dble(A(APOSMAX+int(JMAX,8))))
            ELSE    
             TMAX = SEUIL*UULOCM1
            ENDIF
            IF (K219.GE.2) THEN
             GROWTH = RONE  
             IF (K219.EQ.3) THEN
              IF (DIAG_ORIG(JMAX).EQ.RZERO) THEN
                 DIAG_ORIG(JMAX) = abs(A(POSPV2))
              ELSE
                GROWTH = abs(A(POSPV2))/DIAG_ORIG(JMAX)
              ENDIF
             ELSE IF (K219.EQ.4) THEN
              IF (DIAG_ORIG(JMAX).EQ.RZERO) THEN
               DIAG_ORIG(JMAX)=max(abs(A(POSPV2)),AMAX,TMAX_NOSLAVE)
              ELSE
               GROWTH = max(abs(A(POSPV2)),AMAX,TMAX_NOSLAVE) 
     &                  / DIAG_ORIG(JMAX)
              ENDIF
             ENDIF
             TMAX = TMAX*max(GROWTH,GW_FACTCUMUL)
            ENDIF  
            TMAX = max (TMAX,TMAX_NOSLAVE)
            DETPIV = A(POSPV1)*A(POSPV2) - A(OFFDAG)*A(OFFDAG)
            ABSDETPIV = abs(DETPIV)
            IF (SEUIL.GT.RZERO) THEN
               IF (sqrt(ABSDETPIV) .LE. SEUIL ) THEN
                 GOTO 460
               ENDIF
            ENDIF
            MAXPIV = max(abs(A(POSPV1)),abs(A(POSPV2)))
            IF (MAXPIV.EQ.RZERO) MAXPIV = RONE
            IF ((abs(A(POSPV2))*RMAX+AMAX*TMAX)*UULOC.GT.
     &            ABSDETPIV .OR. ABSDETPIV .EQ. RZERO) THEN
              GO TO 460
            ENDIF
            IF ((abs(A(POSPV1))*TMAX+AMAX*RMAX)*UULOC.GT.
     &           ABSDETPIV .OR. ABSDETPIV .EQ. RZERO) THEN
              GO TO 460
            ENDIF
           CALL ZMUMPS_UPDATE_MINMAX_PIVOT 
     &           ( sqrt(abs(DETPIV)),
     &             DKEEP, KEEP, .FALSE.)
           IF (KEEP(258).NE.0) THEN
             CALL ZMUMPS_UPDATEDETER(DETPIV, DET_MANTW, DET_EXPW )
           ENDIF
           PIVSIZ = 2
           NB22T2W = NB22T2W+1
 415       CONTINUE
           IF (K206.GE.1) THEN
             Inextpiv = max(NPIVP1+PIVSIZ, IPIV+1)
           ENDIF
           DO K=1,PIVSIZ
              IF (PIVSIZ .EQ. 2 ) THEN
                IF (K==1) THEN
                  LPIV = min(IPIV, JMAX)
                  TIPIV(ILOC) = -(LPIV - IBEG_BLOCK_TO_SEND + 1)
                ELSE
                  LPIV = max(IPIV, JMAX)
                  TIPIV(ILOC+1) = -(LPIV - IBEG_BLOCK_TO_SEND + 1)
                ENDIF
              ELSE
                LPIV = IPIV
                TIPIV(ILOC) = IPIV - IBEG_BLOCK_TO_SEND + 1
              ENDIF
              IF (LPIV.EQ.NPIVP1) THEN
                 GOTO 416
              ENDIF
              KEEP8(80) = KEEP8(80)+1
              CALL ZMUMPS_SWAP_LDLT( A, LA, IW, LIW,
     &             IOLDPS, NPIVP1, LPIV, POSELT, NASS,
     &             LDAFS, NFRONT, 2, K219, KEEP(50),
     &             KEEP(IXSZ), IBEG_BLOCK_TO_SEND )
              IF (K219.GE.3) THEN
               RSWOP = DIAG_ORIG(LPIV)   
               DIAG_ORIG(LPIV) = DIAG_ORIG(NPIVP1) 
               DIAG_ORIG(NPIVP1) = RSWOP 
              ENDIF
 416          CONTINUE
              IF (KEEP(201).EQ.1.AND.KEEP(50).NE.1.AND.
     &            OOCWRITE_COMPATIBLE_WITH_BLR) THEN
                CALL ZMUMPS_STORE_PERMINFO( IW(I_PIVRPTR), NBPANELS_L,
     &               IW(I_PIVR), NASS, NPIVP1, LPIV, PP_LastPanelonDisk,
     &               PP_LastPIVRPTRIndexFilled)
              ENDIF
              NPIVP1 = NPIVP1+1
           ENDDO
           IF(PIVSIZ .EQ. 2) THEN
              A(POSELT+LDAFS8*int(NPIV,8)+int(NPIV+1,8)) = DETPIV
           ENDIF
           GOTO 420
  460   CONTINUE
          IF (K206 .GE. 1) THEN
            Inextpiv=IEND_BLOCK+1
          ENDIF
      IF (IEND_BLOCK.EQ.NASS) THEN
       INOPV = 1
      ELSE
       INOPV = 2
      ENDIF
      GO TO 420
  630 CONTINUE
      IFLAG = -10
  420 CONTINUE
      IF (K219.GE.2) THEN
       IF(INOPV .EQ. 0) THEN
         IF(PIVSIZ .EQ. 1) THEN
            GW_FACT = max(AMAX,RMAX_NOSLAVE)/ABS_PIVOT
         ELSE IF(PIVSIZ .EQ. 2) THEN
            GW_FACT = max(
     &          (abs(A(POSPV2))*RMAX_NOSLAVE+AMAX*TMAX_NOSLAVE) 
     &             /  ABSDETPIV ,
     &          (abs(A(POSPV1))*TMAX_NOSLAVE+AMAX*RMAX_NOSLAVE) 
     &            /  ABSDETPIV
     &          )
         ENDIF
         GW_FACT = min(GW_FACT, UULOCM1)  
         GW_FACTCUMUL = max(GW_FACT,GW_FACTCUMUL)
       ENDIF 
      ENDIF  
      RETURN
      END SUBROUTINE ZMUMPS_FAC_I_LDLT_NIV2
      SUBROUTINE ZMUMPS_FAC_MQ_LDLT_NIV2
     &     (IEND_BLOCK,
     &     NASS, NPIV, INODE, A, LA, LDAFS, 
     &     POSELT,IFINB,PIVSIZ,
     &     K219, PIVOT_OPTION, IEND_BLR)
      IMPLICIT NONE
      INTEGER(8), intent(in) :: LA, POSELT
      INTEGER,    intent(in) :: K219
      COMPLEX(kind=8), intent(inout) :: A(LA)
      INTEGER, intent(in)    :: IEND_BLOCK
      INTEGER, intent(in)    :: NPIV, PIVSIZ
      INTEGER, intent(in)    :: NASS,INODE,LDAFS
      INTEGER, intent(out)   :: IFINB
      INTEGER, intent(in)    :: PIVOT_OPTION, IEND_BLR
      COMPLEX(kind=8)    VALPIV
      INTEGER NCB1
      INTEGER(8) :: APOS, APOSMAX
      INTEGER(8) :: LPOS, LPOS1, LPOS2, K1POS 
      INTEGER(8) :: JJ, K1, K2
      INTEGER(8) :: POSPV1, POSPV2, OFFDAG, OFFDAG_OLD
      INTEGER(8) :: LDAFS8
      INTEGER NEL2
      COMPLEX(kind=8) ONE, ALPHA
      COMPLEX(kind=8) ZERO
      INTEGER NPIV_NEW, I
      INTEGER(8) :: IBEG, IEND, IROW, J8
      INTEGER    :: J2
      COMPLEX(kind=8) SWOP,DETPIV,MULT1,MULT2, A11, A22, A12
      PARAMETER (ONE=(1.0D0,0.0D0), ALPHA=(-1.0D0,0.0D0))
      PARAMETER (ZERO=(0.0D0,0.0D0))
      INCLUDE 'mumps_headers.h'
      LDAFS8 = int(LDAFS,8)
      NPIV_NEW = NPIV + PIVSIZ
      IFINB  = 0
      NEL2   = IEND_BLOCK - NPIV_NEW
      IF (NEL2.EQ.0) THEN
        IF (IEND_BLOCK.EQ.NASS) THEN
          IFINB        = -1
        ELSE
          IFINB        = 1
        ENDIF
      ENDIF
      IF(PIVSIZ .EQ. 1) THEN
         APOS   = POSELT + int(NPIV,8)*(LDAFS8 + 1_8)
         VALPIV = ONE/A(APOS)
         LPOS   = APOS + LDAFS8
         DO I = 1, NEL2
           K1POS = LPOS + int(I-1,8)*LDAFS8
           A(APOS+int(I,8))=A(K1POS)
           A(K1POS) = A(K1POS) * VALPIV
           DO JJ=1_8, int(I,8)
             A(K1POS+JJ)=A(K1POS+JJ) - A(K1POS) * A(APOS+JJ)
           ENDDO
         ENDDO
         IF (PIVOT_OPTION.EQ.2) THEN
           NCB1 = NASS - IEND_BLOCK
         ELSE
           NCB1 = IEND_BLR - IEND_BLOCK
         ENDIF
!$OMP    PARALLEL DO PRIVATE(JJ,K1POS) IF (NCB1 > 300)
         DO I=NEL2+1, NEL2 + NCB1
           K1POS = LPOS+ int(I-1,8)*LDAFS8
           A(APOS+int(I,8))=A(K1POS)
           A(K1POS) = A(K1POS) * VALPIV
           DO JJ = 1_8, int(NEL2,8)
             A(K1POS+JJ)=A(K1POS+JJ) - A(K1POS) * A(APOS+JJ)
           ENDDO
         ENDDO
!$OMP    END PARALLEL DO
         IF (K219.eq. -1) THEN
           APOSMAX = POSELT + int(NASS,8) * LDAFS8 + int(NPIV,8)
           A(APOSMAX) = A(APOSMAX) * abs(VALPIV)
           DO J8 = 1_8, int(NASS - NPIV_NEW,8) 
             A(APOSMAX+J8) = A(APOSMAX+J8) +
     &                      A(APOSMAX) * abs(A(APOS+J8))
           ENDDO
         ENDIF
      ELSE
         POSPV1 = POSELT + int(NPIV,8)*(LDAFS8 + 1_8)
         POSPV2 = POSPV1+LDAFS8+1_8
         OFFDAG_OLD = POSPV2 - 1_8
         OFFDAG = POSPV1+1_8
         SWOP = A(POSPV2)
         DETPIV = A(OFFDAG)
         A22 = A(POSPV1)/DETPIV   
         A11 =  SWOP/DETPIV       
         A12 = -A(OFFDAG_OLD)/DETPIV   
         A(OFFDAG)     = A(OFFDAG_OLD)  
         A(OFFDAG_OLD) = ZERO
         LPOS1   = POSPV2 + LDAFS8 - 1_8
         LPOS2   = LPOS1 + 1_8
         CALL zcopy(NASS-NPIV_NEW, A(LPOS1), LDAFS, A(POSPV1+2_8), 1)
         CALL zcopy(NASS-NPIV_NEW, A(LPOS2), LDAFS, A(POSPV2+1_8), 1)
         JJ = POSPV2 + int(NASS-1,8)
         IBEG = JJ + 2_8
         IEND = IBEG
         DO J2 = 1,NEL2
            K1 = JJ
            K2 = JJ+1_8
            MULT1 = - (A11*A(K1)+A12*A(K2))
            MULT2 = - (A12*A(K1)+A22*A(K2))
            K1 = POSPV1+2_8
            K2 = POSPV2+1_8
            DO IROW = IBEG,IEND
               A(IROW) = A(IROW) + MULT1*A(K1) + MULT2*A(K2)
               K1 = K1 + 1_8
               K2 = K2 + 1_8
            ENDDO
            A(JJ) = -MULT1
            A(JJ+1_8) = -MULT2
            IBEG = IBEG + int(NASS,8) 
            IEND = IEND + int(NASS + 1,8)
            JJ = JJ+int(NASS,8)
         ENDDO
         IEND = IEND-1_8
         DO J2 = IEND_BLOCK+1,NASS
            K1 = JJ
            K2 = JJ+1_8
            MULT1 = - (A11*A(K1)+A12*A(K2))
            MULT2 = - (A12*A(K1)+A22*A(K2))
            K1 = POSPV1+2_8
            K2 = POSPV2+1_8
            DO IROW = IBEG,IEND
               A(IROW) = A(IROW) + MULT1*A(K1) + MULT2*A(K2)
               K1 = K1 + 1_8
               K2 = K2 + 1_8
            ENDDO
            A(JJ) = -MULT1
            A(JJ+1_8) = -MULT2
            IBEG = IBEG + int(NASS,8) 
            IEND = IEND + int(NASS,8) 
            JJ = JJ+int(NASS,8) 
         ENDDO
         IF (K219.eq. -1) THEN
           APOSMAX = POSELT + int(NASS,8) * LDAFS8 + int(NPIV,8)
           JJ = APOSMAX
           K1 = JJ
           K2 = JJ + 1_8
           MULT1 = abs(A11)*A(K1)+abs(A12)*A(K2)
           MULT2 = abs(A12)*A(K1)+abs(A22)*A(K2)
           K1 = POSPV1 + 2_8
           K2 = POSPV2 + 1_8
           IBEG = APOSMAX + 2_8
           IEND = APOSMAX + 1_8 + NASS - NPIV_NEW
           DO IROW = IBEG,  IEND
             A(IROW) = A(IROW) + MULT1*abs(A(K1)) + MULT2*abs(A(K2))
             K1 = K1 + 1_8
             K2 = K2 + 1_8
           ENDDO
           A(JJ) = MULT1
           A(JJ+1_8) = MULT2
         ENDIF
      ENDIF
      RETURN
      END SUBROUTINE ZMUMPS_FAC_MQ_LDLT_NIV2
      SUBROUTINE  ZMUMPS_SEND_FACTORED_BLK( COMM_LOAD, ASS_IRECV, N,
     &             INODE, FPERE, IW, LIW, IOLDPS, POSELT, A, LA, LDA_FS,
     &             IBEG_BLOCK, IEND, TIPIV, LPIV, LASTBL, NB_BLOC_FAC,
     &             COMM, MYID, BUFR, LBUFR,LBUFR_BYTES,NBFIN,LEAF,
     &             IFLAG, IERROR, IPOOL,LPOOL, 
     &             SLAVEF, POSFAC, IWPOS, IWPOSCB, IPTRLU, LRLU,
     &             LRLUS, COMP, PTRIST, PTRAST, PTLUST_S, PTRFAC,
     &             STEP, PIMASTER, PAMASTER,
     &             NSTK_S,NBPROCFILS,PROCNODE_STEPS, root,
     &             OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &             FILS, DAD, PTRARW, PTRAIW,
     &             INTARR, DBLARR, ICNTL, KEEP,KEEP8,DKEEP, ND, FRERE,
     &             LPTRAR, NELT, FRTPTR, FRTELT, 
     &             ISTEP_TO_INIV2, TAB_POS_IN_PERE
     &             , NELIM, LR_ACTIVATED, NPARTSASS, CURRENT_BLR_PANEL
     &             , BLR_LorU 
     &             , LRGROUPS
     &            )
      USE ZMUMPS_BUF
      USE ZMUMPS_LOAD
      USE ZMUMPS_LR_TYPE
      USE ZMUMPS_STRUC_DEF, ONLY : ZMUMPS_ROOT_STRUC
      IMPLICIT NONE
      INCLUDE 'mpif.h'
      TYPE (ZMUMPS_ROOT_STRUC) :: root
      INTEGER COMM_LOAD, ASS_IRECV
      INTEGER N, INODE, FPERE, LIW, IBEG_BLOCK, IEND, LPIV, 
     &        IOLDPS, LDA_FS, NB_BLOC_FAC
      INTEGER(8) :: POSELT, LA
      INTEGER IW(LIW), TIPIV(LPIV)
      LOGICAL LASTBL
      COMPLEX(kind=8) A(LA)
      INTEGER COMM, MYID, LBUFR, LBUFR_BYTES
      INTEGER NELT, LPTRAR
      INTEGER FRTPTR( N+1 ), FRTELT( NELT ) 
      INTEGER KEEP(500)
      INTEGER(8) KEEP8(150)
      DOUBLE PRECISION    DKEEP(230)
      INTEGER NBFIN, IFLAG, IERROR, LEAF, LPOOL,
     &        SLAVEF, ICNTL(60)
      INTEGER(8) :: POSFAC, IPTRLU, LRLU, LRLUS
      INTEGER IWPOS, IWPOSCB, COMP
      INTEGER BUFR( LBUFR ), IPOOL(LPOOL),
     &        ITLOC(N+KEEP(253)), FILS(N), DAD( KEEP(28) ),
     &        ND( KEEP(28) ), FRERE( KEEP(28) )
      INTEGER(8), INTENT(IN) :: PTRARW(LPTRAR), PTRAIW(LPTRAR)
      COMPLEX(kind=8) :: RHS_MUMPS(KEEP(255))
      INTEGER(8) :: PTRAST  (KEEP(28))
      INTEGER(8) :: PTRFAC  (KEEP(28))
      INTEGER(8) :: PAMASTER(KEEP(28))
      INTEGER PTRIST(KEEP(28)), PTLUST_S(KEEP(28)),
     &        STEP(N), PIMASTER(KEEP(28)),
     &        NSTK_S(KEEP(28)),
     &        NBPROCFILS(KEEP(28)), PROCNODE_STEPS(KEEP(28))
      INTEGER ISTEP_TO_INIV2(KEEP(71)), 
     &        TAB_POS_IN_PERE(SLAVEF+2,max(1,KEEP(56)))
      DOUBLE PRECISION OPASSW, OPELIW
      COMPLEX(kind=8) DBLARR(KEEP8(26))
      INTEGER INTARR(KEEP8(27))
      LOGICAL, intent(in) ::  LR_ACTIVATED  
      TYPE (LRB_TYPE), DIMENSION(:) :: BLR_LorU
      INTEGER, intent(in) :: LRGROUPS(N)
      INTEGER ::  NELIM
      INTEGER, intent(in) :: NPARTSASS, CURRENT_BLR_PANEL
      INCLUDE 'mumps_headers.h'
      INTEGER(8) :: APOS, LREQA
      INTEGER NPIV, NCOL, PDEST, NSLAVES, WIDTH
      INTEGER IERR, LREQI
      INTEGER :: STATUS(MPI_STATUS_SIZE)
      LOGICAL BLOCKING, SET_IRECV, MESSAGE_RECEIVED
      DOUBLE PRECISION FLOP1,FLOP2
      LOGICAL COMPRESS_CB
      COMPRESS_CB    = ((IW(IOLDPS+XXLR).EQ.1).OR.
     &                    (IW(IOLDPS+XXLR).EQ.3))
      NSLAVES= IW(IOLDPS+5+KEEP(IXSZ))
      IF (NSLAVES.EQ.0) THEN
           WRITE(6,*) ' ERROR 1 in ZMUMPS_SEND_FACTORED_BLK '
           CALL MUMPS_ABORT()
          ENDIF
      NPIV   = IEND - IBEG_BLOCK + 1
      NCOL   = LDA_FS - IBEG_BLOCK + 1
      APOS   = POSELT + int(LDA_FS,8)*int(IBEG_BLOCK-1,8) +
     &                  int(IBEG_BLOCK - 1,8)
      IF (IBEG_BLOCK > 0) THEN
       CALL MUMPS_GET_FLOPS_COST( LDA_FS, IBEG_BLOCK-1, LPIV,
     &                            KEEP(50),2,FLOP1)
      ELSE
        FLOP1=0.0D0
      ENDIF
      CALL MUMPS_GET_FLOPS_COST( LDA_FS, IEND, LPIV,
     &                           KEEP(50),2,FLOP2)
      FLOP2 = FLOP1 - FLOP2
      CALL ZMUMPS_LOAD_UPDATE(1, .FALSE., FLOP2, KEEP,KEEP8)
      IF ((NPIV.GT.0) .OR. 
     &    ((NPIV.EQ.0).AND.(LASTBL))
     &   ) THEN
        IF ((NPIV.EQ.0).AND.(LASTBL)) THEN
          IF (COMPRESS_CB) THEN
          IW(IOLDPS+XXLR) = IW(IOLDPS+XXLR) -1
          COMPRESS_CB = .FALSE.
          ENDIF
        ENDIF
        PDEST  = IOLDPS + 6 + KEEP(IXSZ)
        IF (( NPIV .NE. 0 ).AND.(KEEP(50).NE.0)) THEN
          NB_BLOC_FAC = NB_BLOC_FAC + 1
        END IF
        IERR = -1
        DO WHILE (IERR .EQ.-1)
          WIDTH = NSLAVES   
          CALL ZMUMPS_BUF_SEND_BLOCFACTO( INODE, LDA_FS, NCOL, 
     &               NPIV, FPERE, LASTBL, TIPIV, A(APOS),
     &               IW(PDEST), NSLAVES, KEEP,
     &               NB_BLOC_FAC,
     &               NSLAVES, WIDTH, COMM,
     &               NELIM, NPARTSASS, CURRENT_BLR_PANEL,
     &               LR_ACTIVATED, BLR_LorU, 
     &        IERR )
          IF (IERR.EQ.-1) THEN
            BLOCKING  = .FALSE.
            SET_IRECV = .TRUE.
            MESSAGE_RECEIVED = .FALSE.
            CALL ZMUMPS_TRY_RECVTREAT( COMM_LOAD, ASS_IRECV, 
     &       BLOCKING, SET_IRECV, MESSAGE_RECEIVED,
     &       MPI_ANY_SOURCE, MPI_ANY_TAG,
     &       STATUS, BUFR, LBUFR,
     &       LBUFR_BYTES,
     &       PROCNODE_STEPS, POSFAC, IWPOS, IWPOSCB, IPTRLU,
     &       LRLU, LRLUS, N, IW, LIW, A, LA, PTRIST,
     &       PTLUST_S, PTRFAC,
     &       PTRAST, STEP, PIMASTER, PAMASTER, NSTK_S, COMP, IFLAG,
     &       IERROR, COMM,
     &       NBPROCFILS,
     &       IPOOL, LPOOL, LEAF, NBFIN, MYID, SLAVEF,
     &       root, OPASSW, OPELIW, ITLOC, RHS_MUMPS,
     &       FILS, DAD, PTRARW, PTRAIW,
     &       INTARR, DBLARR, ICNTL, KEEP,KEEP8,DKEEP, ND, FRERE,
     &       LPTRAR, NELT, FRTPTR, FRTELT, 
     &       ISTEP_TO_INIV2, TAB_POS_IN_PERE, .TRUE.
     &               , LRGROUPS
     &        )
            IF (MESSAGE_RECEIVED) THEN
              POSELT = PTRAST(STEP(INODE))
              APOS   = POSELT + int(LDA_FS,8)*int(IBEG_BLOCK-1,8) +
     &                 int(IBEG_BLOCK - 1,8)
            ENDIF
            IF ( IFLAG .LT. 0 ) GOTO 500
          ENDIF
        ENDDO
        IF (IERR .EQ. -2 .OR. IERR.EQ.-3 ) THEN
          IF (IERR.EQ.-2) IFLAG = -17
          IF (IERR.EQ.-3) IFLAG = -20
          LREQA = int(NCOL,8)*int(NPIV,8)
          LREQI = NPIV + 6 + 2*NSLAVES + 2
          CALL MUMPS_SET_IERROR(
     &    int(LREQI,8) * int(KEEP(34),8) + LREQA * int(KEEP(35),8),
     &    IERROR)
          GOTO 300
        ENDIF
      ENDIF
      GOTO 500
  300 CONTINUE
      CALL ZMUMPS_BDC_ERROR( MYID, SLAVEF, COMM, KEEP )
  500 CONTINUE
      RETURN
      END SUBROUTINE  ZMUMPS_SEND_FACTORED_BLK
      END MODULE ZMUMPS_FAC_FRONT_TYPE2_AUX_M
