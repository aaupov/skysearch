ssb:
	gcc sky_search_backend.c -o ssb -O2 -lsmbclient `mysql_config --cflags --libs`

