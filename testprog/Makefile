target := prog

CC := clang
BUILDDIR := build

SRCS := prog.c greeting.c
OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BUILDDIR)/$(target)
	$(AFLGO)/distance/gen_distance_orig.sh $(BUILDDIR) $(BUILDDIR)/temp prog

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) -I. -g -emit-llvm  $(CFLAGS) -c $< -o $(@:.o=).bc
	$(CC) -I. -g $(CFLAGS) -c $< -o $@
	
$(BUILDDIR):
	mkdir -p $(BUILDDIR)/temp

clean:
	rm -fr $(BUILDDIR)
