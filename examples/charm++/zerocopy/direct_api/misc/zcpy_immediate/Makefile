-include ../../../../../common.mk
CHARMC=../../../../../../bin/charmc $(OPTS)

all: zcpy_immediate

OBJS = zcpy_immediate.o

zcpy_immediate: $(OBJS)
	$(CHARMC) -language charm++ -o zcpy_immediate $(OBJS)

cifiles: zcpy_immediate.ci
	$(CHARMC)  zcpy_immediate.ci
	touch cifiles

zcpy_immediate.o: zcpy_immediate.C cifiles
	$(CHARMC) -c zcpy_immediate.C

test: all
	$(call run, +p1 ./zcpy_immediate 20 )
	$(call run, +p2 ./zcpy_immediate 20 )

clean:
	rm -f *.decl.h *.def.h conv-host *.o zcpy_immediate charmrun cifiles
