#include <libsmbclient.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

static void
get_auth_data (const char * srv, const char * shr, char * wg, int wglen,
	       char * un, int unlen, char * pw, int pwlen)
{
	/*smbc_set_credentials ("SKY",
			      "SKYSEARCH",
			      "",
			      0,
			      "off");
	*/
	return;
}

int main (int argc, char* argv[]) {
	int dir;
	char open[255];
	struct smbc_dirent * dirent;
	if (smbc_init(get_auth_data, 0)) printf("%s\n", strerror(errno));
	//else printf("smbc_init: OK\n");
	if (argc > 1) sprintf(open, "smb://%s", argv[1]);
	else strcpy(open, "smb://");
	if ((dir = smbc_opendir(open)) < 0) printf("%s\n", strerror(errno));
	//else printf("smbc_opendir(%s): OK\n", open);
	while ((dirent = smbc_readdir(dir)) != NULL) { 
		printf("%s\n", dirent->name);
		/* printf("%*.*s%-30s", 4, 4, "", dirent->name);
		switch (dirent->smbc_type) {
			case SMBC_WORKGROUP:
				printf("WG"); break;
			case SMBC_SERVER:
				printf("SERV"); break;
			case SMBC_FILE_SHARE:
				printf("FS"); break;
			case SMBC_PRINTER_SHARE:
				printf("PRINT"); break;
			case SMBC_COMMS_SHARE:
				printf("COMMS"); break;
			case SMBC_IPC_SHARE:
				printf("IPC"); break;
			case SMBC_DIR:
				printf("DIR"); break;
			case SMBC_FILE:
				printf("FILE"); break;
			case SMBC_LINK:
				printf("LINK"); break;
		}
		printf("\n");
		*/
	}
	smbc_closedir(dir);
	return 0;
}
