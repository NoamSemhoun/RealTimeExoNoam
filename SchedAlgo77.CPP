/********************************************************************/
/* A Small Real Time System for the Real-Time laboratory                 */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum                  */
/* Jerusalem College of Technology, 5759-64 (1999)                            */
/* SchedAlgo64.CPP, short-term scheduler algorithms                */
/****************************************************************/

#include "smarts77.h"

// différents algorithmes pour la planification à court terme


int roundRobin()
// Round Robin short-term algorithm
{
	int count;
	int nextTask = (SMARTS.getCurrentTask() + 1) % SMARTS.getTotalTasks();
	for (count = 0;
		 SMARTS.getStatus(nextTask) != READY && count < SMARTS.getTotalTasks();
		 count++)
		nextTask = ++nextTask % SMARTS.getTotalTasks();
	if (count == SMARTS.getTotalTasks()) // no found READY task to run
		nextTask = SMARTS.getTotalTasks();
	return nextTask;
}

/* sa c juste pr tester masi sa marche pas
int roundRobin()
{
    int nextTask = (SMARTS.getCurrentTask() + 1) % SMARTS.getTotalTasks();
    while (SMARTS.getStatus(nextTask) != READY) {
        nextTask = (nextTask + 1) % SMARTS.getTotalTasks();
        if (nextTask == SMARTS.getCurrentTask()) {
            return SMARTS.getTotalTasks();
        }
    }
    return nextTask;
}
*/

int earliesDeadlineFirst() {
	int lowest = MAXINT; // initialiser la variable lowest dans les algorithmes d'ordonnancement.  pour représenter l'infini,
	int nextTask = SMARTS.getTotalTasks();
	for (int i = 0; i < SMARTS.getTotalTasks(); i++) {
		if(SMARTS.getDeadline(i) < lowest && SMARTS.getStatus(i) == READY) {
			nextTask = i;
			lowest = SMARTS.getDeadline(i);
		}
	}
	return nextTask;
}

int rate_monotonic_scheduling(){
	int lowest = MAXINT;
	int nextTask = SMARTS.getTotalTasks();
	for (int i = 0; i < SMARTS.getTotalTasks(); i++) {
		if(SMARTS.getRoundTime(i) < lowest && SMARTS.getStatus(i) == READY) {
			nextTask = i;
			lowest = SMARTS.getRoundTime(i);
		}
	}
	return nextTask;
}