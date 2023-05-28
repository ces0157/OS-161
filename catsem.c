/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>
/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

typedef int bool; //creates boolean datatype
#define TRUE 1
#define FALSE 0 


//Decclares all nessecary varaibles to be used
volatile bool all_dishes_available = TRUE; //shared varaible
static struct semaphore *done; //indicates we are done eating
static struct semaphore *mutex; //creates a crtical section

static struct semaphore *dish_mutex; //critical section when eating

//variables to be used by cats
static struct semaphore *cats_queue; //holds the cats
volatile int cats_wait_count = 0; //how many cats are waiting to eat, shared varaible
volatile bool no_cat_eat = TRUE; //is a cat in the kitchen

//variables to be used by mice
static struct semaphore *mice_queue; //holds the mice
volatile int mice_wait_count = 0; //how many mice are waiting, shared varaible
volatile bool no_mouse_eat = TRUE; //is a mice in the kitchen 

volatile int cats_done = 0; //how many cats have finished eating
volatile int mice_done = 0; //how many mice have finished eating

volatile bool dish1_busy = FALSE; //is someone eating
volatile bool dish2_busy = FALSE; //is someone eating

/*
 * 
 * Function Definitions
 * 
 */


/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
 
	//(void) unusedpointer;
        (void) catnumber;
	
	
	bool first_cat_eat = FALSE;
	bool another_cat_eat = FALSE;       
	int  mydish = 0; //indicates the dish the cat is eting at
	P(mutex); //used for shared variable 
	/*
 	* Used to let the first cat in
 	*/
	if(all_dishes_available) {
		all_dishes_available = FALSE;
		V(cats_queue);	
	}
	cats_wait_count++; //we know have at least one waiting cat
	V(mutex);
	
	P(cats_queue); //first cat in, the other cat waits
	P(mutex);
	
	/*
 	* our first cat can enter if no one is dining room
 	*/
	if(no_cat_eat) {
		no_cat_eat = FALSE;
		first_cat_eat = TRUE; 
		
	}
	else {
		first_cat_eat = FALSE;
	}
	

	V(mutex);
	
	
	if(first_cat_eat == TRUE) {
		P(mutex); //we are acessing a shared varaible, create cs
		/*
 		*If another cat is waiting we will let them in
 		*/
		if(cats_wait_count > 1){
			another_cat_eat = TRUE;
			V(cats_queue); //let another cat in	
		}		
		
		V(mutex); //done modifying shared varaible
	
	
	}
	
	kprintf(">>> Cat %d enters the kitchen. \n", catnumber); /*cat name */	
	P(dish_mutex); //protecting the dish-shared varaibles
	
	if(dish1_busy == FALSE) { //if the first cat is not eating yet
		dish1_busy = TRUE;
		mydish = 1;
	}
	else {
		assert(dish2_busy == FALSE);
		dish2_busy = TRUE;
		mydish = 2;
	}

	V(dish_mutex); //done protecting the dish-shared varaibles
	kprintf("*** Cat %d eating at dish %d \n", catnumber, mydish); //cat name
	clocksleep(1); //enjoys food
	kprintf("*** Cat %d finish eating at dish %d \n", catnumber, mydish); //done eating
	
	P(dish_mutex); //protecting dish-shared varaibles
	if(mydish == 1) { //we will release dish 1
		dish1_busy = FALSE;
	}
	else { //we will release dish 2
		//assert(mydish == 2);
		dish2_busy = FALSE;		
	}
	
	V(dish_mutex); //done protecting the dish-shared varaibles
	P(mutex); //protecting shared varaibels
	cats_wait_count --; //there is one less cat in the kitchen
	V(mutex);


	if(first_cat_eat == TRUE) {
		if(another_cat_eat == TRUE) { //both cats are done and will leave together
			P(done); //******IMPORTANT !!! NEED TO ADD SIGNAL DONE SOME where
		}
		
		kprintf("<<<Cat %d is leaving.\n", catnumber);
		P(mutex); //protecting shared varaible
		no_cat_eat = TRUE;
		V(mutex); //done protecting shared varaible
		
		P(mutex); //protect shared varaible
		cats_done++; //since cat is leaving, we increase the done count by 1
		V(mutex); //done protecting

		kprintf("Number of cat's done: %d \n", cats_done);		
		
		//we will now switch to any waiting mice
		switch_turn();			
	}
	
	else {
		kprintf("<<< Cat %d is leaving \n", catnumber);
		P(mutex);
		cats_done++; //since cat is leaving we increase the done count by 1
		V(mutex);
	        kprintf("Number of cat's done: %d\n", cats_done); 
		V(done);
	
	}
	

	
}

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{
      	bool first_mouse_eat = FALSE;
	bool another_mouse_eat = FALSE;	      
         int  mydish = 0;
       // (void) unusedpointer;
        (void) mousenumber;
       
	P(mutex); //used for shared variable 
        
        if(all_dishes_available) { //used to let the first mouse in
                all_dishes_available = FALSE;
                V(mice_queue);
        }
        mice_wait_count++; //we know have at least one waiting mouse
        V(mutex);

        P(mice_queue); //first mouse in, the other mouse waits
	P(mutex);

        
        if(no_mouse_eat) { //first mouse can enter the dining room 
                no_mouse_eat = FALSE;
                first_mouse_eat = TRUE;

        }
        else {
                first_mouse_eat = FALSE;
        }


        V(mutex);

	 if(first_mouse_eat == TRUE) {
                P(mutex); //we are acessing a shared varaible, create cs
                if(mice_wait_count > 1){ //if another mouse is waiting, let them in
                        another_mouse_eat = TRUE;
                        V(mice_queue); //let another cat in     
                }

                V(mutex); //done modifying shared varaible


        }

        kprintf("Mice %d enters the kitchen. \n", mousenumber); /*mice name */	
	
	P(dish_mutex); //protecting the dish-shared varaibles

        if(dish1_busy == FALSE) { //if the first mouse is not eating yet
                dish1_busy = TRUE;
                mydish = 1;
        }
        else {  
                assert(dish2_busy ==FALSE);
                dish2_busy = TRUE;
                mydish = 2;
        }

        V(dish_mutex); //done protecting the dish-shared varaibles
        kprintf("Mice %d eating at dish %d \n", mousenumber, mydish); //mice name
        clocksleep(1); //enjoys food
        kprintf("Mice %d finish eating at dish  %d \n", mousenumber, mydish); //done eating
	
	
	  P(dish_mutex); //protecting dish-shared varaibles
        if(mydish == 1) { //we will release dish 1
                dish1_busy = FALSE;
        }
        else { //we will release dish 2
               // assert(dish2_busy == TRUE);
                dish2_busy = FALSE;
        }

        V(dish_mutex); //done protecting the dish-shared varaibles
        P(mutex); //protecting shared varaibels
        mice_wait_count --; //there is one less mouse in the kitchen
        V(mutex);
	
	 if(first_mouse_eat == TRUE) {
                if(another_mouse_eat == TRUE) { //both mice  are done and will leave together
                        P(done); //******IMPORTANT !!! NEED TO ADD SIGNAL DONE SOME where
                }
		kprintf("<<<Mouse  %d is leaving.\n", mousenumber);				
                P(mutex); //protecting shared varaible
                no_mouse_eat = TRUE;
                V(mutex); //done protecting shared varaibles
		
		P(mutex);
		mice_done++; //number of mice done increass
		V(mutex);
		kprintf("Number of Mice done: %d\n", mice_done);		
			
		//we will now initiate the switch
		switch_turn();
	}
	
	else {
		kprintf("<<<Mouse  %d is leaving \n", mousenumber);
		P(mutex);
		mice_done++; //number of mice done increases
		V(mutex);
		kprintf("Number of Mice done: %d\n", mice_done);
		V(done);		

	}	
	

}


/*
 * Switches the animal type
 * @return nothing
 */
void switch_turn() {
	 	P(mutex); //protecting shared varaible
                if(mice_wait_count > 0) { //mice are waiting signal a switch            
                        V(mice_queue);
                }
                else if(cats_wait_count > 0) { //no mice are waiting, but cats are
                        V(cats_queue);
                }
                else { //no one is currently waiting, give up all dishes
                        all_dishes_available = TRUE;
                }
                V(mutex); //done protecting shared varaible 	
}



/*
 * Method that init's all mutex's
 * @return nothing
 */
static void setup() {
	mutex = sem_create("general mutex", 1); 
        done = sem_create("indicates everyone is done eating", 0); 
        dish_mutex = sem_create("critical section for eating", 1);
        cats_queue = sem_create("Holds the cats", 0); 
        mice_queue = sem_create("Holds the mice", 0);  


}

/*
 * Method indicating that we done. Destroys all mutex's
 * @return nothing
 */
static void cleanup() {
	sem_destroy(mutex);
	sem_destroy(dish_mutex);
	sem_destroy(cats_queue);
	sem_destroy(mice_queue);
	kprintf("Done Cleaning up!\n");


}

/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
	setup(); //sets up our mutex
	/*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
  


        /*
         * Start NCATS catsem() threads.
         */
	
        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );      
        
	      /*
                 * panic() on error.
                 */
           
		if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        /*
         * Start NMICE mousesem() threads.
         *
         */
	
	for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                   ); 
	       /*
                 * panic() on error.
                 */	
		     if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
	
	
	while(mice_done < 2 || cats_done < 6) { //we continously wait unil both conditions are met
		clocksleep(1);
	}  	
	cleanup(); //destory mutex's
        return 0; //we are done
}




/*
 * End of catsem.c
 */
