#include <libsmbclient.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <mysql.h>
#include <stdlib.h>

int debug = 0;
MYSQL *conn;

static void
get_auth_data (const char * srv, const char * shr, char * wg, int wglen,
	       char * un, int unlen, char * pw, int pwlen){
	return;
}

void usage(void) {
    printf("Usage: ssb smb://sharename [-d || --debug]\
            \nIf sharename is omitted, whole network is being scanned\n");
    exit(0);
}

void scan (char* path) {
	char p[1024], query[1024];
    int dir;
	struct smbc_dirent * dirent;
	struct stat stat;

	if ((dir = smbc_opendir(path)) < 0) {
		if (debug) fprintf(stderr, "Couldn't open %s (%s)\n", path, strerror(errno));
		return;
	} 

	while ((dirent = smbc_readdir(dir)) != NULL) {
		if (!strcmp(dirent -> name, ".") ||
		    !strcmp(dirent -> name, "..")) {
			continue;
		}

        sprintf(p, "%s/%s", path, dirent->name);
		if (debug) fprintf(stderr, "%s ", p);

		if (dirent->smbc_type == SMBC_SERVER){
            sprintf(p, "smb://%s", dirent->name);
            scan(p);

        } else //dirent type is not server
            if (dirent->smbc_type == SMBC_WORKGROUP ||
            dirent->smbc_type == SMBC_DIR ||
            dirent->smbc_type == SMBC_FILE_SHARE) {
			if (debug) fprintf(stderr, "\nscan(%s)\n", p);
			scan(p);

		} else //neither server nor wg, directory or share, so it's a file
            if (dirent -> smbc_type == SMBC_FILE) {
			if (smbc_stat(p, &stat) < 0) { 
				if (debug) fprintf(stderr, "stat() failed (%s)\n", strerror(errno));
			} else { //file exists
				if (debug) fprintf(stderr, "(%lu kb)\n", stat.st_size / 1024);
                sprintf(query, "insert into srchdb.files values(\"%s\", %llu, \"%s\")", 
                        dirent->name, stat.st_size, p);
                if (mysql_query(conn, query)) {
                    if (debug) fprintf(stderr, "FAILED \"%s\":\n\t%s\n", query, mysql_error(conn));
                }
			} 

		} else //otherwise, something went wrong
            if (debug) {
			fprintf(stderr, "UNHANDLED: %s ", dirent -> name);
			switch (dirent -> smbc_type){
				case SMBC_PRINTER_SHARE:
					fprintf(stderr, "(PRINTER_SHARE)\n"); break;
				case SMBC_COMMS_SHARE:
					fprintf(stderr, "(COMMS_SHARE)\n"); break;
				case SMBC_IPC_SHARE:
					fprintf(stderr, "(IPC_SHARE)\n"); break;
				case SMBC_LINK:
					fprintf(stderr, "(LINK)\n"); break;
				default:
					fprintf(stderr, "(%d)\n", dirent->smbc_type);
			}
		} //handling of unhandled
	}
    smbc_closedir(dir);
}

int main (int argc, char* argv[]) {
    /*
     * ./ssb smb://sharename [-d || --debug] 
     */
	
    if (argc < 1) usage();

    if (smbc_init(get_auth_data, 0)) {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }

    if ((conn = mysql_init(NULL)) == NULL) {
        fprintf(stderr, "mysql_init() failed: %s\n", mysql_error(conn));
        return 1;
    }
    if (mysql_real_connect(conn, "localhost", "skysearch", "0", "srchdb", 0, NULL, 0) == NULL) {
        fprintf(stderr, "connect to srchdb failed: %s\n", mysql_error(conn));
        return 1;
    }

    mysql_query(conn, "create table files(name varchar(256), size bigint unsigned, fullpath varchar(1024))");

	if ((argc > 2) && (!strcmp(argv[2], "-d") || 
                       !strcmp(argv[2], "--debug"))) debug = 1;
    if ((argc > 1) && (!strncmp(argv[1], "smb://", 6))) scan(argv[1]);
    else usage();
    
    mysql_close(conn);

	return 0;
}
