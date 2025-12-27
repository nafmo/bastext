# Makefile for bastext -------------------------------------------------------
OBJS=main.o inmode.o outmode.o tokens.o tokenize.o dtokeniz.o select.o t64.o utility.o
CFLAGS=-Wall

# All targets ----------------------------------------------------------------
all: bastext

# Main executable ------------------------------------------------------------
bastext: $(OBJS)
	$(CC) -o $@ $^

tokens.o: tokens.c tokens.h
tokenize.o: tokenize.c tokenize.h tokens.h
dtokeniz.o: dtokeniz.c tokenize.h tokens.h
main.o: main.c inmode.h outmode.h tokenize.h
inmode.o: inmode.c tokenize.h version.h inmode.h select.h t64.h
outmode.o: outmode.c tokenize.h version.h outmode.h select.h t64.h
select.o: select.c select.h tokenize.h
t64.o: t64.c t64.h
utility.o: utility.c utility.h

# Cleanup --------------------------------------------------------------------
clean:
	$(RM) bastext core *.o *~
