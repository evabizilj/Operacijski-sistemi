/*
Naloga2 mora izpisovati sistemske informacije s pomočjo datotečnega sistema /proc.

./Naloga2 akcija parametri
1. argument: akcija in predstavlja opravilo, ki naj se izvede
2. argument +: parametri akcije

Ob uspešnem izvajanju naj bo izhodni status enak 0, razen če akcija ne zahteva drugače.

Več akcij izpisuje različne informacije o sistemu oz. o procesih, zato je pri njih first parameter akcije 
pot do imenika (proc sistem), ki vsebuje informacije o jedru (privzeta vrednost parametra je /proc). 
V spodnjih primerih mu podamo pot do imenika proc-demo, katerega najdemo v arhivski datoteki proc-demo.tgz. 
Testni proc sistemi ne bodo vseboval vseh podatkov iz pravega proc sistema, le nekatere pomembnejše datoteke, 
ki jih potrebujete za izvedbo naloge.

Pri implementaciji lahko uporabljamo ovojne funkcije sistemskih klicev:	
fork, wait, exec, pause, kill, signal, sleep, getprocessID, getpprocessID, opendir, readdir, closedir, itd. 

Od "višje nivojskih" funkcij so dovoljene funkcije 
za delo z nizi: strlen, my_strcmp, strcasecmp, atoi
za delo s pomnilnikom: malloc, free, memcpy
za branje in pisanje (izpis): fopen, fclose, fscanf, scanf, printf, fprintf, sprintf 

Izvajanje ukazov, npr. ps, uname, iz okolja ni dovoljeno - vse informacije o sistemu preberemo iz sistema proc. 
Prav tako ni dovoljeno uporabiti funkcij, ki združujejo več operacij, npr. scandir.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h> 
#include <stdbool.h>
#include <ctype.h>

struct process
{
	int processID;
	char processName[1000];
	char state[1000];
	int PprocessID;
	int threads;
	int openDirectories;
} process;

char* my_strcat(char* destination, const char* source)
{
	char* ptr = destination + strlen(destination);

	while (*source != '\0')
		*ptr++ = *source++;

	*ptr = '\0';

	return destination;
}

int my_strcmp(const unsigned char* X, const unsigned char *Y)
{
	while (*X && tolower(*X) == tolower(*Y))
	{
		X++;
		Y++;
	}
	return tolower(*X) - tolower(*Y);
}

bool isNotHidden(struct dirent* entry)
{
    return entry->d_name[0] != '.';
}

bool isDir(struct dirent* entry)
{
    return isdigit(entry->d_name[0]);
}

int compareByName(struct process* a, struct process* b)
{
    return my_strcmp(a->processName, b->processName);
}

int compareprocessID(struct process* a, struct process* b)
{
    return (a->processID > b->processID) ? 1 : -1;
}

/*
Akcija sys
Izpiše osnovne informacije o sistemu:
		- različica Linux jedra
		- različica prevajalnika gcc za prevajanje jedra
		- prva swap particija
		- število jedrnih modulov

Izpis naj bo formatiran kot je razvidno iz primera.
Primer:

$ ./Naloga2 sys proc-demo
Linux: 3.10.0-327.36.2.el7.x86_64
gcc: 4.8.5
Swap: /dev/dm-1
Modules: 59
*/

void sys(char* argument[]) 
{
		char* buffer = (char *) calloc(10000, sizeof(char));
		char* in = (char *) calloc(10000, sizeof(char));
        char* in2 = (char *) calloc(10000, sizeof(char));
        char* in3 = (char *) calloc(10000, sizeof(char));
        FILE *fpProc_version = NULL;
        FILE *fpProc_swaps = NULL;
        FILE *fpProc_modules = NULL;

        // version
		my_strcat(in, argument[2]);
		my_strcat(in, "/version");
        fpProc_version = fopen(in, "r");

		fscanf(fpProc_version, "%*s %*s %s", buffer);
		printf("Linux: %s\n", buffer);
		fscanf(fpProc_version, "%*s %*s %*s %s", buffer);
		printf("gcc: %s\n", buffer);
		
		free(in);
		fclose(fpProc_version);
        
        // swaps
		my_strcat(in2, argument[2]);
		my_strcat(in2, "/swaps");

        fpProc_swaps = fopen(in2, "r");
		
		fgets(buffer, 10000, fpProc_swaps); 
		fscanf(fpProc_swaps, "%s", buffer);
		printf("Swap: %s\n", buffer);

		free(in2);
		fclose(fpProc_swaps); 

        // modules
		my_strcat(in3, argument[2]);
		my_strcat(in3, "/modules");
	
        fpProc_modules = fopen(in3, "r");

		int numLines = 0;
		char ch = getc(fpProc_modules);
		char newline = '\n';
		
		while (ch != EOF)
		{
			if (ch == newline) 
				numLines++;
			ch = getc(fpProc_modules);
		}
		printf("Modules: %d\n", numLines);
		
		free(in3);
		fclose(fpProc_modules);	
}

/*
Akcija processIDs
Naraščajoče urejeno izpiše številke vse procesov (processID), vsak processID naj bo v svoji vrstici. Primer izpisa:

$ ./Naloga2 pid proc-demo
1
10
11
12
14
15
16
20
*/

void pids(char* argument[]) 
{
	DIR* directoryPath;
	struct dirent *entry; 
	int* directories = (int*) calloc(10000, sizeof(int));
	
	int numDirectories = 0;

	directoryPath = opendir(argument[2]); 

	while((entry = readdir(directoryPath)))
		if (isDir(entry)) 
			directories[numDirectories++] = atoi(entry->d_name);	

	int sortDirectories[numDirectories];

	for (int i = 0; i < numDirectories; ++i)
		sortDirectories[i] = directories[i];

	// bubble sort
	for (int i = 0; i < numDirectories; ++i)
	{
		for (int j = 0; j < (numDirectories - i - 1); ++j)
		{
			if (sortDirectories[j] > sortDirectories[j + 1])
			{
				int tmp = sortDirectories[j];
				sortDirectories[j] = sortDirectories[j + 1];
				sortDirectories[j + 1] = tmp;
			}
		}
	}
	for (int i = 0; i < numDirectories; ++i)
		printf("%d\n", sortDirectories[i]); 
}

/*
Akcija names
Izpiše processIDe in imena vseh procesov nepadajoče urejeno po imenih: vsako ime v svoji vrstici, v primeru enakih imen se upošteva urejenost po processIDu. Primer izpisa:

$ ./Naloga2 names proc-demo
1 bash
10 bash
20 copyproc.sh
14 dash
15 dash
16 dash
11 sleep
12 sleep
*/

// dodaj velike črke
//int compareByName(struct process* a, struct process* b)
//{
//	return my_strcmp(a->processName, b->processName);
//}

//int compareprocessID(struct process* a, struct process* b)
//{
//	return (a->processID > b->processID) ? 1 : -1;
//}

void names(char* argument[])
{
    char* buffer = (char *) calloc(10000, sizeof(char));
    char* line = (char *) calloc(10000, sizeof(char));
    char* in = (char *) calloc(10000, sizeof(char));
    int* directories = (int *) calloc(10000, sizeof(int));
    FILE* inF = NULL;

	DIR* directoryPath;
	struct dirent *entry; 

	int numDirectories = 0;

	directoryPath = opendir(argument[2]); 

	while((entry = readdir(directoryPath))) 
		if (isDir(entry))
			directories[numDirectories++] = atoi(entry->d_name);	
	
	closedir(directoryPath);
	
	int sortDirectoriesNumber[numDirectories];

	directoryPath = opendir(argument[2]); 

	struct process* procesi = calloc(numDirectories, sizeof(struct process));

	int i = 0;
	while ((entry = readdir(directoryPath)))
	{ 
		if (isDir(entry))
		{
			memset(in, '\0', 1000);
			my_strcat(in, argument[2]);
			my_strcat(in, "/");
			my_strcat(in, buffer);
			my_strcat(in, entry->d_name);
			my_strcat(in, "/status");

	        struct process* current = &procesi[i++]; 

			inF = fopen(in, "r");

			while (fscanf(inF, "%s", line) == 1)
				if (!my_strcmp(line, "Name:"))
					fscanf(inF, "%s", current->processName);
				else if (!my_strcmp(line, "Pid:"))
					fscanf(inF, "%d", &current->processID);
	        
            fclose(inF);
		}
	}

	closedir(directoryPath);

	for (int i = 0; i < numDirectories; ++i)
	{
		for (int j = i + 1; j < numDirectories; ++j)
		{
			struct process a = procesi[i];
			struct process b = procesi[j];
			if (compareByName(&a, &b) == 0)
			{
				if (compareprocessID(&a, &b) == 1)
				{
					procesi[i] = b;
					procesi[j] = a;
				}
			}
			if (compareByName(&a, &b) > 0)
			{
				procesi[i] = b;
				procesi[j] = a;
			}
		}
	}

	for (int i = 0; i < numDirectories; ++i)
		printf("%d %s\n", 
            procesi[i].processID, 
            procesi[i].processName);

	free(buffer);
	free(line);
	free(in);
    free(procesi);
}		 

/*
Akcija ps
Izpiše naslednje informacije o procesih: processID, PprocessID, stanje in ime procesa. Izpis naj bo urejen po processIDih. Primer izpisa: 

$ ./Naloga2 ps proc-demo
	processID	PprocessID STANJE IME
		1		 0			S bash
	 10		 1			S bash
	 11		10			S sleep
	 12		10			S sleep
	 14		 1			S dash
	 15		14			T dash
	 16		14			T dash
	 20		14			S copyproc.sh
Za formatiranje izpisa lahko uporabite naslednji format:

printf("%5s %5s %6s %s\n", "processID", "PprocessID", "STANJE", "IME");
Če je podan še dodatni argument - število, le-ta predstavlja processID. V tem primeru izpišite zgolj procese, katerih prednik (ne nujno neposredni in vključno z danim procesom) je proces s podanim processID. Primer izpisa:

$ ./Naloga2 ps proc-demo 10
	processID	PprocessID STANJE IME
	 10		 1			S bash
	 11		10			S sleep
	 12		10			S sleep
*/

void ps(int numArguments, char* argument[])
{
	char* buffer = (char *) calloc(10000, sizeof(char));
    char* line = (char *) calloc(10000, sizeof(char));
    char* in = (char *) calloc(10000, sizeof(char));
    int* directories = (int *) calloc(10000, sizeof(int));
    FILE* inF = NULL;
    DIR* directoryPath = NULL;

    struct dirent *entry; 

	int numDirectories = 0;

	directoryPath = opendir(argument[2]); 

	while((entry = readdir(directoryPath)))
		if (isDir(entry)) 
			directories[numDirectories++] = atoi(entry->d_name);	

	int sortDirectoriesNumber[numDirectories];

	directoryPath = opendir(argument[2]); 

	struct process* procesi = calloc(numDirectories, sizeof(struct process));

	int i = 0;
	while ((entry = readdir(directoryPath)))
	{ 
		if (isDir(entry))
		{
			memset(in, '\0', 10000);
			my_strcat(in, argument[2]);
			my_strcat(in, "/");
			my_strcat(in, buffer);
			my_strcat(in, entry->d_name);
			my_strcat(in, "/status");

	        struct process* current = &procesi[i++]; 

		    inF = fopen(in, "r");

			while (fscanf(inF, "%s", line) == 1)
				if (!my_strcmp(line, "Pid:"))
					fscanf(inF, "%d", &current->processID);
				else if (!my_strcmp(line, "Name:"))
					fscanf(inF, "%s", current->processName);
				else if (!my_strcmp(line, "State:"))
					fscanf(inF, "%s", current->state);
				else if (!my_strcmp(line, "PPid:")) 
					fscanf(inF, "%d", &current->PprocessID);

			fclose(inF);
		}
	}
	closedir(directoryPath);

	if (numArguments == 3)
	{
		for (int i = 0; i < numDirectories; ++i)
		{
			for (int j = i + 1; j < numDirectories; ++j)
			{
				struct process a = procesi[i];
				struct process b = procesi[j];
				if (compareprocessID(&a, &b) == 1)
				{
					procesi[i] = b;
					procesi[j] = a;
				}
			}
		}

		printf("%5s %5s %6s %s\n", 
                "PID", 
                "PPID", 
                "STANJE", 
                "IME");

		for (int i = 0; i < numDirectories; ++i)
		{
			printf("%5d %5d %6s %s\n",
                procesi[i].processID, 
                procesi[i].PprocessID, 
                procesi[i].state, 
                procesi[i].processName);
		}
	}

	else if (numArguments == 4)
	{
		// PREDNIKI
        struct process* arrayOfProcesses = malloc(numDirectories *
                                            sizeof(struct process));
		int pid = atoi(argument[3]);
		int length = 0;
		
		for (int i = 0; i < numDirectories; ++i)
		{
		    struct process* current = &procesi[i];
			if (current->processID == pid) 
				arrayOfProcesses[length++] = *current;
		}

        int a = 0;
        int b = length;

        for (;;)
        {
		    for (int i = 0; i < numDirectories; ++i)
		    {
		        struct process* current = &procesi[i];
                for (int j = a; j < b; ++j)
                {
                    int pid = arrayOfProcesses[j].processID;
		    	    if (current->PprocessID == pid) 
		    	    	arrayOfProcesses[length++] = *current;
                }
		    }
            int inserted = length - b;
            if (inserted == 0)
                break;
            a = b;
            b = length;
        }

		for (int i = 0; i < length; ++i)
		{
			for (int j = i + 1; j < length; ++j)
			{
				struct process a = arrayOfProcesses[i];
				struct process b = arrayOfProcesses[j];
				if (compareprocessID(&a, &b) == 1)
				{
					arrayOfProcesses[i] = b;
					arrayOfProcesses[j] = a;
				}
			}
		}

		printf("%5s %5s %6s %s\n", 
                    "PID", 
                    "PPID", 
                    "STANJE", 
                    "IME");
		
        for (int i = 0; i < length; ++i)
		{
			struct process current = arrayOfProcesses[i];
			printf("%5d %5d %6s %s\n", 
                current.processID, 
                current.PprocessID, 
                current.state, 
                current.processName);
		}
	}
}

/*
Akcija psext
Ta akcija je nadgradnja akcije ps. Dodatno izpišite še število niti in število odprtih datotek procesa. Za formatiranje izpisa uporabite naslednji format:

	printf("%5s %5s %6s %6s %6s %s\n", "processID", "PprocessID", "STANJE", "#NITI", "#DAT.", "IME");
Prav tako naj akcija podpira dodatni argument - processID prednika. Primer izpisa:

$ ./Naloga2 psext proc-demo 14
	processID	PprocessID STANJE	#NITI	#DAT. IME
	 14		 1			S			1			4 dash
	 15		14			T			1			4 dash
	 16		14			T			1			4 dash
	 20		14			S			1			4 copyproc.sh

*/

void psext(int numArguments, char* argument[])
{
	struct dirent *entry;
    struct dirent *entry2;

	int* directories = (int*) calloc(10000, sizeof(int));
    char* buffer = (char *) calloc(10000, sizeof(char));
    char* line = (char *) calloc(10000, sizeof(char));
    char* in = (char *) calloc(10000, sizeof(char));
    char* in2 = (char *) calloc(10000, sizeof(char));
    FILE* inF = NULL;
    DIR* directoryPath = NULL;

	int numDirectories = 0;
    int i = 0;

	directoryPath = opendir(argument[2]); 

	while((entry = readdir(directoryPath)))
		if (isDir(entry)) 
			directories[numDirectories++] = atoi(entry->d_name);	

	int sortDirectoriesNumber[numDirectories];

	directoryPath = opendir(argument[2]); 

	struct process* procesi = calloc(numDirectories, sizeof(struct process));

    int count = 0;
	while ((entry = readdir(directoryPath)))
	{ 
		if (isDir(entry))
		{
			memset(in, '\0', 10000);
			my_strcat(in, argument[2]);
			my_strcat(in, "/");
			my_strcat(in, buffer);
			my_strcat(in, entry->d_name);
			my_strcat(in, "/status");

	        struct process* current = &procesi[i++]; 

			inF = fopen(in, "r"); 
			while (fscanf(inF, "%s", line) == 1)
			{
				if (!my_strcmp(line, "Pid:"))
					fscanf(inF, "%d", &current->processID);
				else if (!my_strcmp(line, "Name:"))
					fscanf(inF, "%s", current->processName);
				else if (!my_strcmp(line, "State:"))
					fscanf(inF, "%s", current->state);
				else if (!my_strcmp(line, "PPid:")) 
					fscanf(inF, "%d", &current->PprocessID);
				else if(!my_strcmp(line, "Threads:"))
					fscanf(inF, "%d", &current->threads);

				// lokacija: fd
				memset(in2, '\0', 1000);
				my_strcat(in2, argument[2]);
				my_strcat(in2, "/");
				my_strcat(in2, buffer);
				my_strcat(in2, entry->d_name);
				my_strcat(in2, "/fd");

			    count = 0;

				DIR* directoryPath2 = opendir(in2);

				while ((entry2 = readdir(directoryPath2)))
					if (isNotHidden(entry2)) 
						count++;

				current->openDirectories = count;
				closedir(directoryPath2);
			}
			fclose(inF);
		}
	}
	closedir(directoryPath);

	if (numArguments == 3)
	{
		for (int i = 0; i < numDirectories; ++i)
		{
			for (int j = i + 1; j < numDirectories; ++j)
			{
				struct process a = procesi[i];
				struct process b = procesi[j];
				if (compareprocessID(&a, &b) == 1)
				{
					procesi[i] = b;
					procesi[j] = a;
				}
			}
		}
		printf("%5s %5s %6s %6s %6s %s\n", 
                "PID", 
                "PPID", 
                "STANJE", 
                "#NITI", 
                "#DAT.", 
                "IME");
		
		for (int i = 0; i < numDirectories; ++i)
		{
			printf("%5d %5d %6s %6d %6d %s\n",
                procesi[i].processID,
                procesi[i].PprocessID,
                procesi[i].state,
                procesi[i].threads,
                procesi[i].openDirectories,
                procesi[i].processName);
		}
	}
	else if (numArguments == 4)
	{
		// PREDNIKI
		int pid = atoi(argument[3]);
		int length = 0;
		int tmp = 0;

		struct process* arrayOfProcesses = malloc(numDirectories * sizeof(struct process));
		
		for (int i = 0; i < numDirectories; ++i)
		{
		    struct process* current = &procesi[i];
			if (current->processID == pid) 
				arrayOfProcesses[length++] = *current;
		}

        int a = 0;
        int b = length;

        for (;;)
        {
		    for (int i = 0; i < numDirectories; ++i)
		    {
		        struct process* current = &procesi[i];
                for (int j = a; j < b; ++j)
                {
                    int pid = arrayOfProcesses[j].processID;
		    	    if (current->PprocessID == pid) 
		    	    	arrayOfProcesses[length++] = *current;
                }
		    }
            int inserted = length - b;
            if (inserted == 0)
                break;
            a = b;
            b = length;
        }

		for (int i = 0; i < length; ++i)
		{
			for (int j = i + 1; j < length; ++j)
			{
				struct process a = arrayOfProcesses[i];
				struct process b = arrayOfProcesses[j];
				if (compareprocessID(&a, &b) == 1)
				{
					arrayOfProcesses[i] = b;
					arrayOfProcesses[j] = a;
				}
			}
		}

		printf("%5s %5s %6s %6s %6s %s\n", 
                    "PID", 
                    "PPID", 
                    "STANJE", 
                    "#NITI", 
                    "#DAT.", 
                    "IME");

		for (int i = 0; i < length; ++i)
		{
			printf("%5d %5d %6s %6d %6d %s\n",
                arrayOfProcesses[i].processID,
                arrayOfProcesses[i].PprocessID,
                arrayOfProcesses[i].state,
                arrayOfProcesses[i].threads,
			    arrayOfProcesses[i].openDirectories,
                arrayOfProcesses[i].processName);
		}
	}

    free(directories);
    free(buffer);
    free(line);
    free(in);
    free(in2);
}


int main(int numArguments, char* argument[])
{
	if (!my_strcmp(argument[1], "sys")) 
		sys(argument);
	else if (!my_strcmp(argument[1], "pids")) 
		pids(argument);
	else if (!my_strcmp(argument[1], "names")) 
		names(argument);
	else if (!my_strcmp(argument[1], "ps"))
		ps(numArguments, argument);
	else if (!my_strcmp(argument[1], "psext"))
		psext(numArguments, argument);
	return 0;
}