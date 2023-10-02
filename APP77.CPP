/********************************************************************/
/* A Small Real Time System for the Real-Time laboratory            */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum              */
/* Jerusalem College of Technology, 5759-64 (1999-03)               */
/* update  Tishrey   5777                                           */
/* APP77.CPP, an application to demonstrate SMARTS77  functioning   */
/********************************************************************/

//l'ancien simple

// WITHOUT 0 

#include "smarts77.h"

Mutex ioMutex;		
// pb parametre a envoyer 

 
Event evCtoA, evEtoA;
Event evFtoB, evDtoB;  // SENd WAIT 

void writeToLog(char *str)
{
	FILE *f = fopen("log.txt", "a"); // a  == append  RAJOUTE // CHANGE LE NOM DE FICHIER   EN FCT DE LA ARATSA
	fprintf(f, str);
	fclose(f);
}

// user Functions
void a()
{
	int j;
	ioMutex.Acquire();   // Prend le mutex 
	cout << "\n *************   A Start    *********************\n";
	writeToLog("\n *************   A Start    *********************\n");
	ioMutex.Release();			// libere le mutex
	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "A";
		writeToLog("A");
		ioMutex.Release();
	}

	writeToLog("---- - > I am A and I acquire aMutex\n");
	SMARTS.mutexes[0].Acquire();    ///   AQUERE UN NOIVEAU MUTEX   A    MECHOUTAF

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "A";
		writeToLog("A");
		ioMutex.Release();
	}
	char d;
	writeToLog("---- - > I am A and I send to C \n");
	evCtoA.send('C', &d, 0);
	writeToLog("---- - > I am A and I send to E \n");
	evEtoA.send('E', &d, 0);
	writeToLog("---- - > I am A and I continue running after events");

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "A";
		writeToLog("A");
		ioMutex.Release();
	}

	writeToLog("---- - > I am A and I release aMutex\n");
	SMARTS.mutexes[0].Release();   ////   RELACHE UN NOIVEAU MUTEX   A    MECHOUTAF

	writeToLog("Amutex was released.\n");
	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "A";
		writeToLog("A");
		ioMutex.Release();
	}

	ioMutex.Acquire();
	cout << "\n *************   A Finish   *********************\n";
	writeToLog("\n *************   A finish    *********************\n");
	ioMutex.Release();
}

void e()
{
	int j;
	ioMutex.Acquire();
	cout << "\n *************   E Start    *********************\n";
	writeToLog("\n *************   E Start    *********************\n");
	ioMutex.Release();
	for (j = 0; j < 5; j++) // Waste 600000 operations time
	{
		for (long i = 0; i < 200000; i++)
			;
		ioMutex.Acquire();
		cout << "E";
		writeToLog("E");
		ioMutex.Release();
	}

	char r;
	writeToLog("---- - > I am E and I WAIT to A\n");
	evEtoA.wait(r);
	writeToLog("---- - > I am E and A let me go\n");
	for (j = 0; j < 5; j++) // Waste 600000 operations time
	{
		for (long i = 0; i < 200000; i++)
			;
		ioMutex.Acquire();
		cout << "E";
		writeToLog("E");
		ioMutex.Release();
	}

	ioMutex.Acquire();
	cout << "\n *************   E Finish   *********************\n";
	writeToLog("\n *************   E Finish    *********************\n");
	ioMutex.Release();
}

void c()
{
	int j;
	ioMutex.Acquire();
	cout << "\n *************   C Start    *********************\n";
	writeToLog("\n *************   C Start    *********************\n");
	ioMutex.Release();
	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 600000 operations time
		ioMutex.Acquire();
		cout << "C";
		writeToLog("C");
		ioMutex.Release();
	}
	char r;
	writeToLog("---- - > I am C and I WAIT to A\n");
	evCtoA.wait(r);
	writeToLog("---- - > I am C and A let me go\n");

	writeToLog("---- - > I am C and and I wait to aMutex\n");
	SMARTS.mutexes[0].Acquire();   
	writeToLog("---- - > I am C and I continue, acquired aMutex\n");

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 600000 operations time
		ioMutex.Acquire();
		cout << "C";
		writeToLog("C");
		ioMutex.Release();
	}

	writeToLog("---- - > C release Amutex");
	SMARTS.mutexes[0].Release();
	writeToLog("---- - > I'm C and I release aMutex!\n");
	ioMutex.Acquire();
	cout << "\n *************   C Finish   *********************\n";
	writeToLog("\n *************   C Finish    *********************\n");
	ioMutex.Release();
}

void b()
{
	int j;
	ioMutex.Acquire();
	cout << "\n *************   B Start    *********************\n";
	writeToLog("\n *************   B Start    *********************\n");
	ioMutex.Release();
	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "B";
		writeToLog("B");
		ioMutex.Release();
	}

	writeToLog("---- - > I am B and I acquire bMutex\n");
	SMARTS.mutexes[1].Acquire();

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "B";
		writeToLog("B");
		ioMutex.Release();
	}
	char d;
	writeToLog("---- - > I am B and I send to D \n");
	evDtoB.send('D', &d, 0);   // D IL ATTENDAIT     D  PRIORITE  ! RUN 
	writeToLog("---- - > I am B and I send to F \n");
	evFtoB.send('F', &d, 0);
	writeToLog("---- - > I am B and I continue running after events\n");

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "B";
		writeToLog("B");
		ioMutex.Release();
	}

	writeToLog("---- - > I am B and I release bMutex\n");
	SMARTS.mutexes[1].Release();
	writeToLog("---- - > I am B and bMutex was released\n");

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 200000 operations time
		ioMutex.Acquire();
		cout << "B";
		writeToLog("B");
		ioMutex.Release();
	}

	ioMutex.Acquire();
	cout << "\n *************   B Finish   *********************\n";
	writeToLog("\n *************   B finish    *********************\n");
	ioMutex.Release();
}

void f()
{
	int j;
	ioMutex.Acquire();
	cout << "\n *************   F Start    *********************\n";
	writeToLog("\n *************   F Start    *********************\n");
	ioMutex.Release();
	for (j = 0; j < 5; j++) // Waste 600000 operations time
	{
		for (long i = 0; i < 200000; i++)
			;
		ioMutex.Acquire();
		cout << "F";
		writeToLog("F");
		ioMutex.Release();
	}

	char r;
	writeToLog("---- - > I am F and I WAIT to B\n");
	evFtoB.wait(r);
	writeToLog("---- - > I am F and B let me go\n");
	for (j = 0; j < 5; j++) // Waste 600000 operations time
	{
		for (long i = 0; i < 200000; i++)
			;
		ioMutex.Acquire();
		cout << "F";
		writeToLog("F");
		ioMutex.Release();
	}

	ioMutex.Acquire();
	cout << "\n *************   F Finish   *********************\n";
	writeToLog("\n *************   F Finish    *********************\n");
	ioMutex.Release();
}

void d()
{
	int j;
	ioMutex.Acquire();
	cout << "\n *************   D Start    *********************\n";
	writeToLog("\n *************   D Start    *********************\n");
	ioMutex.Release();
	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 600000 operations time
		ioMutex.Acquire();
		cout << "D";
		writeToLog("D");
		ioMutex.Release();
	}
	char r;
	writeToLog("---- - > I am D and I WAIT to B\n");
	evDtoB.wait(r);    // JE MARRETE ET JATTEND B 
	writeToLog("---- - > I am D and B let me go\n");    // IL MA RELANCE

	writeToLog("---- - > I am D and and I wait to bMutex\n");	
	SMARTS.mutexes[1].Acquire();		// IL PREND LE MUTEX B (numero 1) 
	writeToLog("---- - > I am D and B let me go\n");

	for (j = 0; j < 5; j++)
	{
		for (long i = 0; i < 200000; i++)
			; // Waste 600000 operations time
		ioMutex.Acquire();
		cout << "D";
		writeToLog("D");
		ioMutex.Release();
	}

	SMARTS.mutexes[1].Release();    // RELAHCE LE MUTEX 
	writeToLog("---- - > I'm D and I release bMutex!\n");
	ioMutex.Acquire();
	cout << "\n *************   D Finish   *********************\n";
	writeToLog("\n *************   D Finish    *********************\n");
	ioMutex.Release();
}

void main()
{
	clrscr();
	writeToLog("Start\n");

	for (int i = 0; i < 2; i++)
		SMARTS.declareMutex(0);     // inheritence a 1


	SMARTS.externalFunctions(timerInterruptHandler, scheduler, myTaskEnd, rate_monotonic_scheduling);   //   CHOIX DE LALGO 
	SMARTS.declareTask(a, 'A', 412, 1); // L   nom a declare et le nombre de tour 
	SMARTS.declareTask(e, 'E', 407, 1); // M
	SMARTS.declareTask(c, 'C', 402, 1); // H

	SMARTS.declareTask(b, 'B', 410, 1); // L
	SMARTS.declareTask(f, 'F', 405, 1); // M
	SMARTS.declareTask(d, 'D', 400, 1); // H

	SMARTS.runTheTasks();
	writeToLog("End\n");
}


// PR ECRIRE DS LE KOBETS 