EXE = mines
EXEFLAGS = -std=c89 -Wall -Wextra -Wpedantic ${CFLAGS}
RM ?= rm -f
source = mines.c

$(EXE): $(source)
	$(CC) $(EXEFLAGS) -o $@ $<

clean:
	$(RM) $(EXE)
