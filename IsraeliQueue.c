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
    if(!FriendshipFuncs)    return NULL;
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

    q->FriendshipFuncs = (FriendshipFunction*)malloc((n+1)*(sizeof(int)));
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

    if (friendshipSum/i < q->rivalryThreshold){
        return true;
    }
    return false;
}

israeliNode* findForemostPos(IsraeliQueue q, void* item){
    israeliNode* friend = NULL;
    israeliNode* cur_israeliNode = q->head;
    while (q->head != NULL && cur_israeliNode->next != NULL){
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

    FriendshipFunction* newFriendshipFuncs = (FriendshipFunction*)malloc((n+1 + 1)*(sizeof(int)));
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
    israeliNode* tmpisraeliNode = q->head;

    q->head = tmpisraeliNode->next;
    if (q->head != NULL)    q->head->previous = NULL;
    free(tmpisraeliNode);
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


void insertisraeliNode(IsraeliQueue q, israeliNode* foremostPos, israeliNode* item_israeliNode){
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

/**Advances each item in the queue to the foremost position accessible to it,
 * from the back of the queue frontwards.*/
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q){
    if (q == NULL) return   ISRAELIQUEUE_BAD_PARAM;
    if (q->head == NULL){
        return ISRAELIQUEUE_SUCCESS;
    }

    israeliNode* cur = q->last;
    israeliNode* previous;

    israeliNode* foremostPos;
    while (cur){
        previous = cur->previous;
        // remove cur from queue
            if (cur->previous != NULL){
                cur->previous->next = cur->next;
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
            insertisraeliNode(q, foremostPos, cur);
            cur = previous;
    }

    return ISRAELIQUEUE_SUCCESS;
}


bool isMergeDone(IsraeliQueue* qArr){
    for (int i = 1; qArr[i] != NULL; i++){
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
    while (!isMergeDone(qArr)){
        for (int i = 0; i < n; i++){
            cur = qArr[i]->head;
            if (cur == NULL){
                continue;
            }
            if (insertItem(mergedQ, NULL, cur->element_ptr) == NULL){
                IsraeliQueueDestroy(mergedQ);
                return NULL;
            }
            if (IsraeliQueueDequeue(qArr[i]) == NULL){
                IsraeliQueueDestroy(mergedQ);
                return NULL;
            }
        }
    }

    return mergedQ;
}
