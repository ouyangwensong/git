obj=cp.o
cp : $(obj)
	cc $(obj) -o cp
cp.o : cp.c
.PHONY : clean
clean :
	-rm cp $(obj)