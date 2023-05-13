#ifndef PROVIDED_ISRAELIQUEUE_H
#define PROVIDED_ISRAELIQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FRIEND_QUOTA 5
#define RIVAL_QUOTA 3

typedef struct IsraeliQueue_t * IsraeliQueue;

typedef int (*FriendshipFunction)(void*,void*);
typedef int (*ComparisonFunction)(void*,void*);

typedef enum { ISRAELIQUEUE_SUCCESS, ISRAELIQUEUE_ALLOC_FAILED, ISRAELIQUEUE_BAD_PARAM, ISRAELI_QUEUE_ERROR } IsraeliQueueError;

/**Error clarification:
 * ISRAELIQUEUE_SUCCESS: Indicates the function has completed its task successfully with no errors.
 * ISRAELIQUEUE_ALLOC_FAILED: Indicates memory allocation failed during the execution of the function.
 * ISRAELIQUEUE_BAD_PARAM: Indicates an illegal parameter was passed.
 * ISRAELI_QUEUE_ERROR: Indicates any error beyond the above.
 * */

/**Creates a new IsraeliQueue_t object with the provided friendship functions, a NULL-terminated array,
 * comparison function, friendship threshold and rivalry threshold. Returns a pointer
 * to the new object. In case of failure, return NULL.*/
IsraeliQueue IsraeliQueueCreate(FriendshipFunction *, ComparisonFunction, int, int);

/**Returns a new queue with the same elements as the parameter. If the parameter is NULL or any error occured during
 * the execution of the function, NULL is returned.*/
IsraeliQueue IsraeliQueueClone(IsraeliQueue q);

/**@param IsraeliQueue: an IsraeliQueue created by IsraeliQueueCreate
 *
 * Deallocates all memory allocated by IsraeliQueueCreate for the object pointed to by
 * the parameter.*/
void IsraeliQueueDestroy(IsraeliQueue);

/**@param IsraeliQueue: an IsraeliQueue in which to insert the item.
 * @param item: an item to enqueue
 *
 * Places the item in the foremost position accessible to it.*/
IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue, void *);

/**@param IsraeliQueue: an IsraeliQueue to which the function is to be added
 * @param FriendshipFunction: a FriendshipFunction to be recognized by the IsraeliQueue
 * going forward.
 *
 * Makes the IsraeliQueue provided recognize the FriendshipFunction provided.*/
IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue, FriendshipFunction);

/**@param IsraeliQueue: an IsraeliQueue whose friendship threshold is to be modified
 * @param friendship_threshold: a new friendship threshold for the IsraeliQueue*/
IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue, int);

/**@param IsraeliQueue: an IsraeliQueue whose rivalry threshold is to be modified
 * @param friendship_threshold: a new rivalry threshold for the IsraeliQueue*/
IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue, int);

/**Returns the number of elements of the given queue. If the parameter is NULL, 0
 * is returned.*/
int IsraeliQueueSize(IsraeliQueue);

/**Removes and returns the foremost element of the provided queue. If the parameter
 * is NULL or a pointer to an empty queue, NULL is returned.*/
void* IsraeliQueueDequeue(IsraeliQueue);

/**@param item: an object comparable to the objects in the IsraeliQueue
 *
 * Returns whether the queue contains an element equal to item. If either
 * parameter is NULL, false is returned.*/
bool IsraeliQueueContains(IsraeliQueue, void *);

/**Advances each item in the queue to the foremost position accessible to it,
 * from the back of the queue frontwards.*/
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue);

/**@param q_arr: a NULL-terminated array of IsraeliQueues
 * @param ComparisonFunction: a comparison function for the merged queue
 *
 * Merges all queues in q_arr into a single new queue, with parameters the parameters described
 * in the exercise. Each queue in q_arr enqueues its head in the merged queue, then lets the next
 * one enqueue an item, in the order defined by q_arr. In the event of any error during execution, return NULL.*/
IsraeliQueue IsraeliQueueMerge(IsraeliQueue*,ComparisonFunction);

#endif //PROVIDED_ISRAELIQUEUE_H

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#include "IsraeliQueue.h"

typedef struct israeliNode {
   void* element_ptr;
   struct israeliNode* next;
   struct israeliNode* previous;
   int friendsPassed;
   int rivalsBlocked;

} israeliNode;

typedef struct IsraeliQueue_t {
    israeliNode* head;
    israeliNode* last;
    FriendshipFunction* FriendshipFuncs;
    ComparisonFunction ComparisonFunc;
    int friendshipThreshold;
    int rivalryThreshold;
} IsraeliQueue_t;

// HELPER FUNCTIONS DECLARATIONS
bool areFriends(IsraeliQueue q, void* item1, void* item2);
bool areRivals(IsraeliQueue q, void* item1, void* item2);
israeliNode* findForemostPos(IsraeliQueue q, void* item);
israeliNode* insertItem(IsraeliQueue q, israeliNode* foremostPos, void* item);
void insertIsraeliNode(IsraeliQueue q, israeliNode* foremostPos, israeliNode* item_israeliNode);
israeliNode* findPrevious(IsraeliQueue q, israeliNode* cur);
bool isMergeDone(IsraeliQueue* qArr);
int abs(int n);
unsigned int power(int n, int a);
int findNRoot(unsigned int num, int n);
int findMergedRivalryThreshold(IsraeliQueue* qArr);

/**Error clarification:
 * ISRAELIQUEUE_SUCCESS: Indicates the function has completed its task successfully with no errors.
 * ISRAELIQUEUE_ALLOC_FAILED: Indicates memory allocation failed during the execution of the function.
 * ISRAELIQUEUE_BAD_PARAM: Indicates an illegal parameter was passed.
 * ISRAELI_QUEUE_ERROR: Indicates any error beyond the above.
 * */

/**Creates a new IsraeliQueue_t object with the provided friendship functions, a NULL-terminated array,
 * comparison function, friendship threshold and rivalry threshold. Returns a pointer
 * to the new object. In case of failure, return NULL.*/
IsraeliQueue IsraeliQueueCreate(FriendshipFunction* FriendshipFuncs, ComparisonFunction ComparisonFunc, int friendshipThreshold, int rivalryThreshold){
    if (FriendshipFuncs == NULL) return NULL; // bad parameter
    IsraeliQueue q = (IsraeliQueue)malloc(sizeof(IsraeliQueue_t));
    if (q == NULL){
        return NULL;
    }

    q->head = NULL;
    q->last = NULL;
    q->ComparisonFunc = ComparisonFunc;
    q->friendshipThreshold = friendshipThreshold;
    q->rivalryThreshold = rivalryThreshold;

    int n = 0;
    for (; FriendshipFuncs[n] != NULL; n++);

    q->FriendshipFuncs = (FriendshipFunction*)malloc((n+1)*(sizeof(FriendshipFunction)));
    if (q->FriendshipFuncs == NULL){
        return NULL;
    }

    for (int i = 0; i < n; i++){
        q->FriendshipFuncs[i] = FriendshipFuncs[i];
    }
    q->FriendshipFuncs[n] = NULL;

    return q;
}

/**Returns a new queue with the same elements as the parameter. If the parameter is NULL or any error occured during
 * the execution of the function, NULL is returned.*/
IsraeliQueue IsraeliQueueClone(IsraeliQueue q){
    if (q == NULL){
        return NULL;
    }

    IsraeliQueue qClone = IsraeliQueueCreate(q->FriendshipFuncs, q->ComparisonFunc, q->friendshipThreshold, q->rivalryThreshold);
    if (qClone == NULL){
        return NULL;
    }

    if (q->head == NULL){
        return qClone;
    }

    israeliNode* cur_israeliNode = q->head;
    israeliNode* cur_clone = (israeliNode*)malloc(sizeof(israeliNode));
    if (cur_clone == NULL){
        IsraeliQueueDestroy(qClone);
        return NULL;
    }
        qClone->head = cur_clone;
        cur_clone->element_ptr = cur_israeliNode->element_ptr;
        cur_clone->previous = NULL;
        cur_clone->next = NULL;
        cur_clone->friendsPassed = cur_israeliNode->friendsPassed;
        cur_clone->rivalsBlocked = cur_israeliNode->rivalsBlocked;

    israeliNode* new_clone;
    while (cur_israeliNode->next != NULL){
        new_clone = (israeliNode*)malloc(sizeof(israeliNode));
        if (new_clone == NULL){
            IsraeliQueueDestroy(qClone);
            return NULL;
        }
        qClone->last = new_clone;
        cur_clone->next = new_clone;
        cur_israeliNode = cur_israeliNode->next;

        new_clone->element_ptr = cur_israeliNode->element_ptr;
        new_clone->previous = cur_clone;
        new_clone->next = NULL;
        new_clone->friendsPassed = cur_israeliNode->friendsPassed;
        new_clone->rivalsBlocked = cur_israeliNode->rivalsBlocked;

        cur_clone = cur_clone->next;
    }

    return qClone;
}

/**@param IsraeliQueue: an IsraeliQueue created by IsraeliQueueCreate
 *
 * Deallocates all memory allocated by IsraeliQueueCreate for the object pointed to by
 * the parameter.*/
void IsraeliQueueDestroy(IsraeliQueue q){
    while (q->head != NULL){
        IsraeliQueueDequeue(q);
    }
    free(q->FriendshipFuncs);
    free(q);
}

bool areFriends(IsraeliQueue q, void* item1, void* item2){
    for (int i = 0; q->FriendshipFuncs[i] != NULL; i++){
        if (q->FriendshipFuncs[i](item1, item2) > q->friendshipThreshold){
            return true;
        }
    }
    return false;
}

bool areRivals(IsraeliQueue q, void* item1, void* item2){
    if (areFriends(q, item1, item2)){
        return false;
    }

    int friendshipSum = 0; int i = 0;
    for (; q->FriendshipFuncs[i] != NULL; i++){
        friendshipSum += q->FriendshipFuncs[i](item1, item2);
    }

    if (i == 0)  return false; // no friendship functions

    if (friendshipSum/i < q->rivalryThreshold){
        return true;
    }
    return false;
}

israeliNode* findForemostPos(IsraeliQueue q, void* item){
    israeliNode* friend = NULL;
    israeliNode* cur_israeliNode = q->head;
    while (cur_israeliNode != NULL){
        if (friend == NULL && areFriends(q, cur_israeliNode->element_ptr, item) && cur_israeliNode->friendsPassed < FRIEND_QUOTA){
            friend = cur_israeliNode;
        }
        if (areRivals(q, cur_israeliNode->element_ptr, item) && cur_israeliNode->rivalsBlocked < RIVAL_QUOTA){
            cur_israeliNode->rivalsBlocked++;
            friend = NULL;
        }

        cur_israeliNode = cur_israeliNode->next;
    }

    return friend;
}

israeliNode* insertItem(IsraeliQueue q, israeliNode* foremostPos, void* item){
    israeliNode* item_israeliNode = (israeliNode*)malloc(sizeof(israeliNode));
    if (item_israeliNode == NULL){
        return NULL;
    }

    item_israeliNode->element_ptr = item;
    item_israeliNode->next = NULL;
    item_israeliNode->previous = NULL;
    item_israeliNode->friendsPassed = 0;
    item_israeliNode->rivalsBlocked = 0;

    if (q->head == NULL){
        q->head = item_israeliNode;
        q->last = item_israeliNode;
    }
    else if (!foremostPos){ // doesnt exist
        q->last->next = item_israeliNode;
        item_israeliNode->previous = q->last;
        q->last = item_israeliNode;
    }
    else{
        item_israeliNode->next = foremostPos->next;
        foremostPos->next = item_israeliNode;
        item_israeliNode->previous = foremostPos;
        foremostPos->friendsPassed++;
    }

    return item_israeliNode;
}

/**@param IsraeliQueue: an IsraeliQueue in which to insert the item.
 * @param item: an item to enqueue
 *
 * Places the item in the foremost position accessible to it.*/
IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue q, void* item){
    if (q == NULL || item == NULL){
        return ISRAELIQUEUE_BAD_PARAM;
    }

    israeliNode* foremostPos = findForemostPos(q, item);
    if (insertItem(q, foremostPos, item) == NULL){
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    return ISRAELIQUEUE_SUCCESS;
}

/**@param IsraeliQueue: an IsraeliQueue to which the function is to be added
 * @param FriendshipFunction: a FriendshipFunction to be recognized by the IsraeliQueue
 * going forward.
 *
 * Makes the IsraeliQueue provided recognize the FriendshipFunction provided.*/
IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue q, FriendshipFunction newFunc){
    if (q == NULL || newFunc == NULL){
        return ISRAELIQUEUE_BAD_PARAM;
    }

    int n = 0;
    for (; q->FriendshipFuncs[n] != NULL; n++);

    FriendshipFunction* newFriendshipFuncs = (FriendshipFunction*)malloc((n+1 + 1)*(sizeof(FriendshipFunction)));
    if (newFriendshipFuncs == NULL){
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    for (int i = 0; i < n; i++){
        newFriendshipFuncs[i] = q->FriendshipFuncs[i];
    }
    newFriendshipFuncs[n] = newFunc;
    newFriendshipFuncs[n+1] = NULL;

    FriendshipFunction* tmp = q->FriendshipFuncs;
    free(tmp);
    q->FriendshipFuncs = newFriendshipFuncs;

    return ISRAELIQUEUE_SUCCESS;
}

/**@param IsraeliQueue: an IsraeliQueue whose friendship threshold is to be modified
 * @param friendship_threshold: a new friendship threshold for the IsraeliQueue*/
IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue q, int friendshipThreshold){
    if (q == NULL) return   ISRAELIQUEUE_BAD_PARAM;
    q->friendshipThreshold = friendshipThreshold;

    return ISRAELIQUEUE_SUCCESS;
}

/**@param IsraeliQueue: an IsraeliQueue whose rivalry threshold is to be modified
 * @param friendship_threshold: a new rivalry threshold for the IsraeliQueue*/
IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue q, int rivalryThreshold){
    if (q == NULL) return   ISRAELIQUEUE_BAD_PARAM;
    q->rivalryThreshold = rivalryThreshold;

    return ISRAELIQUEUE_SUCCESS;
}

/**Returns the number of elements of the given queue. If the parameter is NULL, 0
 * is returned.*/
int IsraeliQueueSize(IsraeliQueue q){
    if (q == NULL || (q->head) == NULL)  return 0;

    israeliNode* tmp = q->head;
    int n = 1;
    while(tmp->next){
        n++;
    }
    return n;
}

/**Removes and returns the foremost element of the provided queue. If the parameter
 * is NULL or a pointer to an empty queue, NULL is returned.*/
void* IsraeliQueueDequeue(IsraeliQueue q){
    if (q == NULL || (q->head) == NULL)  return NULL;

    void* tmp = q->head->element_ptr;
    israeliNode* tmpIsraeliNode = q->head;

    q->head = tmpIsraeliNode->next;
    if (q->head != NULL)    q->head->previous = NULL;
    free(tmpIsraeliNode);
    return tmp;
}

/**@param item: an object comparable to the objects in the IsraeliQueue
 *
 * Returns whether the queue contains an element equal to item. If either
 * parameter is NULL, false is returned.*/
bool IsraeliQueueContains(IsraeliQueue q, void * element){
    if (q == NULL || element == NULL)  return false;

    int same = q->ComparisonFunc(element, element);
    israeliNode* cur = q->head;
    while (q->head != NULL && cur->next != NULL){
        if (q->ComparisonFunc(cur->element_ptr, element) == same){
            return true;
        }
    }

    return false;
}


void insertIsraeliNode(IsraeliQueue q, israeliNode* foremostPos, israeliNode* item_israeliNode){
    if (q->head == NULL){
        q->head = item_israeliNode;
    }

    if (!foremostPos){
        q->last->next = item_israeliNode;
        item_israeliNode->previous = q->last;
        item_israeliNode->next = NULL;
        q->last = item_israeliNode;
    }
    else{
        item_israeliNode->next = foremostPos->next;
        foremostPos->next = item_israeliNode;
        item_israeliNode->previous = foremostPos;
        foremostPos->friendsPassed++;
    }
}

israeliNode* findPrevious(IsraeliQueue q, israeliNode* cur){
    if (!q || !cur || !(q->head)) return NULL; // bad parameters

    if (cur == q->head) return NULL; // head previous is NULL
    israeliNode* node = q->head;
    while (node){
        if (node->next == cur){
            return node;
        }
        node = node->next;
    }

    return NULL; // shouldn't happen
}

/**Advances each item in the queue to the foremost position accessible to it,
 * from the back of the queue frontwards.*/
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q){
    if (q == NULL) return   ISRAELIQUEUE_BAD_PARAM;
    if (q->head == NULL){
        return ISRAELIQUEUE_SUCCESS;
    }

    israeliNode* cur = q->last;
    israeliNode* previousOG;
    israeliNode* curPrevious;

    israeliNode* foremostPos;
    while (cur){
        previousOG = cur->previous;
        curPrevious = findPrevious(q, cur);
        // remove cur from queue
            if (curPrevious != NULL){
                curPrevious->next = cur->next;
            }
            else{ // cur is head
                q->head = cur->next;
            }
            if (cur->next != NULL){
                cur->next->previous = cur->previous;
            }
            else{ // cur is last
                q->last = cur->previous;
            }
        // enque them again
            foremostPos = findForemostPos(q, cur->element_ptr);
            insertIsraeliNode(q, foremostPos, cur);
            cur = previousOG;
    }

    return ISRAELIQUEUE_SUCCESS;
}

bool isMergeDone(IsraeliQueue* qArr){
    for (int i = 0; qArr[i] != NULL; i++){
        if (qArr[i]->head != NULL){
            return false;
        }
    }

    return true;
}

int abs(int n){
    if(n<0) return -n;
    return n;
}
unsigned int power(int n, int a){
    unsigned res=n;
    if(a==0){
        return 1;
    }
    for(int i=0;i<(a-1);i++){
        res*=n;
    }
    return res;
}
int findNRoot(unsigned int num, int n){
    int res=0;
    while(power(res,n)<num){
        res++;
    }
    return res;
}
int findMergedRivalryThreshold(IsraeliQueue* qArr){
    unsigned int multiplication=1;
    int i=0;
    for(;qArr[i];i++){
        multiplication*=(unsigned int)abs(qArr[i]->rivalryThreshold);
    }
    return findNRoot(multiplication, i);
}


/**@param q_arr: a NULL-terminated array of IsraeliQueues
 * @param ComparisonFunction: a comparison function for the merged queue
 *
 * Merges all queues in q_arr into a single new queue, with parameters the parameters described
 * in the exercise. Each queue in q_arr enqueues its head in the merged queue, then lets the next
 * one enqueue an item, in the order defined by q_arr. In the event of any error during execution, return NULL.*/
IsraeliQueue IsraeliQueueMerge(IsraeliQueue* qArr, ComparisonFunction ComparisonFunc){
    if (qArr == NULL || ComparisonFunc == NULL) return   NULL;

    // Array Length
        int n = 0;
        for (; qArr[n] != NULL; n++);

    // Friendship functions
        int n_funcs = 0;
        for (int i = 0; qArr[i] != NULL; i++){
            for (int j = 0; qArr[i]->FriendshipFuncs[j] != NULL; j++){
                n_funcs++;
            }
        }

        FriendshipFunction* FriendshipFuncs = (FriendshipFunction*)malloc((n_funcs+1)*(sizeof(FriendshipFunction)));
        if (FriendshipFuncs == NULL){
            return NULL;
        }

        int cur_func = 0;
        for (int i = 0; qArr[i] != NULL; i++){
            for (int j = 0; qArr[i]->FriendshipFuncs[j] != NULL; j++){
                FriendshipFuncs[cur_func++] = qArr[i]->FriendshipFuncs[j];
            }
        }
        FriendshipFuncs[n_funcs] = NULL;

    // Friendship Threshold
        int friendshipThresholdSum = 0;
        for (int i = 0; i < n; i++){
            friendshipThresholdSum += qArr[i]->friendshipThreshold;
        }
        int friendshipThreshold = friendshipThresholdSum / n;

    // Rivalry Threshold
        int rivalryThreshold = findMergedRivalryThreshold(qArr);

    IsraeliQueue mergedQ = IsraeliQueueCreate(FriendshipFuncs, ComparisonFunc, friendshipThreshold, rivalryThreshold);
    free(FriendshipFuncs);
    if (mergedQ == NULL){
        return NULL;
    }

    israeliNode* cur;
    void* item;
    int i = 0;
    while (!isMergeDone(qArr)){
        cur = qArr[i % n]->head;
        if (cur == NULL){
            i++;
            continue;
        }
        item = IsraeliQueueDequeue(qArr[i % n]);
        if (item == NULL || IsraeliQueueEnqueue(mergedQ, item) != ISRAELIQUEUE_SUCCESS){
            IsraeliQueueDestroy(mergedQ);
            return NULL;
        }
        i++;
    }

    return mergedQ;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

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

EnrollmentSystem createEnrollment(FILE *students, FILE *courses, FILE *hackers);

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

HackEnrollmentError hackEnrollment(EnrollmentSystem sys, FILE* out);

void destroyEnrollment(EnrollmentSystem sys);

#endif //HACKENROLLMENT_H

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#include "HackEnrollment.h"
#include "IsraeliQueue.h"

// STRUCTS
typedef struct Node
{
    void *element_ptr;
    struct Node *next;
} Node;

typedef struct Queue_t{
    Node* head;
    Node* last;
} Queue_t;

typedef struct Queue_t *Queue;

typedef struct Hacker{
    /*  <Student ID> \n
        <Course Numbers>*\n //Desired courses
        <Student ID>*\n //Friends
        <Student ID>*\n //Rivals
    */
    Queue desiredCoursesNums;
    Queue friendsIDs;
    Queue rivalsIDs;
} Hacker;

typedef struct Student
{
    // <Student ID> <Total Credits> <GPA> <Name> <Surname> <City> <Department>\n
    long studentID;
    int totalCredits;
    int gpa;
    char *name;
    char *surname;
    char *city;
    char *department;
    Hacker *hackerAlt;
} Student;

typedef struct Course{
    // <Course Number> <Size>\n
    long courseNum;
    int size;
    IsraeliQueue courseQueue;
} Course;

typedef struct EnrollmentSystem_t
{
    // students nodes (pointer to hackers)
    Queue studentsQueue;
    // courses nodes + queuesB
    Queue coursesQueue;
    // queues nodes (pointer to every Israeli Queue)
} EnrollmentSystem_t;

typedef struct EnrollmentSystem_t *EnrollmentSystem;


typedef enum { STUDENTS_Q, COURSES_Q, DEFAULT_Q } QueueType;


// QUEUE
Node* createNode(void* item){
    Node* node_ptr = (Node*)malloc(sizeof(Node));
    if (node_ptr == NULL){
        return NULL;
    }

    node_ptr->element_ptr = item;
    node_ptr->next = NULL;

    return node_ptr;
}

HackEnrollmentError enqueue(Queue q, void* item){
    Node* node = createNode(item);
    if (node == NULL){
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    if (q->head == NULL){
        q->head = node;
        q->last = node;
    }
    else{
        q->last->next = node;
        q->last = node;
    }

    return HACKENROLLMENT_SUCCESS;
}

Node* dequeueQueue(Queue q, QueueType typeQ);
Node* destroyQueue(Queue q, QueueType typeQ);

Node* dequeueQueue(Queue q, QueueType typeQ){
    if (q == NULL || (q->head) == NULL)  return NULL;

    Node* tmp = q->head;
    q->head = tmp->next;

    if (typeQ == STUDENTS_Q){
        Student* student = (Student*)(tmp->element_ptr);
        if (student->hackerAlt){
            destroyQueue(student->hackerAlt->desiredCoursesNums, DEFAULT_Q);
            destroyQueue(student->hackerAlt->friendsIDs, DEFAULT_Q);
            destroyQueue(student->hackerAlt->rivalsIDs, DEFAULT_Q);
        }
        free(student->hackerAlt);
        free(student->city);
        free(student->department);
        free(student->name);
        free(student->surname);
    }
    else if (typeQ == COURSES_Q){
        Course* course = (Course*)(tmp->element_ptr);
        if (course->courseQueue){
            IsraeliQueueDestroy(course->courseQueue);
        }
    }

    free(tmp->element_ptr);
    free(tmp);
    return q->head;
}

Node* destroyQueue(Queue q, QueueType typeQ){
    if (!q) return NULL;
    Node* tmp = q->head;
    while (q->head != NULL){
        tmp = dequeueQueue(q, typeQ);
    }
    free(q);
    return tmp;
}

long readStringIntoLong(FILE *file, bool* endofline_ptr){
    char digit = '0';
    while (!('0' <= (digit = fgetc(file)) && digit <= '9')){ // skip till first digit
        if (feof(file)){
            *endofline_ptr = true;
            return -1;
        }
    }
    long num = 0;
    while ('0' <= digit && digit <= '9'){
        num *= 10;
        num += digit - '0';
        digit = fgetc(file);

        if (digit == '\r' || digit == '\n'){
            *endofline_ptr = true;
            break;
        }
    }
    return num;
}

char* readWord(FILE* file, bool* endofline_ptr){
    char* word = (char*)malloc(sizeof(char));
    if (!word)  return NULL;
    int wordSize = 1;
    char c;
    while (!(('a' <= (c = fgetc(file)) && c <= 'z') || ('A' <= c && c <= 'Z'))){ // skip till first letter or end of file
        if (feof(file)){
            *endofline_ptr = true;
            return NULL;
        }
    }
    while (c != ' ') {
        word = realloc(word, (++wordSize) * sizeof(char));
        word[wordSize - 2] = c;
        c=fgetc(file);

        if (c == '\r' || c == '\n'){ // indecator to end of line
            *endofline_ptr = true;
            break;
        }
    }

    word[wordSize - 1] = '\0';
    return word;
}


// STUDENTS
Student* createStudent(FILE* students)
{
    // <Student ID> <Total Credits> <GPA> <Name> <Surname> <City> <Department>\n
    bool eol = false;
    Student* student_ptr = (Student*)malloc(sizeof(Student));
    if(!student_ptr)    return NULL;
    student_ptr->studentID = readStringIntoLong(students, &eol);
    if (eol){
        free(student_ptr);
        return NULL;
    }
    eol = false;
    student_ptr->totalCredits = (int)readStringIntoLong(students, &eol);
    eol = false;
    student_ptr->gpa = (int)readStringIntoLong(students, &eol);
    eol = false;
    student_ptr->name = readWord(students, &eol);
    if(!student_ptr->name)  return NULL;
    eol = false;
    student_ptr->surname = readWord(students, &eol);
    if(!student_ptr->surname)  return NULL;
    eol = false;
    student_ptr->city = readWord(students, &eol);
    if(!student_ptr->city)  return NULL;
    eol = false;
    student_ptr->department = readWord(students, &eol);
    if(!student_ptr->department)  return NULL;
    student_ptr->hackerAlt = NULL;

    return student_ptr;
}

Queue createStudentsQueue(FILE *students){
    Queue q = (Queue)malloc(sizeof(Queue_t));
    if (q == NULL)
    {
        return NULL;
    }

    q->head = NULL;
    q->last = NULL;

    Student* cur_student;
    while(!feof(students)){
        cur_student = createStudent(students);
        if (!cur_student) break;
        if (enqueue(q, cur_student) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(q, STUDENTS_Q);
            return NULL;
        }
    }

    return q;
}

Student* findStudent(Queue students, Node* startingPos, long studentID){
    Node* curNode = startingPos;
    Student* curStudent;
    while (curNode != NULL){
        curStudent = curNode->element_ptr;
        if (curStudent == NULL)  return NULL; // error
        if (curStudent->studentID == studentID){
            return curStudent;
        }
        curNode = curNode->next;
    }

    return NULL;
}


// HACKERS
Queue createHackerCoursesQueue(Student* hacker, FILE* hackers){
    Queue desiredQueuesNums = (Queue)malloc(sizeof(Queue_t));
    if (desiredQueuesNums == NULL){
        return NULL;
    }

    desiredQueuesNums->head = NULL;
    desiredQueuesNums->last = NULL;

    long* courseNum_ptr;
    bool endofline = false;
    while (!feof(hackers)){
        courseNum_ptr = (long*)malloc(sizeof(long));
        *courseNum_ptr = readStringIntoLong(hackers, &endofline);
        if (enqueue(desiredQueuesNums, courseNum_ptr) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(desiredQueuesNums, COURSES_Q);
            return NULL;
        }
        if (endofline)  break;
    }

    return desiredQueuesNums;
}

Queue createHackerFriends(Student* hacker, FILE* hackers){
    Queue friendsIDs = (Queue)malloc(sizeof(Queue_t));
    if (friendsIDs == NULL){
        return NULL;
    }

    friendsIDs->head = NULL;
    friendsIDs->last = NULL;

    long* friendID_ptr;
    bool endofline = false;
    while (!feof(hackers)){
        friendID_ptr = (long*)malloc(sizeof(long));
        *friendID_ptr = readStringIntoLong(hackers, &endofline);
        if (enqueue(friendsIDs, friendID_ptr) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(friendsIDs, DEFAULT_Q);
            return NULL;
        }
        if (endofline)  break;
    }

    return friendsIDs;
}

Queue createHackerRivals(Student* hacker, FILE* hackers){
    Queue rivalsIDs = (Queue)malloc(sizeof(Queue_t));
    if (rivalsIDs == NULL){
        return NULL;
    }

    rivalsIDs->head = NULL;
    rivalsIDs->last = NULL;

    long* rivalID_ptr;
    bool endofline = false;
    while (!feof(hackers)){
        rivalID_ptr = (long*)malloc(sizeof(long));
        *rivalID_ptr = readStringIntoLong(hackers, &endofline);
        if (enqueue(rivalsIDs, rivalID_ptr) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(rivalsIDs, DEFAULT_Q);
            return NULL;
        }
        if (endofline)  break;
    }

    return rivalsIDs;
}

HackEnrollmentError fillHackerInfo(Student* hacker, FILE* hackers){
    if (!hacker || !hackers) return HACKENROLLMENT_BAD_PARAM;

    hacker->hackerAlt = (Hacker*)malloc(sizeof(Hacker));
    if (hacker->hackerAlt == NULL){
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    hacker->hackerAlt->desiredCoursesNums = createHackerCoursesQueue(hacker, hackers);
    if (hacker->hackerAlt->desiredCoursesNums == NULL){
        free(hacker->hackerAlt);
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    hacker->hackerAlt->friendsIDs = createHackerFriends(hacker, hackers);
    if (hacker->hackerAlt->friendsIDs == NULL){
        free(hacker->hackerAlt);
        destroyQueue(hacker->hackerAlt->desiredCoursesNums, DEFAULT_Q);
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    hacker->hackerAlt->rivalsIDs = createHackerRivals(hacker, hackers);
    if (hacker->hackerAlt->rivalsIDs == NULL){
        free(hacker->hackerAlt);
        destroyQueue(hacker->hackerAlt->desiredCoursesNums, DEFAULT_Q);
        destroyQueue(hacker->hackerAlt->friendsIDs, DEFAULT_Q);
        return HACKENROLLMENT_ALLOC_FAILED;
    }

    return HACKENROLLMENT_SUCCESS;
}

HackEnrollmentError insertHackersInfo(Queue students, FILE* hackers){
    if (!students || !hackers) return HACKENROLLMENT_BAD_PARAM;
    long hackerID;
    Student* hacker;
    bool eol = false;
    while (!feof(hackers)){
        hackerID = readStringIntoLong(hackers, &eol);
        if (eol && hackerID == -1) break;
        eol = false;
        hacker = (Student*)findStudent(students, students->head, hackerID);
        /* add a NULL check even though it shouldnt occur
        if(!hacker)     return HACKENROLLMENT_BAD_PARAM;*/ 
        if (fillHackerInfo(hacker, hackers) == HACKENROLLMENT_ALLOC_FAILED){
            return HACKENROLLMENT_ALLOC_FAILED;
        }
    }

    return HACKENROLLMENT_SUCCESS;
}


// COURSES
Course* createCourse(FILE *courses){
    bool eol = false;
    Course *course_ptr = (Course *)malloc(sizeof(Course));
    if (course_ptr == NULL){
        return NULL;
    }
    course_ptr->courseNum = readStringIntoLong(courses, &eol);
    if (eol){
        free(course_ptr);
        return NULL;
    }
    eol = false;
    course_ptr->size = (int)readStringIntoLong(courses, &eol);
    course_ptr->courseQueue = NULL;

    return course_ptr;
}

Queue createCoursesQueue(FILE *courses){
    Queue q = (Queue)malloc(sizeof(Queue_t));
    if (q == NULL){
        return NULL;
    }

    q->head = NULL;
    q->last = NULL;

    Course* cur_course;
    while (!feof(courses)){
        cur_course = createCourse(courses);
        if (!cur_course) break;
        if (enqueue(q, cur_course) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(q, COURSES_Q);
            return NULL;
        }
    }

    return q;
}

Course* findCourse(Queue courses, Node* startingPos, long courseNum){
    Node* curNode = startingPos;
    Course* curCourse;
    while (curNode != NULL){
        curCourse = curNode->element_ptr;
        if (curCourse == NULL)  return NULL; // error
        if (curCourse->courseNum == courseNum){
            return (Course*)(curNode->element_ptr);
        }
        curNode = curNode->next;
    }

    return NULL;
}


// Helper Functions
Node* findStudentHacker(Queue students, Node* startingPos){
    Node* curNode = startingPos;
    Student* curStudent;
    while (curNode != NULL && curNode->element_ptr != NULL){
        curStudent = (Student*)(curNode->element_ptr);
        if (curStudent->hackerAlt){
            return curNode;
        }
        curNode = curNode->next;
    }

    return NULL;
}

HackEnrollmentError ImproveHackerPositions(EnrollmentSystem sys){
    if (!sys || !(sys->coursesQueue) || !(sys->studentsQueue))    return HACKENROLLMENT_BAD_PARAM;
    Node* curCourseNumNode;
    long curCourseNum;
    Course* curCourse;
    Node* curStudentNode = findStudentHacker(sys->studentsQueue, sys->studentsQueue->head);;
    Student* curStudent;
    Hacker* curHacker;
        while(curStudentNode != NULL && curStudentNode->element_ptr != NULL){
            // find and improve positions of hackers by using enqueue on them only
                curStudent = (Student*)(curStudentNode->element_ptr);
            // for Hacker's desired courses
                curHacker = (Hacker*)(curStudent->hackerAlt);
                if (!curHacker || !(curHacker->desiredCoursesNums) || !(curHacker->friendsIDs) || !(curHacker->rivalsIDs))
                    return HACKENROLLMENT_ERROR;
                curCourseNumNode = curHacker->desiredCoursesNums->head;
                while (curCourseNumNode != NULL && curCourseNumNode->element_ptr != NULL){
                    curCourseNum = *((long*)(curCourseNumNode->element_ptr));
                    curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
                    if (curCourse ==NULL || curCourse->courseQueue == NULL || IsraeliQueueEnqueue(curCourse->courseQueue, curStudent) != ISRAELIQUEUE_SUCCESS){
                        return HACKENROLLMENT_ERROR;
                    }
                    curCourseNumNode = curCourseNumNode->next;
                }
                curStudentNode = findStudentHacker(sys->studentsQueue, curStudentNode->next);
        }

        return HACKENROLLMENT_SUCCESS;
}

bool isEnrolled(IsraeliQueue courseQueue, int courseSize, Student* wanted){
    Student* student = (Student*)IsraeliQueueDequeue(courseQueue);
    courseSize--;
    while (student && courseSize > 0){
        if (student == wanted){
            return true;
        }
        student = (Student*)IsraeliQueueDequeue(courseQueue);
        courseSize--;
    }

    return false;
}

HackEnrollmentError countEnrollment(EnrollmentSystem sys, Student* student, int* enroll_counter_ptr){
    if (!sys || !student) return HACKENROLLMENT_BAD_PARAM;
    if (!student->hackerAlt || !student->hackerAlt->desiredCoursesNums) return HACKENROLLMENT_ERROR;

    Node* curCourseNumNode = student->hackerAlt->desiredCoursesNums->head;
    long curCourseNum;
    Course* curCourse;
    IsraeliQueue curCourseQueue_cpy;

    while (curCourseNumNode != NULL || *(enroll_counter_ptr) < 2){
        curCourseNum = *((long*)(curCourseNumNode->element_ptr));
        curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
            if (curCourse == NULL)  return HACKENROLLMENT_ERROR;

        curCourseQueue_cpy = IsraeliQueueClone(curCourse->courseQueue);
            if (curCourseQueue_cpy == NULL)  return HACKENROLLMENT_ALLOC_FAILED;

        if (isEnrolled(curCourseQueue_cpy, (int)(curCourse->size), student)){
            (*(enroll_counter_ptr))++;
        }
        IsraeliQueueDestroy(curCourseQueue_cpy);
        curCourseNumNode = curCourseNumNode->next;
    }
    return HACKENROLLMENT_SUCCESS;
}

HackEnrollmentError findDissatisfied(EnrollmentSystem sys, long* hackerID_ptr){
    if (sys == NULL || sys->studentsQueue == NULL || sys->coursesQueue == NULL){
        return HACKENROLLMENT_BAD_PARAM;
    }

    // VARIABLES
        // HACKER
        int enroll_counter = 0;
        Node* hackerNode = findStudentHacker(sys->studentsQueue, sys->studentsQueue->head);
        Student* hacker; Hacker* hacker_alt;
        // COURSE
        Node* curCourseNumNode; long curCourseNum; Course* curCourse; IsraeliQueue curCourseQueue_cpy;

    // FIND IF ALL HACKERS ARE SATISFIED
    while (hackerNode != NULL){
        if (hackerNode->element_ptr != NULL) return HACKENROLLMENT_ERROR;
        // find hacker
            hacker = (Student*)(hackerNode->element_ptr);
            hacker_alt = hacker->hackerAlt;
            if (hacker_alt == NULL || hacker_alt->desiredCoursesNums == NULL)
                return HACKENROLLMENT_BAD_PARAM;
        // going over the desired courses one by one checking enrollment
            if (countEnrollment(sys, hacker, &enroll_counter) != HACKENROLLMENT_SUCCESS){
                return HACKENROLLMENT_ERROR;
            }
        // check dissatisfaction
            // exception
                if (enroll_counter == 1 && hacker_alt->desiredCoursesNums->head == hacker_alt->desiredCoursesNums->last){
                    continue;
                }
            // dissatisfied
                if (enroll_counter < 2){
                    *hackerID_ptr = hacker->studentID;
                    return HACKENROLLMENT_SUCCESS;
                }
        // go to next hacker
            hackerNode = findStudentHacker(sys->studentsQueue, hackerNode);
            enroll_counter = 0;
    }

    return HACKENROLLMENT_SUCCESS;
}

HackEnrollmentError printOut(EnrollmentSystem sys, FILE* out){
    if (!sys || !out)   return HACKENROLLMENT_BAD_PARAM;
    if (!sys->coursesQueue)   return HACKENROLLMENT_ERROR;

    // VARIABLES
        // COURSE
        Node* courseNode = sys->coursesQueue->head;
        Course* curCourse;
        // STUDENT
        Node* studentNode;
        Student* student;
        long cur_studentID;

    while(courseNode){
        curCourse = (Course*)(courseNode->element_ptr);
        if (!curCourse || !(curCourse->courseNum) || !(curCourse->courseQueue))
            return HACKENROLLMENT_ERROR;

        fprintf(out, "%ld", curCourse->courseNum);
        studentNode = (Node*)(IsraeliQueueDequeue(curCourse->courseQueue));
        while(studentNode){
            student = (Student*)(studentNode->element_ptr);
            if (!student || !(student->studentID))   return HACKENROLLMENT_ERROR;
            cur_studentID = student->studentID;
            fprintf(out, " %ld", cur_studentID);
            studentNode = studentNode->next;
        }
        fprintf(out, "\n");
        courseNode = courseNode->next;
    }

    return HACKENROLLMENT_SUCCESS;
}


// Friendship
bool findStudentInIDQueue(Queue studentIDs, long wantedID){
    if (!studentIDs) return false;
    Node* curStudentIDNode = studentIDs->head;
    long* curStudentID;
    while (curStudentIDNode){
        curStudentID = (long*)(curStudentIDNode->element_ptr);
        if ((*curStudentID) == wantedID) return true;
        curStudentIDNode = curStudentIDNode->next;
    }
    return false;
}
long absL(long n){
    if (n < 0) return -n;
    return n;
}
// finds the ascii value of a word, returning -1 if bad parameter
long findStringAscii(char* s){
    if (!s) return -1;
    long value = 0;
    for (int i = 0; s[i] != '\0'; i++){
        value += s[i];
    }
    return value;
}

// finds if the students are friends according to HACKERS FILE
// returns 20 if friends, -20 if rivals, 0 if neither
int areFriendsAccordingToHacker(void* student1, void* student2){
    if (!(Student*)student1 || !(Student*)student2) return 0;
    if (((Student*)student1)->hackerAlt){
        if (findStudentInIDQueue(((Student*)student1)->hackerAlt->friendsIDs, ((Student*)student2)->studentID)){
            return 20;
        }
        else if (findStudentInIDQueue(((Student*)student1)->hackerAlt->rivalsIDs, ((Student*)student2)->studentID)){
            return -20;
        }
    }
    else if (((Student*)student2)->hackerAlt){
        if (findStudentInIDQueue(((Student*)student2)->hackerAlt->friendsIDs, ((Student*)student1)->studentID)){
            return 20;
        }
        else if (findStudentInIDQueue(((Student*)student2)->hackerAlt->rivalsIDs, ((Student*)student1)->studentID)){
            return -20;
        }
    }

    return 0;
}

// finds the difference between the ASCII values of the names of the two students
// the difference is >= 0, returns -1 if bad parameters
int findNameAsciiDifference(void* student1, void* student2){
    if (!student1 || !student2) return -1;
    long value1 = findStringAscii(((Student*)student1)->name);
    long value2 = findStringAscii(((Student*)student2)->name);
    if (value1 != -1 || value2 != -1) return -1;

    return ((int)(absL(value1 - value2)));
}

// finds the difference between the IDs of the two students
int findIDDifference(void* student1, void* student2){
    if (!(Student*)student1 || !(((Student*)student1)->studentID) || !(Student*)student2 || !(((Student*)student2)->studentID)) return -1;
    return ((int)absL((((Student*)student1)->studentID) - (((Student*)student2)->studentID))); // |id1 - id2|
}


// Functions
EnrollmentSystem createEnrollment(FILE *students, FILE *courses, FILE *hackers){
    // Create a new EnrollmentSystem
    EnrollmentSystem enrollment = (EnrollmentSystem)malloc(sizeof(EnrollmentSystem_t));
    if (enrollment == NULL){
        return NULL;
    }

    // Initialize and Fill up the students and courses queues
    enrollment->studentsQueue = createStudentsQueue(students);
    if (enrollment->studentsQueue == NULL){
        return NULL;
    }
    enrollment->coursesQueue = createCoursesQueue(courses);
    if (enrollment->coursesQueue == NULL){
        destroyQueue(enrollment->studentsQueue, STUDENTS_Q);
        return NULL;
    }

    // Fill up hackers details
    if (insertHackersInfo(enrollment->studentsQueue, hackers) == HACKENROLLMENT_ALLOC_FAILED){
        destroyQueue(enrollment->studentsQueue, STUDENTS_Q);
        destroyQueue(enrollment->coursesQueue, STUDENTS_Q);
    }

    return enrollment;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues){
    if (!sys || !(sys->coursesQueue) || !(sys->studentsQueue) || !queues){
        destroyEnrollment(sys);
        return NULL;
    }
    Course* curCourse;
    bool eol = false;
    FriendshipFunction fArr[] = {NULL};
    Student* curStudent;
    long curCourseNum, cur_studentID;
    while(!feof(queues)){
        curCourseNum = readStringIntoLong(queues, &eol);
        if (eol && curCourseNum == -1) break;
        eol = false;
        curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
        if(!curCourse)  return NULL;
        curCourse->courseQueue = IsraeliQueueCreate(fArr, NULL, FRIENDHIP_THRESHLOD, RIVALRY_THRESHLOD);
        if (curCourse->courseQueue == NULL){
            destroyEnrollment(sys);
            return NULL;
        }
        while(!eol){
            cur_studentID = readStringIntoLong(queues, &eol);
            if (eol && cur_studentID == -1) break;
            curStudent = findStudent(sys->studentsQueue, sys->studentsQueue->head, cur_studentID);
            if (IsraeliQueueEnqueue(curCourse->courseQueue, curStudent) != ISRAELIQUEUE_SUCCESS){
                destroyEnrollment(sys);
                return NULL;
            }

        }
        eol = false;
        if (IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, areFriendsAccordingToHacker) != ISRAELIQUEUE_SUCCESS
        ||  IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, findNameAsciiDifference)
        ||  IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, findIDDifference)){
                destroyEnrollment(sys);
                return NULL;
        }


    }
    return sys;
}

HackEnrollmentError hackEnrollment(EnrollmentSystem sys, FILE* out){
    // improve positions for HACKERS
    if (ImproveHackerPositions(sys) != HACKENROLLMENT_SUCCESS){
        return HACKENROLLMENT_ERROR;
    }

    // check if ALL HACKERS are satisfied
    long dissatisfiedHackerID = -1;
    if(findDissatisfied(sys, &dissatisfiedHackerID) != HACKENROLLMENT_SUCCESS){
        return HACKENROLLMENT_ERROR;
    }
    else if (dissatisfiedHackerID != -1){
        fprintf(out, "Cannot satisfy constraints for %ld", dissatisfiedHackerID);
    }

    // print result
    if (printOut(sys, out) != HACKENROLLMENT_SUCCESS){
        return HACKENROLLMENT_ERROR;
    }

    return HACKENROLLMENT_SUCCESS;
}

void destroyEnrollment(EnrollmentSystem sys){
    if (!sys) return;
    destroyQueue(sys->coursesQueue, COURSES_Q);
    destroyQueue(sys->studentsQueue, STUDENTS_Q);
    free(sys);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

int main(){
    FILE* students = fopen("tests/Test26/students.txt", "r");
    FILE* courses = fopen("tests/Test26/courses.txt", "r");
    FILE* hackers = fopen("tests/Test26/hackers.txt", "r");
    FILE* queues = fopen("tests/Test26/queues.txt", "r");
    FILE* out = fopen("out.txt", "w");
    if (!students || !courses || !hackers || !queues || !out){
        printf("couldn't open files\n");
        return 1;
    }

    EnrollmentSystem sys = createEnrollment(students, courses, hackers);
    if (sys == NULL)
        return 2;

    sys = readEnrollment(sys, queues);
    if (sys == NULL)
        return 3;

    if (hackEnrollment(sys, out) != HACKENROLLMENT_SUCCESS){
        printf("hackEnrollment ERROR\n");
    }

    destroyEnrollment(sys);

	return 0;
}