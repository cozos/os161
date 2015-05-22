/*
 * catmouse.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 * 26-11-2007: KMS : Modified to use cat_eat and mouse_eat
 * 21-04-2009: KMS : modified to use cat_sleep and mouse_sleep
 * 21-04-2009: KMS : added sem_destroy of CatMouseWait
 *
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
#include "opt-A1.h"

/*
 * 
 * cat,mouse,bowl simulation functions defined in bowls.c
 *
 * For Assignment 1, you should use these functions to
 *  make your cats and mice eat from the bowls.
 * 
 * You may *not* modify these functions in any way.
 * They are implemented in a separate file (bowls.c) to remind
 * you that you should not change them.
 *
 * For information about the behaviour and return values
 *  of these functions, see bowls.c
 *
 */

/* this must be called before any calls to cat_eat or mouse_eat */
extern int initialize_bowls(unsigned int bowlcount);

extern void cleanup_bowls( void );
extern void cat_eat(unsigned int bowlnumber);
extern void mouse_eat(unsigned int bowlnumber);
extern void cat_sleep(void);
extern void mouse_sleep(void);

/*
 *
 * Problem parameters
 *
 * Values for these parameters are set by the main driver
 *  function, catmouse(), based on the problem parameters
 *  that are passed in from the kernel menu command or
 *  kernel command line.
 *
 * Once they have been set, you probably shouldn't be
 *  changing them.
 *
 *
 */
int NumBowls;  // number of food bowls
int NumCats;   // number of cats
int NumMice;   // number of mice
int NumLoops;  // number of times each cat and mouse should eat
volatile int cateating;
volatile int mouseeating;
volatile int catsturn = 1;
static int *bowls;

struct cv *freebowls;

/*
 * Once the main driver function (catmouse()) has created the cat and mouse
 * simulation threads, it uses this semaphore to block until all of the
 * cat and mouse simulations are finished.
 */
struct semaphore *CatMouseWait;

/*
 * 
 * Function Definitions
 * 
 */


int find_bowl(){
    int i;
    for (i = 1; i < NumBowls+1; i++){
        if (bowls[i] == 0){
            return i;
        }
    }
    return -1;
}
/*
 * cat_simulation()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds cat identifier from 0 to NumCats-1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Each cat simulation thread runs this function.
 *
 *      You need to finish implementing this function using 
 *      the OS161 synchronization primitives as indicated
 *      in the assignment description
 */



static
void
cat_simulation(void * unusedpointer, 
	       unsigned long catnumber)
{
    (void)unusedpointer;
    (void)catnumber;
    int i;
    struct lock *mutex;
    struct lock *lbowl;
    lbowl = lock_create("merp" + catnumber);
    mutex = lock_create("derp" + catnumber);
    
  for(i=0;i<NumLoops;i++) {

      int bowl;
      
     
      cat_sleep();
      

      
      lock_acquire(mutex);
      
      while(mouseeating > 0 || find_bowl() == -1 || catsturn == 0){
          cv_wait(freebowls,mutex);
      }
     
      lock_acquire(lbowl);
      cateating++;
      bowl = find_bowl();
      assert(bowl != -1);
      bowls[bowl] = 1;
      
      cat_eat(bowl);
      lock_release(lbowl);
      
      cateating--;
      bowls[bowl] = 0;
      
      if(cateating == 0){
          catsturn = 0;
          cv_broadcast(freebowls,mutex);
      }
      
      lock_release(mutex);
      
     
  }
    catsturn = 0;
    lock_destroy(mutex);
    lock_destroy(lbowl);
  V(CatMouseWait); 
}
	
/*
 * mouse_simulation()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds mouse identifier from 0 to NumMice-1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      each mouse simulation thread runs this function
 *
 *      You need to finish implementing this function using 
 *      the OS161 synchronization primitives as indicated
 *      in the assignment description
 *
 */

static
void
mouse_simulation(void * unusedpointer,
          unsigned long mousenumber)
{
    (void)unusedpointer;
    (void)mousenumber;
    int bowl;
    int i;
    struct lock *mutex;
    struct lock *lbowl;
    lbowl = lock_create("derp" + mousenumber);
    mutex = lock_create("herp" + mousenumber);
  
  for(i=0;i<NumLoops;i++) {

      
      mouse_sleep();

      
      
      lock_acquire(mutex);
      
      while(cateating > 0 || find_bowl() == -1 || catsturn == 1 ){
          cv_wait(freebowls,mutex);
      }
      
      lock_acquire(lbowl);
      mouseeating++;
      bowl = find_bowl();
      assert(bowl != -1);
      bowls[bowl] = 1;
      
     
      mouse_eat(bowl);
      lock_release(lbowl);
	
      if(mouseeating == NumBowls || mouseeating == NumMice){
          catsturn = 1;
          cv_broadcast(freebowls,mutex);
      }   
   
      mouseeating--;
      bowls[bowl] = 0;
      
      if(mouseeating == 0){
          catsturn = 1;
          cv_broadcast(freebowls,mutex);
      }
      
      lock_release(mutex);
  

  }
    catsturn = 1;
    lock_destroy(mutex);
    lock_destroy(lbowl);
  V(CatMouseWait); 
}


/*
 * catmouse()
 *
 * Arguments:
 *      int nargs: should be 5
 *      char ** args: args[1] = number of food bowls
 *                    args[2] = number of cats
 *                    args[3] = number of mice
 *                    args[4] = number of loops
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up cat_simulation() and
 *      mouse_simulation() threads.
 *      You may need to modify this function, e.g., to
 *      initialize synchronization primitives used
 *      by the cat and mouse threads.
 *      
 *      However, you should should ensure that this function
 *      continues to create the appropriate numbers of
 *      cat and mouse threads, to initialize the simulation,
 *      and to wait for all cats and mice to finish.
 */

int
catmouse(int nargs,
	 char ** args)
{
  int index, error;
  int i;

  /* check and process command line arguments */
  if (nargs != 5) {
    kprintf("Usage: <command> NUM_BOWLS NUM_CATS NUM_MICE NUM_LOOPS\n");
    return 1;  // return failure indication
  }

  /* check the problem parameters, and set the global variables */
  NumBowls = atoi(args[1]);
  if (NumBowls <= 0) {
    kprintf("catmouse: invalid number of bowls: %d\n",NumBowls);
    return 1;
  }
  NumCats = atoi(args[2]);
  if (NumCats < 0) {
    kprintf("catmouse: invalid number of cats: %d\n",NumCats);
    return 1;
  }
  NumMice = atoi(args[3]);
  if (NumMice < 0) {
    kprintf("catmouse: invalid number of mice: %d\n",NumMice);
    return 1;
  }
  NumLoops = atoi(args[4]);
  if (NumLoops <= 0) {
    kprintf("catmouse: invalid number of loops: %d\n",NumLoops);
    return 1;
  }
  kprintf("Using %d bowls, %d cats, and %d mice. Looping %d times.\n",
          NumBowls,NumCats,NumMice,NumLoops);

  /* create the semaphore that is used to make the main thread
     wait for all of the cats and mice to finish */
  CatMouseWait = sem_create("CatMouseWait",0);
  if (CatMouseWait == NULL) {
    panic("catmouse: could not create semaphore\n");
  }
 
    //mutex = lock_create("mutex");
    freebowls = cv_create("freebowls");

  /* 
   * initialize the bowls
   */
  if (initialize_bowls(NumBowls)) {
    panic("catmouse: error initializing bowls.\n");
  }
    
    bowls = kmalloc((NumBowls+1)*sizeof(int));
    if (bowls == NULL) {
        panic("initialize_bowls: unable to allocate space for %d bowls\n",NumBowls);
    }
    /* initialize bowls */
    for(i=0;i<NumBowls;i++) {
        bowls[i] = 0;
    }

  /*
   * Start NumCats cat_simulation() threads.
   */
  for (index = 0; index < NumCats; index++) {
    error = thread_fork("cat_simulation thread",NULL,index,cat_simulation,NULL);
    if (error) {
      panic("cat_simulation: thread_fork failed: %s\n", strerror(error));
    }
  }

  /*
   * Start NumMice mouse_simulation() threads.
   */
  for (index = 0; index < NumMice; index++) {
    error = thread_fork("mouse_simulation thread",NULL,index,mouse_simulation,NULL);
    if (error) {
      panic("mouse_simulation: thread_fork failed: %s\n",strerror(error));
    }
  }

  /* wait for all of the cats and mice to finish before
     terminating */  
  for(i=0;i<(NumCats+NumMice);i++) {
    P(CatMouseWait);
  }

  /* clean up the semaphore that we created */
  sem_destroy(CatMouseWait);
   // lock_destroy(mutex);
    cv_destroy(freebowls);
  /* clean up resources used for tracking bowl use */
  cleanup_bowls();

    if (bowls != NULL) {
        kfree( bowls );
        bowls = NULL;
    }
  

  return 0;
}

/*
 * End of catmouse.c
 */