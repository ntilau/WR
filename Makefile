.SUFFIXES: .cpp .o

MSG = $(shell date +%Y%m%d%H%M)

ARCH = $(shell uname -m)
PLAT = $(shell uname -s | tr '[:upper:]' '[:lower:]')-gnu

# ARCH = x86_64
# PLAT = w64-mingw32

CC=$(ARCH)-$(PLAT)-g++ -w
CC = g++ -w

BIN = fes
SRCDIR = ./src

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
LIBS = -lsmumps -ldmumps -lcmumps -lzmumps -lmumps_common -lmpiseq -lpord -lopenblas -larpack -lgfortran -ltet

ifdef OS
	LIBS = $(LIBS) -lquadmath -lpthread
endif

CFLAGS = $(INCDIR) -std=c++17 -static
LFLAGS = $(LIBDIR) -std=c++17 $(LIBS)

SRCS=$(wildcard $(SRCDIR)/*.cpp)
OBJS=$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

ifdef OS
  RM = del /F /S /Q
  FixPath = $(subst /,\,$1)
else
  RM = rm -f
  FixPath = $1
endif

all: $(BINDIR) $(OBJDIR) $(OBJS) $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BINDIR)/$@ $^ $(LFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
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
