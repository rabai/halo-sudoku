# halo-sudoku
2 player Sudoku game through sockets in C
1: gcc szerver.c -o szerver
2: gcc kliens.c -o kliens
3: start a terminal at the "szerver" directory, then: ./szerver
4: start 2 terminal at the "kliens" directory, then: ./kliens 51716 (port number stated in the szerver.c file) in both
5: the first accepted player starts, then the second, and so on, with the commands on the terminal window
