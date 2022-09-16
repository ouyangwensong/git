obj=jpeg.o
jpeg : $(obj)
	cc $(obj) -o jpeg
jpeg.o : jpeg.c bmp.h
.PHONY : clean
clean :
	-rm jpeg $(obj)
