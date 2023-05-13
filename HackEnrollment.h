#ifndef HACKENROLLMENT_H
#define HACKENROLLMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FRIENDHIP_THRESHLOD 20
#define RIVALRY_THRESHLOD 0

struct EnrollmentSystem_t;
typedef struct EnrollmentSystem_t * EnrollmentSystem;

typedef enum { HACKENROLLMENT_SUCCESS, HACKENROLLMENT_ALLOC_FAILED, HACKENROLLMENT_BAD_PARAM, HACKENROLLMENT_ERROR } HackEnrollmentError;
/* ERROR CLARIFICARION:
 * HACKENROLLMENT_SUCCESS: Indicates the function has completed its task successfully with no errors.
 * HACKENROLLMENT_ALLOC_FAILED: Indicates memory allocation failed during the execution of the function.
 * HACKENROLLMENT_BAD_PARAM: Indicates an illegal parameter was passed.
 * HACKENROLLMENT_ERROR: Indicates any error beyond the above.
 * */

/*
creates a new EnrollmentSystem_t object containing in it the Students and Courses provided by the Students and Courses Files.
returns the pointer of the object.
In case of failure, returns NULL.
*/
EnrollmentSystem createEnrollment(FILE *students, FILE *courses, FILE *hackers);


/*
updates a given EnrollmentSystem_t (provided by its pointer) with the enrollment queues for the courses provided previously by the Courses File.
returns the pointer for the object.
In case of failure, frees the object, preventing memory leakage, and returns NULL.
*/
EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

/*
writes to the Out File the new enrollment queues for the courses provided previously by the Courses File, with every hacker enrolled to at least two of the courses he asked for (located within <Size). Otherwise, the message “Cannot satisfy constraints for <Student ID> ” is printed out, with  <Student ID> being the ID of the first unsatisfied hacker according to the Hackers File.
The hackers are added to each course according to:
1. One by One according to their order in the Hackers File
2. Each hacker is enqueued (In the way of the Israeli Queues) to all the courses he asked for before moving to the next hacker.
*/
HackEnrollmentError hackEnrollment(EnrollmentSystem sys, FILE* out);

/*
destroys a given EnrollmentSystem_t (provided by its pointer)
*/
void destroyEnrollment(EnrollmentSystem sys);

#endif //HACKENROLLMENT_H