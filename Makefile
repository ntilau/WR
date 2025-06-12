.SUFFIXES: .cpp .o

MSG = $(shell date +%Y%m%d%H%M)

ARCH = $(shell uname -m)
PLAT = $(shell uname -s | tr '[:upper:]' '[:lower:]')-gnu

# ARCH = x86_64
# PLAT = w64-mingw32

CC=$(ARCH)-$(PLAT)-g++ -w
CC = g++ -w

WRBIN = wr
WRSRX = ./src

FEBIN = fes
FESRC = ./src_old

BINDIR  = ./bin/$(ARCH)-$(PLAT)
OBJDIR  = ./obj/$(ARCH)-$(PLAT)

INCDIR = \
	-I./dep/include \
	-I./dep/include/tetgen \
	-I./dep/include/mumps/include \
	-I./dep/include/arma/include \
	-I./dep/include/metis \
	-I./dep/include/gmm \
	-I./dep/include/metis \
	-DTETLIBRARY
LIBDIR = -L./dep/lib/$(ARCH)-$(PLAT) 
INCDIR = -L/usr/local/include
LIBDIR = -L/usr/local/lib
LIBS = -lsmumps -ldmumps -lcmumps -lzmumps -lmumps_common -lmpiseq -lpord -lopenblas -larpack -ltet -lgfortran -lquadmath
ifdef OS
	LIBS = $(LIBS) -lquadmath -lpthread
endif

CFLAGS = $(INCDIR) -std=c++14 
LFLAGS = $(LIBDIR) -std=c++14 $(LIBS)

FEOBJS=$(patsubst $(FESRC)/%.cpp, $(OBJDIR)/%.o, $(wildcard $(FESRC)/*.cpp))
WROBJS=$(patsubst $(WRSRC)/%.cpp, $(OBJDIR)/%.o, $(wildcard $(WRSRC)/*.cpp))

ifdef OS
  RM = del /F /S /Q
  FixPath = $(subst /,\,$1)
else
  RM = rm -f
  FixPath = $1
endif

all: $(BINDIR) $(OBJDIR) $(WROBJS) $(FEOBJS) $(WRBIN) $(FEBIN)

$(WRBIN): $(WROBJS)
	$(CC) -o $(BINDIR)/$@ $^ $(LFLAGS)

$(FEBIN): $(FEOBJS)
	$(CC) -o $(BINDIR)/$@ $^ $(LFLAGS)


$(OBJDIR)/%.o: $(WRSRC)/%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@

$(OBJDIR)/%.o: $(FESRC)/%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@

$(BINDIR):
	if [ ! -d "$(BINDIR)" ]; then mkdir -p $(BINDIR); fi
	
$(OBJDIR):
	if [ ! -d "$(OBJDIR)" ]; then mkdir -p $(OBJDIR); fi

.PHONY: clean
clean:
	$(RM) $(call FixPath,$(OBJDIR)/*.o)

.PHONY: push
push:
	git add --all
	git commit -m $(MSG)
	git pull --rebase
	git push -u origin master

.PHONY: test
test:
	$(BINDIR)/$(BIN) models/WR10_3 100e9 +p 3 +tfe
