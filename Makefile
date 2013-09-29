OBJDIR=obj
SRCDIR=src
APPNAME=app.bin

CC=g++
CFLAGS+=-g -c -Wall `pkg-config opencv --cflags`
LFLAGS+=`pkg-config opencv --libs`

ifdef REL
	CFLAGS+=-O3
endif

ifdef DBG
	CFLAGS+=-fno-inline
endif

SRCS=$(wildcard $(SRCDIR)/*.cc)
OBJS=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(notdir $(SRCS)))))

# rule to create the library
all: $(OBJS)
	$(CC) -o $(APPNAME) $^ $(LFLAGS)

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $(CFLAGS) $^ > $(OBJDIR)/$*.d

clean:
	rm -rf $(OBJDIR)
	rm -f $(APPNAME)
