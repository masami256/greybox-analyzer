target = prog

CC = clang

objs = prog.o \
	greeting.o

all: $(objs)
	${CC} ${CFLAGS} ${LDFLAGS} ${objs} -o ${target}

.c.o:
	${CC} -I. -g -emit-llvm  ${CFLAGS} -c $< -o $(@:.o=).bc
	${CC} -I. -g ${CFLAGS} -c $<

clean:
	rm -fr ${target} *.o *.bc