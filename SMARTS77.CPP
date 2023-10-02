/****************************************************************/
/* A Small Real Time System for the Real-Time laboratory        */
/* built by: A.Teitelbaum on an idea of H.G.Mendelbaum          */
/* Jerusalem College of Technology, 5759-64 (1999)              */
/* update  Tishrey   5777                                       */
/* SMARTS77.CPP, SMARTS class body                              */
/****************************************************************/
#include "smarts77.h"

//  système de tâches en temps réel avec des fonctionnalités de parallélisme, de planification et de synchronisation à l'aide de mutex. 
//  Il permet l'exécution de tâches concurrentes et le partage de ressources tout en garantissant le respect des contraintes temporelles.

//  La classe Parallelism : Cette classe gère la parallélisme et l'ordonnancement des tâches. Elle contient les fonctions pour déclarer et exécuter les tâches.
//  
//  La classe Task :  une tâche individuelle. contient les fonctions pour déclarer une tâche et gérer son état, notamment  les priorités et les délais.
//  
//  La classe Mutex :  un verrou(mutex) qui peut être utilisé pour synchroniser l'accès concurrent à des ressources partagées entre les tâches.
//  
//  La classe Queue : utilisée pour implémenter une file d'attente FIFO, utilisée pour gérer les tâches en attente d'un mutex.

/**********    Function     **********/
unsigned getTimerClocks()
// Gets the remaining clocks of the timer register
{
	unsigned clocks;
	/* latch counter #0 */
	outportb(0x43, 0x00);
	/* read counter #0 low byte */
	clocks = inportb(0x40);
	/* read counter #0 high byte */
	clocks += inportb(0x40) << 8;
	return clocks;
}
////////////////////////////////////////////////////
/**********    class body:  Parallelism     **********/
Parallelism::Parallelism()
{
	currentTask = 0;
	sleepTasks = 0;
	activeTasks = 0;
	totalTasks = 0;
	suspendedTasks = 0;
	deadlock = false;
	errorFlag = false;
	endOfTimeSlice = true;
}

void Parallelism::externalFunctions(void interrupt (*timerInterruptHandler)(...),
									void far *scheduler, void far *userTaskEnd,
									int far (*algorithm)())
// Sets the external functions

// DEFINIR LES FONTION EXTERN PR SMART     ELLE PREND DES POINTERIM VERS LES MENALIM PSIKOT   ET LE  TASK
{
	this->timerInterruptHandler = timerInterruptHandler;
	this->scheduler = scheduler;
	this->userTaskEnd = userTaskEnd;
	this->algorithm = algorithm;
	contextSched.declare(scheduler, userTaskEnd, 'S', 1, 1); // prepare the stack of the scheduler task, this is weird, check it
	for (int i = MaxStack - 1; i >= (MaxStack - 14); i--)
		schedCopy[i] = contextSched.stack[i];
}

int Parallelism::declareTask(void far *code, char name, int roundTime, int roundCircle)
// Insert a new task entry in SMARTS context array [ ]
{

	// DECLARER UNE NOUVELLE TACHE AVEC UN NOM    TMP DEXECUTION ET    NOMBRE DE TOURS  
	if (totalTasks < MaxTask - 1)
	{
		context[totalTasks++].declare(code, userTaskEnd, name, roundTime, roundCircle);
		++activeTasks;  //  Elle ajoute la tâche à la liste des tâches actives.
		return true;
	}
	else
		return false;
}

int Parallelism::declareMutex(int inheritance)
{
	if (currentMutex == MaxMutex)
		return false;
	if (inheritance)
	{
		mutexes[currentMutex].enableInheritence();  // Il appel la priority inheritence
	}
	currentMutex++;
	return true;
}

void Parallelism::runTheTasks()
// Start running all tasks declared in SMARTS.

// démarrer l'exécution des tâches déclarées.
//configure les interruptions et gère le basculement entre les tâches.
{
	context[totalTasks].status = READY; // for task 'runTheTasks' (IDLE task)
	context[totalTasks].priority = MAXINT;
	context[totalTasks].currentPriority = MAXINT;

	currentTask = totalTasks;

	asm cli;							 // forbids interrupts (clear interrupts) while changing the interrupt vect
										 // saves the original BIOS userInt in our variable 'userIntAddress' to be restored at the end
	userIntAddress = getvect(userInt);	 // BIOS userInt 0x60  (unused by PC)
										 // puts the normal BIOS timerInt into the unused userInt address
	setvect(userInt, getvect(timerInt)); // timerInt 0x08

	// sets our SMARTS external function 'timerInterruptHandler' as the new PC hard interrupt time handler
	setvect(timerInt, timerInterruptHandler);
	asm sti; // allows back interrupts (set interrupts)

	// waits for end of runTheTasks (end of all the tasks)
	while (true)
	{
		if (deadlock)
		{
			textcolor(RED);
			cprintf("\n\n\rExit : deadlock");
			break;
		}
		if (errorFlag)
		{
			textcolor(RED);
			cprintf("\n\n\rExit : Error, function did not finish in time.");
			break;
		}
		if (allTasksDone())
		{
			cprintf("\n\n\rExit : finish");
			break;
		}
	}

	// restore the original BIOS 'interrupt vector' at the end before returning to regular DOS
	asm cli;							 // no interrupts
	setvect(timerInt, getvect(userInt)); // restore original BIOS time handler
	setvect(userInt, userIntAddress);	 // restore original BIOS userInt
	asm sti;							 // yes interrupts
}

void Parallelism::callScheduler()
// Return the control to the scheduler, this sets ON the software interrupt ProgInt flag
{
	setProgInt();
	asm int timerInt;
}

int Parallelism::allTasksDone()
{
	for (int i = 0; i < totalTasks; ++i)
		if (context[i].status != NOT_ACTIVE || context[i].remainingRoundCircle > 1)
			return false;
	return true;
}

void Parallelism::restoreSchedStack()
// Restore the scheduler stack
{
	for (int i = MaxStack - 1; i >= (MaxStack - 14); i--)
		contextSched.stack[i] = schedCopy[i];
}

int Parallelism::getCurrentTask()
{
	return currentTask;
}

void Parallelism::setCurrentTask(int taskNum)
// Sets the next task to be run
{
	if (taskNum <= totalTasks)
		currentTask = taskNum;
}

int Parallelism::getTotalTasks()
// Gets total tasks declared
{
	return totalTasks;
}

int Parallelism::getDeadlock()
{
	return deadlock;
}

void Parallelism::setDeadlock()
{
	deadlock = true;
}

int Parallelism::getError()
{
	return errorFlag;
}

void Parallelism::setError()
{
	errorFlag = true;
}

int Parallelism::contextSwitchOn()
// flag which enables context switch
//   DEGEL LOCALI 
{
	if (endOfTimeSlice) // is current time slice finished ?
	{
		endOfTimeSlice = false;
		context[currentTask].contextSwitchFlag = true;
		callScheduler(); // return control to the scheduler
		return 1;
	}
	context[currentTask].contextSwitchFlag = true;//   DEGEL LOCALI 

	return 0;
}

void Parallelism::contextSwitchOff()
// Disable context switch
{
	context[currentTask].contextSwitchFlag = false; //   DEGEL LOCALI 

}

int Parallelism::getContextSwitch()
{
	return context[currentTask].contextSwitchFlag;//   DEGEL LOCALI 

}

int Parallelism::getRoundTime(int taskNum)
{
	return (taskNum <= totalTasks) ? context[taskNum].roundTime : -1;
}

int Parallelism::getDeadline(int taskNum)
{
	return (taskNum <= totalTasks) ? context[taskNum].currentDeadline : -1;
}

/*
int Parallelism::getRoundCircle(int taskNum)
{
	return (taskNum <= totalTasks) ? context[taskNum].roundCircle : -1;
}
*/

int Parallelism::getRemainingRoundCircle(int taskNum)
{
	return (taskNum <= totalTasks) ? context[taskNum].remainingRoundCircle : -1;
}

void Parallelism::setDeadline(int taskNum, int deadline)
{
	(taskNum <= totalTasks) ? context[taskNum].currentDeadline = deadline : -1;
}

void Parallelism::setRoundTime(int taskNum, int roundTime)
{
	(taskNum <= totalTasks) ? context[taskNum].roundTime = roundTime : -1;
}

void Parallelism::setRemainingRoundCircle(int taskNum, int roundCircle)
{
	(taskNum <= totalTasks) ? context[taskNum].remainingRoundCircle = roundCircle : -1;
}

void Parallelism::setProgInt()
// flag indicates to the extern function 'timerInterruptHandler'
// that this is an internal SMARTS software interrupt call,
// and the original BIOS function has no to be called.
{
	progInt = true;
}

void Parallelism::resetProgInt()
{
	progInt = false;
}

int Parallelism::getProgInt()
{
	return progInt;
}

void Parallelism::setEndOfTimeSlice()
// flag indicates that when 'context switch' will be enabled,
// it must also return the control to the scheduler.
{
	endOfTimeSlice = true;
}

char Parallelism::getName(int taskNum) // returns name found or ' ' if not
{
	return (taskNum <= totalTasks) ? context[taskNum].name : ' ';
}

char Parallelism::getCurrentName()
{
	return context[currentTask].name;
}

taskStatus Parallelism::getStatus(int taskNum)
// returns status or undefined if not found
{
	return (taskNum <= totalTasks) ? context[taskNum].status : UNDEFINED;
}

taskStatus Parallelism::getCurrentStatus()
{
	return context[currentTask].status;
}

void Parallelism::resume(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].status = READY;
}

void Parallelism::resume(char taskName)
{
	for (int i = 0; i < totalTasks; ++i)
		if (context[i].name == taskName)
			context[i].status = READY;
}

void Parallelism::setCurrentNotActive()
{
	context[currentTask].status = NOT_ACTIVE;
	--activeTasks;
}
void Parallelism::suspended()
{
	context[currentTask].status = SUSPENDED;
	callScheduler();
}

void Parallelism::incrPriority(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].incrPriority();
}
void Parallelism::setOriginalPriority(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].setOriginalPriority();
}

void Parallelism::setCurrentOriginalPriority()
{
	context[currentTask].setOriginalPriority();
}

Event *Parallelism::getExpectedEvent(int taskNum)
// returns *Event  or  NULL  if not found
{
	return (taskNum <= totalTasks) ? context[taskNum].expectedEvent : NULL;
}

Event *Parallelism::getCurrentExpectedEvent()
{
	return context[currentTask].expectedEvent;
}

void Parallelism::setCurrentExpectedEvent(Event *expectedEvent)
{
	context[currentTask].expectedEvent = expectedEvent;
}

void Parallelism::sleep(int t)
// Current task sleeps for 't' milliseconds
{
	if (t < MAXINT)
	{
		context[currentTask].sleepCount = t / 55 + 1;
		context[currentTask].status = SLEEP;
		++sleepTasks;
		callScheduler(); // return control to scheduler
	}
}

void Parallelism::sleepDecr(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].sleepDecr();
}

void Parallelism::deadlineDecr(int taskNum)
{
	if (taskNum < totalTasks)
		context[taskNum].deadlineDecr();
}

void Parallelism::getCurrentStack(unsigned &StackSeg, unsigned &StackPtr)
// Load current task stack pointer
{
	StackSeg = context[currentTask].stackSeg;
	StackPtr = context[currentTask].stackPtr;
}

void Parallelism::setCurrentStack(unsigned StackSeg, unsigned StackPtr)
// Save current task stack pointer
{
	context[currentTask].stackSeg = StackSeg;
	context[currentTask].stackPtr = StackPtr;
}

void Parallelism::getSchedStack(unsigned &StackSeg, unsigned &StackPtr)
// Load scheduler  stack pointer
{
	StackSeg = contextSched.stackSeg;
	StackPtr = contextSched.stackPtr;
}

void Parallelism::handleTimers()
// handling of the sleep status mode
{
	int i;
	for (i = totalTasks - 1; i >= 0; --i)
	{
		if (getStatus(i) == SLEEP)
		{
			sleepDecr(i);
			if (getStatus(i) == READY)
				--sleepTasks;
		}
	}
	for (i = 0; i < totalTasks; i++)
	{
		deadlineDecr(i);
		/*
		cprintf("Debug info: information about task number %d", i);
		cprintf("Name: %c", context[i].name);
		cprintf("Name: %s", context[i].name);
		cprintf("Status: %s", context[i].status);
		cprintf("Remaining Round Circle: %d", context[i].remainingRoundCircle);
		cprintf("Current Deadline: %d", context[i].currentDeadline);
		*/
	}
}

void Parallelism::taskEnd()
// This function is called after the last operation of a task, instead of function return
{
	SMARTS.setCurrentNotActive();
	SMARTS.callScheduler(); // return the control to the scheduler to run a next task
}

void Parallelism::inheriteRoundTime(int owner, int roundTime)
{

	SMARTS.setRoundTime(owner, roundTime);
	for (int i = 0; i < currentMutex; i++)
	{
		Mutex mutex = mutexes[i];
		if (mutexes[i].isTaskInMutex(owner))
		{
			inheriteRoundTime(mutexes[i].getOwner(), roundTime);
			return;
		}
	}
	return;
}

/**********    class body:  Task     **********/
Task::Task()
{
	stack[MaxStack - 14] = _BP;
	stack[MaxStack - 13] = _DI;
	stack[MaxStack - 12] = _SI;
	stack[MaxStack - 11] = _DS;
	stack[MaxStack - 10] = _ES;
	stack[MaxStack - 9] = _DX;
	stack[MaxStack - 8] = _CX;
	stack[MaxStack - 7] = _BX;
	stack[MaxStack - 6] = _AX;
	stackSeg = FP_SEG(&stack[MaxStack - 14]);
	stackPtr = FP_OFF(&stack[MaxStack - 14]);
	status = NOT_ACTIVE;
	sleepCount = 0;
	currentPriority = priority = 0;
	contextSwitchFlag = true;
}
//-----------------------------------------------------
void Task::declare(void far *code, void far *userTaskEnd, char name, int roundTime, int roundCircle)
{
	stack[MaxStack - 5] = FP_OFF(code);
	stack[MaxStack - 4] = FP_SEG(code);
	stack[MaxStack - 3] = _FLAGS;
	stack[MaxStack - 2] = FP_OFF(userTaskEnd);
	stack[MaxStack - 1] = FP_SEG(userTaskEnd);
	this->name = name;
	this->roundTime = roundTime;
	this->remainingRoundCircle = roundCircle;
	this->currentDeadline = roundTime;
	status = READY;
	saveState();
}

void Task::saveState()
{
	memcpy(initialStack, stack, sizeof(stack));
	initialStackSeg = stackSeg;
	initialStackPtr = stackPtr;
}

void Task::redeclare()
{
	memcpy(stack, initialStack, sizeof(initialStack));
	stackSeg = initialStackSeg;
	stackPtr = initialStackPtr;
	this->currentDeadline = roundTime;
	this->remainingRoundCircle = this->remainingRoundCircle - 1;
	status = READY;
}

//----------------------------------------------------
void Task::incrPriority()
{
	--currentPriority;
}
//----------------------------------------------------
void Task::setOriginalPriority()
{
	currentPriority = priority;
}
//----------------------------------------------------
void Task::sleepDecr()
// Decrements the sleep counter and update the task status accordingly

// dormir pendant un certain temps spécifié en millisecondes.
{
	if (status == SLEEP)
	{
		if (sleepCount > 0)
			--sleepCount;
		if (!sleepCount)
			status = READY;
	}
}

void Task::deadlineDecr()
{
	if (currentDeadline > 0)
		--currentDeadline;
	if (!currentDeadline)
	{
		if (status != NOT_ACTIVE) // The task was not completed within deadline
		{
			SMARTS.setError();
		}
		else if (remainingRoundCircle - 1) // There are more rounds left
			redeclare();
	}
}

/////////////////////////////////////////////////
/**********    class body:  Mutex     **********/

Mutex::Mutex()
{
	owner = -1;
	s = 1;
	level = 0;
	inheritence = 0;
}

void Mutex::enableInheritence() 
// activer l'héritage de priorité pour les tâches qui utilisent ce mutex.
{
	inheritence = 1;   //  Si ya un taalih qui vien plus prioritaire, il va herité de ce taalih 
}

void Mutex::Acquire()

//pour acquérir un mutex. 
//Si le mutex est déjà pris par une autre tâche, la tâche courante est mise en attente jusqu'à ce qu'elle puisse acquérir le mutex.
{
	SMARTS.contextSwitchOff();
	if (owner == SMARTS.getCurrentTask() || s == 1)
	{
		s = 0;
	}
	else
	{
		queue.enQueue(SMARTS.getCurrentTask());
		if (inheritence)
		{
			int ownerRoundTime = SMARTS.getRoundTime(owner);
			int currentRoundTime = SMARTS.getRoundTime(SMARTS.getCurrentTask());
			if (ownerRoundTime > currentRoundTime)
			{
				//SMARTS.setRoundTime(owner, currentRoundTime);
				SMARTS.inheriteRoundTime(owner, currentRoundTime);
			}
		}
		SMARTS.suspended();
	}
	owner = SMARTS.getCurrentTask();
	originalRoundTime = SMARTS.getRoundTime(SMARTS.getCurrentTask());
	level++;
	SMARTS.contextSwitchOn();
}

void Mutex::Release()
// pour libérer un mutex, permettant ainsi à une autre tâche en attente d'acquérir le mutex.
{
	SMARTS.contextSwitchOff();
	if (owner == SMARTS.getCurrentTask())
	{
		if (--level)
			return;
		else
		{
			if (inheritence)
			{
				SMARTS.setRoundTime(owner, originalRoundTime);
			}
			owner = -1;
			if (!queue.isEmpty())
			{
				int task = queue.deQueue();
				SMARTS.resume(task);
			}
			else
			{
				s = 1;
			}
		}
	}
	SMARTS.contextSwitchOn();
}

int Mutex::isTaskInMutex(int taskNumber)
{
	return queue.isNumberInQueue(taskNumber);
}

int Mutex::getOwner()
{
	return owner;
}

Queue ::Queue()
{
	front = rear = -1;
	size = 5;
}

void Queue::enQueue(int value)
{
	// displayQueue();
	if (isFull())
	{
		printf("\nQueue is Full");
		return;
	}

	if (isEmpty())
	{
		front = rear = 0;
	}
	else
	{
		rear = (rear + 1) % size;
	}

	arr[rear] = value;
}

int Queue::deQueue()
{
	if (isEmpty())
	{
		printf("\nQueue is Empty");
		return -1;
	}

	int data = arr[front];
	arr[front] = -1;

	if (front == rear)
	{
		front = rear = -1;
	}
	else
	{
		front = (front + 1) % size;
	}

	return data;
}

void Queue::displayQueue()
{
	printf("Front: %d\n", front);
	printf("Rear: %d\n", rear);
	printf("Size: %d\n", size);

	if (isEmpty())
	{
		printf("Queue is Empty");
		return;
	}

	printf("Elements in Circular Queue are: ");

	for (int j = 0; j < 5; j++)
	{
		printf("%d ", arr[j]);
	}

	printf("\n"); // Add a newline character after printing the queue elements
}

int Queue::isFull()
{
	return (front == 0 && rear == size - 1) || (rear == (front - 1) % (size - 1));
}

int Queue::isEmpty()
{
	return front == -1;
}

int Queue::isNumberInQueue(int number)
{
	if (isEmpty())
		return false;

	for (int i = 0; i < size; i++)
	{
		if (arr[i] == number)
		{
			return true; // Number found in the queue
		}
	}

	return false; // Number not found in the queue
}
