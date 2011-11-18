#include <libsmbclient.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>

int debug = 0;

static void
get_auth_data (const char * srv, const char * shr, char * wg, int wglen,
	       char * un, int unlen, char * pw, int pwlen)
{
	return;
}

void scan (char* path) {
	char p[256];
	int dir;
	struct smbc_dirent * dirent;
	struct stat stat;

	if ((dir = smbc_opendir(path)) < 0) {
		if (debug) printf("Couldn't open %s (%s)\n", path, strerror(errno));
		return;
	} 

	while ((dirent = smbc_readdir(dir)) != NULL) {
		if (debug) printf("\nProcessing %s on %s\n", dirent -> name, path);

		if (!strcmp(dirent -> name, ".") ||
		    !strcmp(dirent -> name, "..")) {
			printf("%s\n", dirent -> name);
			continue;
		}

		if (dirent -> smbc_type == SMBC_DIR ||
		    dirent -> smbc_type == SMBC_FILE_SHARE) {
			if (debug) printf("scan(%s/%s)\n", path, dirent -> name);
			strcpy(p, path);
			strcat(p, "/");
			strcat(p, dirent -> name);
			if (!debug) printf("\n%s\n", path);
			scan(p);
		} else if (dirent -> smbc_type == SMBC_FILE) {
			if (smbc_stat(path, &stat) < 0) {
				if (debug) printf("stat() failed (%s)\n", strerror(errno));
			} else {
				printf("%s (%lu kb)\n", dirent -> name, stat.st_size / 1024);
			}
		} else if (debug) {
			printf("UNHANDLED: %s ", dirent -> name);
			switch (dirent -> smbc_type){
				case SMBC_WORKGROUP:
					printf("(WORKGROUP)\n"); break;
				case SMBC_PRINTER_SHARE:
					printf("(PRINTER_SHARE)\n"); break;
				case SMBC_COMMS_SHARE:
					printf("(COMMS_SHARE)\n"); break;
				case SMBC_IPC_SHARE:
					printf("(IPC_SHARE)\n"); break;
				case SMBC_LINK:
					printf("(LINK)\n"); break;
				default:
					printf("(this should never happen)\n");
			}
		}
	}
}

int main (int argc, char* argv[]) {
	int root; 
	//int i;
	char p[255];
	struct smbc_dirent * dirent;
	
	if ((argc > 1) && (!strcmp(argv[1], "-d") || 
                       !strcmp(argv[1], "--debug"))) debug = 1;
	
	if (smbc_init(get_auth_data, 0)) printf("%s\n", strerror(errno));
	if ((root = smbc_opendir("smb://SKY")) < 0) {
		if (debug) printf("Couldn't get list of workgroups (%s)\n", 
                strerror(errno)); 
		return 0;
	} 

	while ((dirent = smbc_readdir(root)) != NULL) {
		if (dirent -> smbc_type == SMBC_SERVER) {
			sprintf(p, "smb://%s", dirent -> name);
			scan(p);
		} else if (debug) printf("NOSCAN %s\n", dirent -> name);
	}
	smbc_closedir(root);
	return 0;
}
