clean:
		rm -f *.o
		rm -f *.out

%.o: %.s %.cfg
	../cc65/bin/ca65 $< -o $@

%.out: %.o %.cfg
	../cc65/bin/ld65 -o $@ -C $*.cfg $<