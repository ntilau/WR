###########################################################################
#
#  Program:         ARPACK
#
#  Module:          ARmake.inc
#
#  Purpose:         Top-level Definitions
#
#  Creation date:   February 22, 1996
#
#  Modified:
#
#  Send bug reports, comments or suggestions to arpack@caam.rice.edu
#
############################################################################
#
# %---------------------------------%
# |  SECTION 1: PATHS AND LIBRARIES |
# %---------------------------------%
#
#
# %--------------------------------------%
# | You should change the definition of  |
# | home if ARPACK is built some place   | 
# | other than your home directory.      |
# %--------------------------------------%
#
home = $(HOME)/WR/dep/src/ARPACK
#home = $(HOME)/ARPACK
#
#  %--------------------------------------%
#  | The platform identifier to suffix to |
#  | the end of library names             |
#  %--------------------------------------%
#
PLAT = 
#
#  %------------------------------------------------------%
#  | The directories to find the various pieces of ARPACK |
#  %------------------------------------------------------%
#
BLASdir      = $(home)/BLAS
LAPACKdir    = $(home)/LAPACK
UTILdir      = $(home)/UTIL
SRCdir       = $(home)/SRC
#
DIRS        = $(BLASdir) $(LAPACKdir) $(UTILdir) $(SRCdir)
#
# %-------------------------------------------------------------------%
# | Comment out the previous line and uncomment the following         |
# | if you already have the BLAS and LAPACK installed on your system. |
# | NOTE: ARPACK assumes the use of LAPACK version 2 codes.           |
# %-------------------------------------------------------------------%
#
#DIRS         = $(UTILdir) $(SRCdir)
#
# %---------------------------------------------------%
# | The name of the libraries to be created/linked to |
# %---------------------------------------------------%
#
ARPACKLIB  = $(home)/libarpack.a
LAPACKLIB = 
BLASLIB = 
#
ALIBS =  $(ARPACKLIB) $(LAPACKLIB) $(BLASLIB) 
#
# 
# %---------------------------------------------------------%
# |                  SECTION 2: COMPILERS                   |
# |                                                         |
# | The following macros specify compilers, linker/loaders, |
# | the archiver, and their options.  You need to make sure |
# | these are correct for your system.                      |
# %---------------------------------------------------------%
#
#
# %------------------------------%
# | Make our own suffixes' list. |
# %------------------------------%
#
.SUFFIXES:
.SUFFIXES:	.f	.o
#
# %------------------%
# | Default command. |
# %------------------%
#
.DEFAULT:
	@$(ECHO) "Unknown target $@, try:  make help"
#
# %-------------------------------------------%
# |  Command to build .o files from .f files. |
# %-------------------------------------------%
#
.f.o:
	@$(ECHO) Making $@ from $<
	@$(FC) -c $(FFLAGS) $<
#
# %-----------------------------------------%
# | Various compilation programs and flags. |
# | You need to make sure these are correct |
# | for your system.                        |
# %-----------------------------------------%
#
FC      = gfortran 
FFLAGS	= -O -ff2c -fallow-argument-mismatch

LDFLAGS = 
CD      = cd

ECHO    = echo

LN      = ln 
LNFLAGS = -s 

MAKE    = make

RM      = rm
RMFLAGS = -rf

SHELL   = /bin/sh
#
#  %----------------------------------------------------------------%
#  | The archiver and the flag(s) to use when building an archive   |
#  | (library).  Also the ranlib routine.  If your system has no    |
#  | ranlib, set RANLIB = touch.                                    |
#  %----------------------------------------------------------------%
#
AR = ar 
ARFLAGS = rv
#RANLIB  = touch
RANLIB   = ranlib
#
# %----------------------------------%
# | This is the general help target. |
# %----------------------------------%
#
help:
	@$(ECHO) "usage: make ?"
