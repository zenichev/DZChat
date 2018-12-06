/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* This functionality is responsible for showing the introduction to a user */

/* HEADER FILE INCLUDES */
#include <stdio.h>			/* for printf() and fprintf() */

/* ------------------ NOTIFICATION TO USER (LOCALY GENERATED) ------------------------------ */
void introduction(void) {
	printf("\n---------------------------------------------------------------------------------\n");
	printf("|     Use the following syntax(without angle brackets): <name>: <text-message>  |\n");
	printf("|     A group message(without angle brackets): group <name> say: <text-message> |\n");
	printf("|     To get a help type> help;                                                 |\n");
	printf("---------------------------------------------------------------------------------\n");
	printf("\n");
}
/* ----------------------------------------------------------------------------------------- */

/* --------------------- HELP INTRO TO USER (LOCALY GENERATED) ------------------------------ */
void help(void) {
	printf("\n---------------------------------------------------------------------------------\n");
	printf("|      quit the chat,      type> quit;                                          |\n");
	printf("|      clear a terminal,   type> clear;                                         |\n");
	printf("|      check online users, type> online;                                        |\n");
	printf("|      add a group,        type> group add groupname;                           |\n");
	printf("|      delete a group,     type> group del groupname;                           |\n");
	printf("|      join a group,       type> group join groupname;                          |\n");
	printf("|      quit a group,       type> group quit groupname;                          |\n");
	printf("|      show a list of groups,        type> group show;                          |\n");
	printf("|      show a list of group members, type> group show members groupname;        |\n");
	printf("---------------------------------------------------------------------------------\n");
	printf("\n");
}
/* ----------------------------------------------------------------------------------------- */

