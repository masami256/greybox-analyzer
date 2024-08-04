target = prog

CC = clang

all:
	${CC} -I. -g -emit-llvm  ${CFLAGS} -c ${target}.c -o ${target}.bc
	${CC} -I. -g  ${CFLAGS} -c ${target}.c
	${CC} ${CFLAGS} ${LDFLAGS} ${target}.o -o ${target}

cleanall:
	rm -fr ${target} obj-aflgo *.bc *.txt *.o

clean:
	rm -fr ${target} *.o *.bc