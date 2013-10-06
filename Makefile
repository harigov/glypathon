OBJDIR=obj
SRCDIR=src
APPNAME=app.bin

CC=g++
CFLAGS+=-std=c++11 -stdlib=libc++ -g -Wall \
		`pkg-config opencv sdl2 SDL2_image --cflags`
LFLAGS+=-std=c++11 -stdlib=libc++ \
		`pkg-config opencv sdl2 SDL2_image --libs`

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
	$(CC) $(CFLAGS) $(SRCDIR)/$*.cc -c -o $@
	$(CC) -MM $(CFLAGS) $(SRCDIR)/$*.cc > $(OBJDIR)/$*.d
	@mv -f $(OBJDIR)/$*.d $(OBJDIR)/$*.d.tmp
	@sed -e 's,.*:,$(OBJDIR)/$*.o:,' < $(OBJDIR)/$*.d.tmp > $(OBJDIR)/$*.d
	@rm -f $(OBJDIR)/$*.d.tmp

clean:
	rm -rf $(OBJDIR)
	rm -f $(APPNAME)
