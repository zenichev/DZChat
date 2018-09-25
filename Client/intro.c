/* DZChat - Client. Sep 2018 @Donat Zenichev */
/* This functionality is responsible for showing the introduction to a user */

/* HEADER FILE INCLUDES */
#include <stdio.h>			/* for printf() and fprintf() */

/* ------------------ NOTIFICATION TO USER (LOCALY GENERATED) ------------------------------ */
void introduction(void) {
	printf("\n>------------------------------------------------------------------------------>\n");
	printf("|      Now you can send messages to online participants                         |\n");
	printf("|      Use the following syntax(without angle brackets): <name>: <text-message> |\n");
	printf("|      If you want to quit the chat just type> quit;                            |\n");
	printf("|      If you want to clear a terminal just type> clear;                        |");
	printf("\n>------------------------------------------------------------------------------>\n");
	printf("\n");
}
/* ----------------------------------------------------------------------------------------- */
