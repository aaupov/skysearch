#include <libsmbclient.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <mysql.h>

int debug = 0;
MYSQL *conn;

static void
get_auth_data (const char * srv, const char * shr, char * wg, int wglen,
	       char * un, int unlen, char * pw, int pwlen)
{
	return;
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
		if (debug) fprintf(stderr, "\nProcessing %s on %s\n", dirent -> name, path);

		if (!strcmp(dirent -> name, ".") ||
		    !strcmp(dirent -> name, "..")) {
			continue;
		}

		strcpy(p, path);
		strcat(p, "/");
		strcat(p, dirent -> name);
		//if (!debug) printf("%s\n", path);

		if (dirent -> smbc_type == SMBC_DIR ||
		    dirent -> smbc_type == SMBC_FILE_SHARE) {
			if (debug) fprintf(stderr, "scan(%s)\n", p);
			scan(p);
		} else if (dirent -> smbc_type == SMBC_FILE) {
			if (smbc_stat(p, &stat) < 0) {
				if (debug) fprintf(stderr, "stat() failed (%s)\n", strerror(errno));
			} else {
				if (debug) printf("%s (%lu kb)\n", dirent -> name, stat.st_size / 1024);
                sprintf(query, "insert into files values(\"%s\", %llu, \"%s\")", 
                        dirent->name, stat.st_size, p);
                if (mysql_query(conn, query)) {
                    if (debug) fprintf(stderr, "FAILED \"%s\":\n\t%s\n", query, mysql_error(conn));
                }
			}
		} else if (debug) {
			fprintf(stderr, "UNHANDLED: %s", dirent -> name);
			switch (dirent -> smbc_type){
				case SMBC_WORKGROUP:
					fprintf(stderr, "(WORKGROUP)\n"); break;
				case SMBC_PRINTER_SHARE:
					fprintf(stderr, "(PRINTER_SHARE)\n"); break;
				case SMBC_COMMS_SHARE:
					fprintf(stderr, "(COMMS_SHARE)\n"); break;
				case SMBC_IPC_SHARE:
					fprintf(stderr, "(IPC_SHARE)\n"); break;
				case SMBC_LINK:
					fprintf(stderr, "(LINK)\n"); break;
				default:
					fprintf(stderr, "(this should never happen)\n");
			}
		}
	}
    smbc_closedir(dir);
}

int main (int argc, char* argv[]) {
	int root; 
	//int i;
	char p[255];
	struct smbc_dirent * dirent;
	
	if ((argc > 1) && (!strcmp(argv[1], "-d") || 
                       !strcmp(argv[1], "--debug"))) debug = 1;

/* Initialisation */
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

	if ((root = smbc_opendir("smb://SKY")) < 0) {
		fprintf(stderr, "Couldn't get list of workgroups (%s)\n", 
                strerror(errno)); 
		return 1;
	} 

    mysql_query(conn, "create table files(name varchar(256), size bigint unsigned, fullpath varchar(1024))");

/* Main */
	while ((dirent = smbc_readdir(root)) != NULL) {
		if (dirent -> smbc_type == SMBC_SERVER) {
			sprintf(p, "smb://%s", dirent -> name);
			scan(p);
		} else if (debug) fprintf(stderr, "NOSCAN %s\n", dirent -> name);
	}

/* Quit */
	smbc_closedir(root);
    mysql_close(conn);

	return 0;
}
