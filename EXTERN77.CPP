/*****************************************************************/
/* A Small Real Time System for the Real-Time laboratory         */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum           */
/* Jerusalem College of Technology, 5759-64 (1999)               */
/* update Tishrey  5777                                          */
/* EXTERN77.CPP, holds all external functions                    */
/*****************************************************************/
#include "smarts77.h"


// fonctions externes et globales utilisées dans le système SMARTS77 pour la gestion des interruptions et des tâches.

// gestion du temps réel, de l'ordonnancement des tâches et de la commutation entre les tâches pour un bon fonctionnement du système

/**********    GLOBAL Variables    **********/
Parallelism SMARTS;
static unsigned StackSegAct, StackPtrAct;

/**********    EXTERNAL FUNCTION     **********/
void far interrupt timerInterruptHandler(...)
// This function is called 18.2 times per second ( every ~~ 55 millisec).


//Elle gère les interruptions du timer. La fonction sauvegarde l'état de la tâche en cours d'exécution si nécessaire,
//gère les interruptions et les appels de fonctions planifiées. 
//Elle est responsable de la gestion des tâches en fonction de leurs échéances, délais d'exécution et priorités.
//
{
	asm	mov	StackPtrAct,sp; 	// get the stack addr of the task interrupted
	asm	mov	ax,ss;		// by the clock or by a Program interrupt
	asm	mov	StackSegAct,ax;

	SMARTS.timerClocksEnd = getTimerClocks( );

			// if it was called by the timer interrupt (hardware source),
			// it calls the original BIOS routine. not to change the PC work.

	if (!SMARTS.getProgInt( ))   // SI PSIKAT HOMRA 
	{
		asm	int userInt;
		SMARTS.handleTimers();
	}

			// if the 'context switch' flag=true, save the current task variables
			// and call the 'scheduler'
	if (SMARTS.getProgInt() || SMARTS.getContextSwitch())   // save stack addr of interrupted task in current stack

	//


	{
		SMARTS.resetProgInt();
		SMARTS.setCurrentStack(StackSegAct,StackPtrAct);
		SMARTS.restoreSchedStack( );	      // prepare activation of the task "scheduler"
		SMARTS.getSchedStack(StackSegAct,StackPtrAct);
		asm	mov	ax,StackSegAct;
		asm	mov	ss,ax
		asm	mov	sp,StackPtrAct
	}
			// else if 'context switch' flag=false set 'end of time slice' flag
//	else 
//	{
//		SMARTS.setEndOfTimeSlice();
//		++SMARTS.TScount;	// increment Time Share count
//	}
}
/**********    EXTERNAL FUNCTION     **********/
void scheduler( )	// this also a parallel task
		// This function is called by hardware called 'timerInterruptHandler' function
		// or by software SMARTS system's functions
		// It selects & switches the next task to be run.


	//joue le rôle du planificateur à court terme. Elle sélectionne la prochaine tâche à exécuter en fonction de l'algorithme d'ordonnancement choisi.
	//Le choix de l'algorithme d'ordonnancement peut être déterminé dans d'autres parties du code 
	//(comme le fichier SCHEDALGO64.CPP que vous avez mentionné précédemment).

	//Si une tâche est sélectionnée, la fonction restaure l'état de la tâche et la reprend.
{
	int nextTask, i;

		// calling to the 'short term scheduler' algorithm
	nextTask = SMARTS.algorithm( );

	if (SMARTS.getError())
		nextTask = SMARTS.getTotalTasks();

		// checking for 'deadlock' : not found task to run and tasks remain suspended
	if (nextTask == SMARTS.getTotalTasks() && 
		SMARTS.sleepTasks==0 && SMARTS.activeTasks>0)
	{
		for (i=SMARTS.getTotalTasks()-1; i >= 0 ; --i)
			if (SMARTS.getStatus(i) == SUSPENDED) 
			{
				cprintf("\ntask %c  is suspended",SMARTS.getName(i)) ;
				SMARTS.setDeadlock();
			}
	}
		// ok to run a next task
	SMARTS.setCurrentTask(nextTask);

		//  'dispatcher' to restore the next task stack addr and registers
	SMARTS.getCurrentStack(StackSegAct,StackPtrAct);
	asm	mov	ax,StackSegAct;
	asm	mov	ss,ax
	asm	mov	sp,StackPtrAct

	asm	pop	bp;
	asm	pop	di;
	asm	pop	si;
	asm	pop	ds;
	asm	pop	es;
	asm	pop	dx;
	asm	pop	cx;
	asm	pop	bx;
	asm	pop	ax;
	asm	iret;	// return from interrupt 
}

/**********    EXTERNAL FUNCTION     **********/
void myTaskEnd( )
//appelée lorsque la tâche courante a terminé son exécution. 
//Elle indique à SMARTS qu'une tâche est terminée et qu'il est prêt à sélectionner une nouvelle tâche à exécuter.

{
	SMARTS.taskEnd();	
}