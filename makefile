CC = gcc
OBJS = tests/electionTestsExample.o election.o mtm_map/map.o area.o info.o utils.o mtm_map/key.o
EXEC = election
DEBUG = -g
CFLAGS = -std=c99 -Wall -Werror -pedantic-errors -DNDEBUG $(DEBUG) 

$(EXEC) : $(OBJS)
	$(CC) $(DEBUG_FLAG) $(OBJS) -o $@

tests/electionTestsExample.o: tests/electionTestsExample.c election.h mtm_map/map.h test_utilities.h
area.o: area.c mtm_map/map.h area.h info.h utils.h
election.o: election.c mtm_map/map.h election.h area.h info.h utils.h
info.o: info.c info.h
utils.o: utils.c utils.h
mtm_map/key.o: mtm_map/key.c mtm_map/key.h
mtm_map/map.o: mtm_map/map.c mtm_map/map.h mtm_map/key.h

clean:
	rm -f $(OBJS)

