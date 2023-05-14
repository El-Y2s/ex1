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
IsraeliQueueError insertIsraeliNode(IsraeliQueue q, israeliNode* foremostPos, israeliNode* item_israeliNode);
israeliNode* findPrevious(IsraeliQueue q, israeliNode* cur);
bool isMergeDone(IsraeliQueue* qArr);
int abs(int n);
unsigned int power(int n, int exp);
int findNRoot(unsigned int num, int n);
int findMergedFriendshipThreshold(IsraeliQueue* qArr);
int findMergedRivalryThreshold(IsraeliQueue* qArr);

/**Creates a new IsraeliQueue_t object with the provided friendship functions, a NULL-terminated array,
 * comparison function, friendship threshold and rivalry threshold. Returns a pointer
 * to the new object. In case of failure, return NULL.*/
IsraeliQueue IsraeliQueueCreate(FriendshipFunction* FriendshipFuncs, ComparisonFunction ComparisonFunc, int friendshipThreshold, int rivalryThreshold){
    if (FriendshipFuncs == NULL) return NULL; // bad parameter
    IsraeliQueue q = (IsraeliQueue)malloc(sizeof(IsraeliQueue_t));
    if (q == NULL) return NULL;

    q->head = NULL;
    q->last = NULL;
    q->ComparisonFunc = ComparisonFunc;
    q->friendshipThreshold = friendshipThreshold;
    q->rivalryThreshold = rivalryThreshold;

    int n = 0;
    for (; FriendshipFuncs[n] != NULL; n++);

    q->FriendshipFuncs = (FriendshipFunction*)malloc((n+1)*(sizeof(FriendshipFunction)));
    if (q->FriendshipFuncs == NULL){
        free(q);
        return NULL;
    }

    for (int i = 0; i < n; i++){
        q->FriendshipFuncs[i] = FriendshipFuncs[i];
    }
    q->FriendshipFuncs[n] = NULL; // NULL terminated array

    return q;
}

/**Returns a new queue with the same elements as the parameter. If the parameter is NULL or any error occured during
 * the execution of the function, NULL is returned.*/
IsraeliQueue IsraeliQueueClone(IsraeliQueue q){
    if (q == NULL) return NULL;

    FriendshipFunction fArr[] = { NULL };
    IsraeliQueue qClone = IsraeliQueueCreate(fArr, q->ComparisonFunc, q->friendshipThreshold, q->rivalryThreshold);
    if (qClone == NULL) return NULL; // error

    israeliNode* cur_israeliNode = q->head;
    while (cur_israeliNode){
        if (IsraeliQueueEnqueue(qClone, cur_israeliNode->element_ptr) != ISRAELIQUEUE_SUCCESS){
            IsraeliQueueDestroy(qClone);
            return NULL;
        }
        // copy data
        qClone->last->friendsPassed = cur_israeliNode->friendsPassed;
        qClone->last->rivalsBlocked = cur_israeliNode->rivalsBlocked;

        cur_israeliNode = cur_israeliNode->next;
    }

    // add functions
    for (int i = 0; (q->FriendshipFuncs)[i] != NULL; i++){
        if (IsraeliQueueAddFriendshipMeasure(qClone, (q->FriendshipFuncs)[i]) != ISRAELIQUEUE_SUCCESS){
            IsraeliQueueDestroy(qClone);
            return NULL;
        }
    }

    return qClone;
}

/**@param IsraeliQueue: an IsraeliQueue created by IsraeliQueueCreate
 *
 * Deallocates all memory allocated by IsraeliQueueCreate for the object pointed to by
 * the parameter.*/
void IsraeliQueueDestroy(IsraeliQueue q){
    if (!q) return; // already destroyed
    while (q->head != NULL){
        IsraeliQueueDequeue(q);
    }
    if (!(q->FriendshipFuncs))  free(q->FriendshipFuncs);
    free(q);
}

bool areFriends(IsraeliQueue q, void* item1, void* item2){
    if (!q || !(q->FriendshipFuncs) || !item1 || !item2) return false; // bad parameters

    for (int i = 0; q->FriendshipFuncs[i] != NULL; i++){
        if (q->FriendshipFuncs[i](item1, item2) > q->friendshipThreshold){
            return true;
        }
    }
    return false;
}

bool areRivals(IsraeliQueue q, void* item1, void* item2){
    if (!q || !(q->FriendshipFuncs) || !item1 || !item2) return false; // bad parameters
    
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
    if (!q || !item) return NULL; // bad parameters

    israeliNode* friend = q->last;
    israeliNode* cur_israeliNode = q->head;
    while (cur_israeliNode != NULL){
        if (friend == q->last && areFriends(q, cur_israeliNode->element_ptr, item) && cur_israeliNode->friendsPassed < FRIEND_QUOTA){
            friend = cur_israeliNode;
        }
        if (areRivals(q, cur_israeliNode->element_ptr, item) && cur_israeliNode->rivalsBlocked < RIVAL_QUOTA){
            cur_israeliNode->rivalsBlocked++;
            friend = q->last;
        }

        cur_israeliNode = cur_israeliNode->next;
    }

    return friend;
}

israeliNode* insertItem(IsraeliQueue q, israeliNode* foremostPos, void* item){
    if (!q || !foremostPos || !item)  return NULL; // bad parameters

    // CREATE NODE
    israeliNode* item_israeliNode = (israeliNode*)malloc(sizeof(israeliNode));
    if (item_israeliNode == NULL)  return NULL;
    item_israeliNode->element_ptr = item;
    item_israeliNode->next = NULL;
    item_israeliNode->previous = NULL;
    item_israeliNode->friendsPassed = 0;
    item_israeliNode->rivalsBlocked = 0;

    // PLACE NODE
    if (q->head == NULL){ // empty queue
        q->head = item_israeliNode;
        q->last = item_israeliNode;
    } 
    else if (foremostPos == q->last){ // no possible skip, place last
        q->last->next = item_israeliNode;
        item_israeliNode->previous = q->last;
        q->last = item_israeliNode;
        if (areFriends(q, foremostPos->element_ptr, item_israeliNode->element_ptr)){
            foremostPos->friendsPassed++;
        }
    }
    else{ // can skip
        item_israeliNode->next = foremostPos->next;
        item_israeliNode->previous = foremostPos;

        foremostPos->next->previous = item_israeliNode;
        foremostPos->next = item_israeliNode;
        foremostPos->friendsPassed++;
    }

    return item_israeliNode;
}

/**@param IsraeliQueue: an IsraeliQueue in which to insert the item.
 * @param item: an item to enqueue
 *
 * Places the item in the foremost position accessible to it.*/
IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue q, void* item){
    if (!q || !item)  return ISRAELIQUEUE_BAD_PARAM;

    israeliNode* foremostPos = findForemostPos(q, item);
    if (!foremostPos)  return ISRAELI_QUEUE_ERROR;
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
    if (!q || !newFunc)  return ISRAELIQUEUE_BAD_PARAM;
    
    int n = 0;
    for (; q->FriendshipFuncs[n] != NULL; n++);

    FriendshipFunction* newFriendshipFuncs = (FriendshipFunction*)malloc((n+1 + 1)*(sizeof(FriendshipFunction)));
    if (!newFriendshipFuncs)  return ISRAELIQUEUE_ALLOC_FAILED;

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
    if (!q) return   ISRAELIQUEUE_BAD_PARAM;
    q->friendshipThreshold = friendshipThreshold;

    return ISRAELIQUEUE_SUCCESS;
}

/**@param IsraeliQueue: an IsraeliQueue whose rivalry threshold is to be modified
 * @param friendship_threshold: a new rivalry threshold for the IsraeliQueue*/
IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue q, int rivalryThreshold){
    if (!q) return   ISRAELIQUEUE_BAD_PARAM;
    q->rivalryThreshold = rivalryThreshold;

    return ISRAELIQUEUE_SUCCESS;
}

/**Returns the number of elements of the given queue. If the parameter is NULL, 0
 * is returned.*/
int IsraeliQueueSize(IsraeliQueue q){
    if (!q || !(q->head))  return 0;

    israeliNode* tmp = q->head;
    int n = 1;
    while(tmp->next){
        n++;
        tmp = tmp->next;
    }
    return n;
}

/**Removes and returns the foremost element of the provided queue. If the parameter
 * is NULL or a pointer to an empty queue, NULL is returned.*/
void* IsraeliQueueDequeue(IsraeliQueue q){
    if (!q || !(q->head))  return NULL;

    void* tmp = q->head->element_ptr;
    israeliNode* tmpIsraeliNode = q->head;

    q->head = tmpIsraeliNode->next; // remove the head
    if (q->head != NULL)  q->head->previous = NULL;
    free(tmpIsraeliNode);
    return tmp;
}

/**@param item: an object comparable to the objects in the IsraeliQueue
 *
 * Returns whether the queue contains an element equal to item. If either
 * parameter is NULL, false is returned.*/
bool IsraeliQueueContains(IsraeliQueue q, void* element){
    if (!q || !element)  return false;
    
    int same = q->ComparisonFunc(element, element); // defining SAME
    israeliNode* cur = q->head;
    while (cur){
        if (q->ComparisonFunc(cur->element_ptr, element) == same){
            return true;
        }
        cur = cur->next;
    }

    return false;
}


// inserts an israeli node into the queue by updating the NEXT pointers only
// inserts the node AFTER foremostPos
// In the case of (foremostPos == NULL) the Node is inserted at the end of the queue
IsraeliQueueError insertIsraeliNode(IsraeliQueue q, israeliNode* foremostPos, israeliNode* item_israeliNode){
    if (!q || !foremostPos || !item_israeliNode)  return ISRAELIQUEUE_BAD_PARAM; // bad parameters

    if (!(q->head)){ // empty queue
        q->head = item_israeliNode;
        q->last = item_israeliNode;
    }
    
    if (foremostPos == q->last){ // no position to skip to, put last
        q->last->next = item_israeliNode;
        item_israeliNode->next = NULL;
        q->last = item_israeliNode;
        if (areFriends(q, foremostPos->element_ptr, item_israeliNode->element_ptr)){
            foremostPos->friendsPassed++;
        }
    }
    else{ // skip to position
        item_israeliNode->next = foremostPos->next;
        foremostPos->next = item_israeliNode;
        foremostPos->friendsPassed++;
    }

    return ISRAELIQUEUE_SUCCESS;
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
    if (!q)          return ISRAELIQUEUE_BAD_PARAM;
    if (!(q->head))  return ISRAELIQUEUE_SUCCESS;

    israeliNode* cur = q->last;
    israeliNode* previousOG; // the previous in the OG queue
    israeliNode* curPrevious;

    israeliNode* foremostPos;
    while (cur){
        previousOG = cur->previous;
        curPrevious = findPrevious(q, cur);
        // remove cur from queue (by updating the NEXT pointers)
        // we keep the PREVIOUS pointers unchanged so we could improve positions properly
            if (curPrevious != NULL){
                curPrevious->next = cur->next;
            }
            else{ // cur is head
                q->head = cur->next;
            }
            if (!(cur->next)){ // cur is last
                q->last = curPrevious;
            }
        // enque them again
            foremostPos = findForemostPos(q, cur->element_ptr);
            if (!foremostPos)  return ISRAELI_QUEUE_ERROR;
            if (insertIsraeliNode(q, foremostPos, cur) != ISRAELIQUEUE_SUCCESS){
                return ISRAELI_QUEUE_ERROR;
            }
            cur = previousOG;
    }
    // update the PREVIOUS pointers
    cur = q->head;
    cur->previous = NULL;
    while(cur->next){
        cur->next->previous = cur;
        cur = cur->next;
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
    if (n < 0) return -n;
    return n;
}

unsigned int power(int n, int exp){
    unsigned res = 1;
    for(int i = 0; i < exp; i++){
        res *= n;
    }
    return res;
}

int findNRoot(unsigned int num, int n){ // upper integer
    int res = 0;
    while(power(res, n) < num){
        res++;
    }
    return res;
}

int findMergedFriendshipThreshold(IsraeliQueue* qArr){
    if (qArr[0]) return 0; // bad parameter

    int friendshipThreshold = 0;
    int friendshipThresholdSum = 0;
    int i = 0;
    for ( ; qArr[i]; i++){
        friendshipThresholdSum += qArr[i]->friendshipThreshold;
    }
    
    return friendshipThresholdSum / i;
}

int findMergedRivalryThreshold(IsraeliQueue* qArr){
    unsigned int multiplication = 1;
    int i = 0;
    for(; qArr[i] ; i++){
        multiplication *= (unsigned int)abs(qArr[i]->rivalryThreshold);
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
    if (!qArr || !(qArr[0])) return NULL; // bad parameter

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
        if (FriendshipFuncs == NULL)  return NULL;

        int cur_func = 0;
        for (int i = 0; qArr[i] != NULL; i++){
            for (int j = 0; qArr[i]->FriendshipFuncs[j] != NULL; j++){
                FriendshipFuncs[cur_func++] = qArr[i]->FriendshipFuncs[j];
            }
        }
        FriendshipFuncs[n_funcs] = NULL;

    // Friendship Threshold
        int friendshipThreshold = findMergedFriendshipThreshold(qArr);

    // Rivalry Threshold
        int rivalryThreshold = findMergedRivalryThreshold(qArr);

    IsraeliQueue mergedQ = IsraeliQueueCreate(FriendshipFuncs, ComparisonFunc, friendshipThreshold, rivalryThreshold);
    free(FriendshipFuncs);
    if (mergedQ == NULL) return NULL; // error

    israeliNode* cur;
    void* item;
    int i = 0;
    while (!isMergeDone(qArr)){
        cur = qArr[i % n]->head;
        if (!cur){
            i++;
            continue;
        }
        item = IsraeliQueueDequeue(qArr[i % n]);
        if (!item || IsraeliQueueEnqueue(mergedQ, item) != ISRAELIQUEUE_SUCCESS){ // errors
            IsraeliQueueDestroy(mergedQ);
            return NULL;
        }
        i++;
    }

    return mergedQ;
}