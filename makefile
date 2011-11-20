ssb:
	gcc sky_search_backend.c -o ssb -O2 -lsmbclient `mysql_config --cflags --libs`
debug:
	gcc sky_search_backend.c -o ssb -O0 -lsmbclient `mysql_config --cflags --libs` -ggdb
