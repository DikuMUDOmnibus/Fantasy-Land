#CC      = ccache gcc
CC      = gcc
PROF    = -O -ggdb
C_FLAGS = -Wall -Dlinux $(PROF)
L_FLAGS = $(PROF)

ROT_FILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o  \
	    arena.o ban.o bit.o chart.o clans.o comm.o const.o db.o effects.o \
	    fight.o flags.o games.o gquest.o handler.o interp.o magic.o       \
	    mob_cmds.o mob_prog.o note.o olc.o olc_act.o olc_save.o pathfind.o\
	    quest.o rand_obj.o recycle.o save.o skills.o special.o string.o   \
	    tables.o update.o wizlist.o

all:: rot

rot: $(ROT_FILES)
	rm -f rot
	$(CC) $(L_FLAGS) -o rot $(ROT_FILES) -lcrypt -lz

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o
	make

copy:
	cp -f rot ../area/

done:
	rm *.o
	rm -f rot
