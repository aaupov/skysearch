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
	//char* path;// = strcat(base, offset);
	char* p;
	int dir;
	struct smbc_dirent * dirent;
	struct stat stat;
	//strcat(base, "/");
	//path = strcat(base, offset);
	if ((dir = smbc_opendir(path)) < 0) {
		if (debug) printf("Couldn't open %s (%s)\n", path, strerror(errno));
		//p = strrchr(path,'/'); 
		//p = '\0';
		//printf("Returning with path = %s\n", path);
		return;
	} 
	p = (char*) calloc(256, sizeof(char));
	//printf("--scan(%s)--\n", path);
	while ((dirent = smbc_readdir(dir)) != NULL) {
		if (debug) printf("\nProcessing %s on %s\n", dirent -> name, path);
		if (!strcmp(dirent -> name, ".") ||
		    !strcmp(dirent -> name, "..")) {
			printf("%s\n", dirent -> name);
			continue;
		}
		if (dirent -> smbc_type == SMBC_DIR ||
		    dirent -> smbc_type == SMBC_FILE_SHARE) {
			//printf("\n%s:\n", path);
			//strcat(p, "/");
			//strcat(p, dirent -> name);
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
	//char* wg [16]; int wgs = 0; //yes, maximum number of workgroups is hardcoded
	int root; 
	//int i;
	char p[255];
	struct smbc_dirent * dirent;
	
	if ((argc > 1) && (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--debug"))) debug = 1;
	if (smbc_init(get_auth_data, 0)) printf("%s\n", strerror(errno));
	//scan("smb://SKY");
	if ((root = smbc_opendir("smb://SKY")) < 0) {
		if (debug) printf("Couldn't get list of workgroups (%s)\n", strerror(errno)); 
		return 0;
	} 
	//scan("smb://SKY");
	while ((dirent = smbc_readdir(root)) != NULL) {
		if (dirent -> smbc_type == SMBC_SERVER) {
			//wg[wgs] = (char*) malloc(256 * sizeof(char));
			sprintf(p, "smb://%s", dirent -> name);
			//strcpy(wg[wgs], p);
			//wgs++;
			scan(p);
		} else if (debug) printf("NOSCAN %s\n", dirent -> name);
	}
	/*for (i = 0; i < wgs; i++){
		printf("%s\n", wg[i]);
		scan(wg[i]);
	}*/
	//smbc_closedir(root);
	/*if (argc > 1) sprintf(open, "smb://%s", argv[1]);
	else strcpy(open, "smb://");
	if ((dir = smbc_opendir(open)) < 0) printf("%s\n", strerror(errno));
	while ((dirent = smbc_readdir(dir)) != NULL) { 
		printf("%s\n", dirent->name);
	}
	smbc_closedir(dir);
	*/
	return 0;
}
