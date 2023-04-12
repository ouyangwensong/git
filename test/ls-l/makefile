obj=ls-l.o
ls-l : $(obj)
	cc $(obj) -o ls-l
ls-l.o : ls-l.c
.PHONY : clean
clean :
	-rm ls-l $(obj)