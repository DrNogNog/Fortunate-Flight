#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Limit constants
#define MAX_CITY_NAME_LEN 20
#define MAX_FLIGHTS_PER_CITY 5
#define MAX_DEFAULT_SCHEDULES 50

// Time definitions
#define TIME_MIN 0
#define TIME_MAX ((60 * 24)-1)
#define TIME_NULL -1


/******************************************************************************
 * Structure and Type definitions                                             *
 ******************************************************************************/
typedef int time_t;                        // integers used for time values
typedef char city_t[MAX_CITY_NAME_LEN+1];; // null terminate fixed length city
 
// Structure to hold all the information for a single flight
//   A city's schedule has an array of these
struct flight {
  time_t time;       // departure time of the flight
  int available;  // number of seats currently available on the flight
  int capacity;   // maximum seat capacity of the flight
};

// Structure for an individual flight schedule
// The main data structure of the program is an Array of these structures
// Each structure will be placed on one of two linked lists:
//                free or active
// Initially the active list will be empty and all the schedules
// will be on the free list.  Adding a schedule is finding the first
// free schedule on the free list, removing it from the free list,
// setting its destination city and putting it on the active list
struct flight_schedule {
  city_t destination;                          // destination city name
  struct flight flights[MAX_FLIGHTS_PER_CITY]; // array of flights to the city
  struct flight_schedule *next;                // link list next pointer
  struct flight_schedule *prev;                // link list prev pointer
};

/******************************************************************************
 * Global / External variables                                                *
 ******************************************************************************/
// This program uses two global linked lists of Schedules.  See comments
// of struct flight_schedule above for details
struct flight_schedule *flight_schedules_free = NULL;
struct flight_schedule *flight_schedules_active = NULL;


/******************************************************************************
 * Function Prototypes                                                        *
 ******************************************************************************/
// Misc utility io functions
int city_read(city_t city);           
bool time_get(time_t *time_ptr);      
bool flight_capacity_get(int *capacity_ptr);
void print_command_help(void);

// Core functions of the program
void flight_schedule_initialize(struct flight_schedule array[], int n);
struct flight_schedule * flight_schedule_find(city_t city);
struct flight_schedule * flight_schedule_allocate(void);
void flight_schedule_free(struct flight_schedule *fs);
void flight_schedule_add(city_t city);
void flight_schedule_listAll(void);
void flight_schedule_list(city_t city);
void flight_schedule_add_flight(city_t city);
void flight_schedule_remove_flight(city_t city);
void flight_schedule_schedule_seat(city_t city);
void flight_schedule_unschedule_seat(city_t city);
void flight_schedule_remove(city_t city);

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs);
int  flight_compare_time(const void *a, const void *b);


int main(int argc, char *argv[]) 
{
  long n = MAX_DEFAULT_SCHEDULES;
  char command;
  city_t city;

  if (argc > 1) {
    // If the program was passed an argument then try and convert the first
    // argument in the a number that will override the default max number
    // of schedule we will support
    char *end;
    n = strtol(argv[1], &end, 10); // CPAMA p 787
    if (n==0) {
      printf("ERROR: Bad number of default max scedules specified.\n");
      exit(EXIT_FAILURE);
    }
  }

  // C99 lets us allocate an array of a fixed length as a local 
  // variable.  Since we are doing this in main and a call to main will be 
  // active for the entire time of the program's execution this
  // array will be alive for the entire time -- its memory and values
  // will be stable for the entire program execution.
  struct flight_schedule flight_schedules[n];
 
  // Initialize our global lists of free and active schedules using
  // the elements of the flight_schedules array
  flight_schedule_initialize(flight_schedules, n);

  // DEFENSIVE PROGRAMMING:  Write code that avoids bad things from happening.
  //  When possible, if we know that some particular thing should have happened
  //  we think of that as an assertion and write code to test them.
  // Use the assert function (CPAMA p749) to be sure the initilization has set
  // the free list to a non-null value and the the active list is a null value.
  assert(flight_schedules_free != NULL && flight_schedules_active == NULL);

  // Print the instruction in the beginning
  print_command_help();

  // Command processing loop
  while (scanf(" %c", &command) == 1) {
    switch (command) {
    case 'A': 
      //  Add an active flight schedule for a new city eg "A Toronto\n"
      city_read(city);
      flight_schedule_add(city);

      break;
    case 'L':
      // List all active flight schedules eg. "L\n"
      flight_schedule_listAll();
      break;
    case 'l': 
      // List the flights for a particular city eg. "l\n"
      city_read(city);
      flight_schedule_list(city);
      break;
    case 'a':
      // Adds a flight for a particular city "a Toronto\n
      //                                      360 100\n"
      city_read(city);
      flight_schedule_add_flight(city);
      break;
    case 'r':
      // Remove a flight for a particular city "r Toronto\n
      //                                        360\n"
      city_read(city);
      flight_schedule_remove_flight(city);
	break;
    case 's':
      // schedule a seat on a flight for a particular city "s Toronto\n
      //                                                    300\n"
      city_read(city);
      flight_schedule_schedule_seat(city);
      break;
    case 'u':
      // unschedule a seat on a flight for a particular city "u Toronto\n
      //                                                      360\n"
        city_read(city);
        flight_schedule_unschedule_seat(city);
        break;
    case 'R':
      // remove the schedule for a particular city "R Toronto\n"
      city_read(city);
      flight_schedule_remove(city);  
      break;
    case 'h':
        print_command_help();
        break;
    case 'q':
      goto done;
    default:
      printf("Bad command. Use h to see help.\n");
    }
  }
 done:
  return EXIT_SUCCESS;
}

/**********************************************************************
 * city_read: Takes in and processes a given city following a command *
 *********************************************************************/
int city_read(city_t city) {
  int ch, i=0;

  // skip leading non letter characters
  while (true) {
    ch = getchar();
    if ((ch >= 'A' && ch <= 'Z') || (ch >='a' && ch <='z')) {
      city[i++] = ch;
      break;
    }
  }
  while ((ch = getchar()) != '\n') {
    if (i < MAX_CITY_NAME_LEN) {
      city[i++] = ch;
    }
  }
  city[i] = '\0';
  return i;
}


/****************************************************************
 * Message functions so that your messages match what we expect *
 ****************************************************************/
void msg_city_bad(char *city) {
  printf("No schedule for %s\n", city);
}

void msg_city_exists(char *city) {
  printf("There is a schedule of %s already.\n", city);
}

void msg_schedule_no_free(void) {
  printf("Sorry no more free schedules.\n");
}

void msg_city_flights(char *city) {
  printf("The flights for %s are:", city);
}

void msg_flight_info(int time, int avail, int capacity) {
  printf(" (%d, %d, %d)", time, avail, capacity);
}

void msg_city_max_flights_reached(char *city) {
  printf("Sorry we cannot add more flights on this city.\n");
}

void msg_flight_bad_time(void) {
  printf("Sorry there's no flight scheduled on this time.\n");
}

void msg_flight_no_seats(void) {
    printf("Sorry there's no more seats available!\n");
}

void msg_flight_all_seats_empty(void) {
  printf("All the seats on this flights are empty!\n");
}

void msg_time_bad() {
  printf("Invalid time value\n");
}

void msg_capacity_bad() {
  printf("Invalid capacity value\n");
}

void print_command_help()
{
  printf("Here are the possible commands:\n"
	 "A <city name>     - Add an active empty flight schedule for\n"
	 "                    <city name>\n"
	 "L                 - List cities which have an active schedule\n"
	 "l <city name>     - List the flights for <city name>\n"
	 "a <city name>\n"
         "<time> <capacity> - Add a flight for <city name> @ <time> time\n"
	 "                    with <capacity> seats\n"  
	 "r <city name>\n"
         "<time>            - Remove a flight form <city name> whose time is\n"
	 "                    <time>\n"
	 "s <city name>\n"
	 "<time>            - Attempt to schedule seat on flight to \n"
	 "                    <city name> at <time> or next closest time on\n"
	 "                    which their is an available seat\n"
	 "u <city name>\n"
	 "<time>            - unschedule a seat from flight to <city name>\n"
	 "                    at <time>\n"
	 "R <city name>     - Remove schedule for <city name>\n"
	 "h                 - print this help message\n"
	 "q                 - quit\n"
);
}


/****************************************************************
 * Resets a flight schedule                                     *
 ****************************************************************/
void flight_schedule_reset(struct flight_schedule *fs) {
    fs->destination[0] = 0;
    for (int i=0; i<MAX_FLIGHTS_PER_CITY; i++) {
      fs->flights[i].time = TIME_NULL;
      fs->flights[i].available = 0;
      fs->flights[i].capacity = 0;
    }
    fs->next = NULL;
    fs->prev = NULL;
}

/******************************************************************
* Initializes the flight_schedule array that will hold any flight *
* schedules created by the user. This is called in main for you.  *
 *****************************************************************/

void flight_schedule_initialize(struct flight_schedule array[], int n)
{
  flight_schedules_active = NULL;
  flight_schedules_free = NULL;

  // takes care of empty array case
  if (n==0) return;

  // Loop through the Array connecting them
  // as a linear doubly linked list
  array[0].prev = NULL;
  for (int i=0; i<n-1; i++) {
    flight_schedule_reset(&array[i]);
    array[i].next = &array[i+1];
    array[i+1].prev = &array[i];
  }
  // Takes care of last node.  
  flight_schedule_reset(&array[n-1]); // reset clears all fields
  array[n-1].next = NULL;
  array[n-1].prev = &array[n-2];
  flight_schedules_free = &array[0];
}

/***********************************************************
 * time_get: read a time from the user
   Time in this program is a minute number 0-((24*60)-1)=1439
   -1 is used to indicate the NULL empty time 
   This function should read in a time value and check its 
   validity.  If it is not valid eg. not -1 or not 0-1439
   It should print "Invalid Time" and return false.
   othewise it should return the value in the integer pointed
   to by time_ptr.
 ***********************************************************/
bool time_get(int *time_ptr) {
  if (scanf("%d", time_ptr)==1) {
    return (TIME_NULL == *time_ptr || 
	    (*time_ptr >= TIME_MIN && *time_ptr <= TIME_MAX));
  } 
  msg_time_bad();
  return false;
}

/***********************************************************
 * flight_capacity_get: read the capacity of a flight from the user
   This function should read in a capacity value and check its 
   validity.  If it is not greater than 0, it should print 
   "Invalid capacity value" and return false. Othewise it should 
   return the value in the integer pointed to by cap_ptr.
 ***********************************************************/
bool flight_capacity_get(int *cap_ptr) {
  if (scanf("%d", cap_ptr)==1) {
    return *cap_ptr > 0;
  }
  msg_capacity_bad();
  return false;
}

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs) 
{
  qsort(fs->flights, MAX_FLIGHTS_PER_CITY, sizeof(struct flight),
	flight_compare_time);
}

int flight_compare_time(const void *a, const void *b) 
{
  const struct flight *af = a;
  const struct flight *bf = b;
  
  return (af->time - bf->time);
}

//WRITE SOME CODE!


//time to work I guess


//Function 1: Moves schedule from free list to active list
struct flight_schedule * flight_schedule_allocate()
{
  if (flight_schedule_free == NULL){
    msg_schedule_no_free();
  }
  //Creates temp struct and moves first schedule from free to active 
  struct flight_schedule * temp = flight_schedules_active;
  flight_schedules_active = flight_schedules_free;
  flight_schedules_free = flight_schedules_free->next;
  flight_schedules_active->next = temp;
  
  if(temp != NULL){
    temp->prev = flight_schedules_active;
  }

  if (flight_schedules_free != NULL){
    flight_schedules_free->prev = NULL;
  }

return flight_schedules_active;
}




//Function 2: Moves schedule from active list to free list
void flight_schedule_free(struct flight_schedule * schedule){

//checks the one case where there's only one node in flight_schedules_active
if (flight_schedules_active == schedule){
  flight_schedules_active = schedule->next;
}

//Removes from active list by changing pointers
if (schedule->next != NULL){
  schedule->next->prev = schedule->prev;
}
if (schedule->prev!= NULL){
schedule->prev->next = schedule->next;
}

//clears the schedule in active
flight_schedule_reset(schedule);


schedule->next = flight_schedules_free;
if (flight_schedules_free != NULL){
  schedule->next->prev = schedule;
}
flight_schedules_free = schedule;
}



//Function 3: Adds schedule to active with input city
void flight_schedule_add(city_t city){
struct flight_schedule * schedule = flight_schedule_find(city);

//if city not in flight_schedules_active
if(schedule != NULL){
  msg_city_exists(city);
}
else if(flight_schedules_free == NULL){
  msg_schedule_no_free();
}

else{
struct flight_schedule * schedule = flight_schedule_allocate();

strcpy(schedule->destination, city);
}
}


//Function 4: Removes schedule from active and puts it into free
void flight_schedule_remove(city_t city){
struct flight_schedule * schedule = flight_schedule_find(city);

if (schedule == NULL){
  msg_city_bad(city);
}

else{
  flight_schedule_free(schedule);
}
}



//Function 5:  Just lists all present schedules
void flight_schedule_listAll(){
struct flight_schedule * schedule = flight_schedules_active;
while (schedule != NULL)
  printf("%s\n", schedule->destination);
  schedule = schedule->next;
}

}

//Function 6: Lists all flights heading to a specific city
void flight_schedule_list(city_t city){
struct flight_schedule * city_flight = flight_schedule_find(city);

if (city_flight == NULL){
  msg_city_bad(city);
}

else
{
int x,y,z;
int size = sizeof city_flight->flights/sizeof city_flight->flights[0];
msg_city_flights(city);
for (int i=0; i<size; i++) {
      x = city_flight->flights[i].time;
      y = city_flight->flights[i].available;
      z = city_flight->flights[i].capacity;
      if (x>0){
      printf(" (%d, %d, %d)", x,y,z);
      }
}
}
printf("\n");
}
//Function 7: Adds flight to whatever input city is given
void flight_schedule_add_flight(city_t city){
  struct flight_schedule * schedule = flight_schedule_find(city);
  if (schedule == NULL)
  {
    msg_city_bad(city);
  }
  else{
    time_t time;
    int capacity;
    if (time_get(&time) && flight_capacity_get(&capacity)){
    bool success = false;
    for (int i = 0; i<MAX_FLIGHTS_PER_CITY;i++)
    {
      if (schedule->flights[i].time == TIME_NULL){
        schedule->flights[i].time = time;
        schedule->flights[i].available = capacity;
        schedule->flights[i].capacity = capacity;
        success = true;
        flight_schedule_sort_flights_by_time(schedule);
        break;
      }
    }
      
    if (success == false){
      msg_city_max_flights_reached(city);
    }
}
  }
}

//Function 8:removes a flight from the input city given
void flight_schedule_remove_flight(city_t city){
struct flight_schedule * schedule = flight_schedule_find(city);
if (schedule == NULL){
  msg_city_bad(city);
}


else{
  time_t time;
  bool success = false;
  if (time_get(&time)){
  for (int i=0;i<MAX_FLIGHTS_PER_CITY;i++){

    if (schedule->flights[i].time == time){
    success = true;
    schedule->flights[i].time = TIME_NULL;
    schedule->flights[i].available = 0;
    schedule->flights[i].capacity = 0;
    flight_schedule_sort_flights_by_time(schedule);
    break;
    }
  }
  if (success == false){
    msg_flight_bad_time();
  }
}
}

}

//Function 9:Schedules a seat to be used up for a 
//flight to a specific input city
void flight_schedule_schedule_seat(city_t city){
struct flight_schedule * schedule= flight_schedule_find(city);
if (schedule == NULL){
  msg_city_bad(city);
}else {
  time_t time;
  bool success=false;
  if (time_get(&time)){
  for (int x = 0; x < MAX_FLIGHTS_PER_CITY; x++){
    if (time - schedule ->flights[x].time <=0){
      schedule -> flights[x].available -=1;
      success = true;
      if (schedule->flights[x].available == -1){
      schedule -> flights[x].available =0;
      msg_flight_no_seats();
      }
      break;
    }
  }
  }
  if (success == false){
    msg_flight_no_seats();
  }
}

}

//Function 10:Frees up a seat for a 
//flight to a specific input city
void flight_schedule_unschedule_seat(city_t city){
struct flight_schedule * schedule= flight_schedule_find(city);
if (schedule == NULL){
  msg_city_bad(city);
}else {
  time_t time;
  bool success=false;
  if (time_get(&time)){
  for (int x = 0; x < MAX_FLIGHTS_PER_CITY; x++){
    if (time == schedule->flights[x].time){
      schedule -> flights[x].available +=1;
      success = true;
      if (schedule->flights[x].available == schedule->flights[x].capacity+1){
      schedule -> flights[x].available = schedule->flights[x].capacity;
      msg_flight_all_seats_empty();
      }
      break;
    }
  }
  }
  if (success == false){
    msg_flight_bad_time();
  }
}


}


//Function 11:Looks for input city in active flight list
// and returns the flight schedule for this city
struct flight_schedule * flight_schedule_find(city_t city){
struct flight_schedule * active = flight_schedules_active;
while (active != NULL)
{
  if (strncmp(city, active->destination, MAX_CITY_NAME_LEN) == 0){
    break;
  }
  active = active->next;
}
return active;
}