/*
 * mac.c - MAC Address spoofer
 * (C) dayt0n (2016)
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/stat.h>
#include "ifconfig.c"

char* genaddr = "openssl rand -hex 6 | sed 's/\\(..\\)/\\1:/g; s/.$//'";

void startinternet(char* en) {
	char* params[] = { "ifconfig", en, "up", NULL };
	ifconfigmain(3,params);
}

void killinternet(char* en) {
	char *params[] = { "ifconfig", en, "down", NULL };
	ifconfigmain(3,params);
}

void restartConnection(char* en) {
	printf("Restarting %s...\n",en);
	killinternet(en);
	startinternet(en);
}

extern char **environ;
void run_cmd(char* cmd)
{
	pid_t pid;
	char *argv[] = {"sh", "-c", cmd, NULL};
	int status;
	status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ);
	if (status == 0) {
		//printf("Child pid: %i\n", pid);
		if (waitpid(pid, &status, 0) != -1) {
			//printf("Child exited with status %i\n", status);
		} else {
			perror("waitpid");
		}
	} else {
		printf("posix_spawn: %s\n", strerror(status));
	}
}

char* getMAC() {
	#define LENG 18
	static char linee[LENG];
	FILE *ye = popen(genaddr,"r");
	fgets(linee,sizeof(linee),ye);
	pclose(ye);
	return linee;
}

char* getethoMAC() {
	#define LENG 18
	static char linee[LENG];
	FILE *ye = popen(genaddr,"r");
	fgets(linee,sizeof(linee),ye);
	pclose(ye);
	return linee;
}

void setetho(char* neweno) {
	char* params[] = { "ifconfig", "en0", "ether", neweno, NULL };
	ifconfigmain(4,params);
}
void setwifi(char* newwifi) {
	char* params[] = { "ifconfig", "en1", "lladdr", newwifi, NULL };
	ifconfigmain(4,params);
}

void resetaddr(char* neweno, char* newwifi) {
	printf("Setting new ethernet MAC address...\n");
	setetho(neweno);
	printf("Setting new wifi MAC address...\n");
	setwifi(newwifi);
}

int main(int argc, char* argv[]) {
	pid_t pid, sid;
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	umask(0);
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}
	printf("Starting MAC changer daemon...\n");
	struct stat st = {0};
	uid_t uid=getuid(), euid=geteuid(); // we need root for certain ifconfig stuff
	if (uid == 0 || uid!=euid) {
	} else {
		printf("You are not root! MACSpoof needs root to function properly\n");
		exit(0);
	}
	if ( stat("/usr/local/bin/airport",&st) != 0 ) {
		printf("[WARNING]: /usr/local/bin/airport not found. Linking...\n"); // we gotta use this to disassociate the airport card
		if ( symlink("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport","/usr/local/bin/airport") != 0 ) {
			printf("For some reason I couldn't link /System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport to /usr/local/bin/airport...\n");
			exit(0);
		}
	}
	printf("Disassociating device from airport card...\n");
	printf("Telling system to delay changing of location until the next reboot...\n");
	close(STDIN_FILENO);
	close(STDERR_FILENO);
	close(STDOUT_FILENO);
	run_cmd("/usr/local/bin/airport -z"); // disassociate the airport card
	run_cmd("scselect -n"); // dont even look for a location unless I reboot
	run_cmd("killall \"System Preferences\""); // just a safety measure
	while(1) {
		char* addrwifi = getMAC();
		char* addretho = getethoMAC();
		if(strcmp(addretho,addrwifi) == 0) {
			sleep(1);
			char* addrwifi = getMAC();
		}
		resetaddr(addretho,addrwifi);
		restartConnection("en0");
		restartConnection("en1");
		sleep(3600); // repeat every hour
	}
	exit(EXIT_SUCCESS);
	return 0;
}