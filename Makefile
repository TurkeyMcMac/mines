EXE = mines
EXEFLAGS = -std=c89 -Wall -Wextra -Wpedantic ${CFLAGS}
source = mines.c

$(EXE): $(source)
	$(CC) $(EXEFLAGS) -o $@ $<

clean:
	rm -f $(EXE)
