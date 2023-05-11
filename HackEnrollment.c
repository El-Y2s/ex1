#include "HackEnrollment.h"

#define FRIENDHIP_THRESHLOD 20
#define RIVALRY_THRESHLOD 0

// STRUCTS
typedef struct Node
{
    void *element_ptr;
    struct Node *next;
} Node;

typedef struct Queue_t{
    Node *head;
    Node *last;
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


// ERRORS
typedef enum { HACKENROLLMENT_SUCCESS, HACKENROLLMENT_ALLOC_FAILED, HACKENROLLMENT_BAD_PARAM, HACKENROLLMENT_ERROR } HackEnrollmentError;

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
void destroyQueue(Queue q, QueueType typeQ);

void dequeueQueue(Queue q, QueueType typeQ){
    if (q == NULL || (q->head) == NULL)  return;

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
    }
    else if (typeQ == COURSES_Q){
        Course* course = (Course*)(tmp->element_ptr);
        IsraeliQueueDestroy(course->courseQueue);
    }

    free(tmp->element_ptr);
    free(tmp);
    return;
}

void destroyQueue(Queue q, QueueType typeQ){
    while (q->head != NULL){
        dequeueQueue(q, typeQ);
    }
    free(q);
}

long readStringIntoLong(FILE *file, bool* endofline_ptr){
    char digit = '0';
    while (!('0' <= (digit = fgetc(file)) && digit <= '9')); // skip till first digit

    long num = 0;
    while ('0' <= digit && digit <= '9'){
        num *= 10;
        num += digit - '0';
        digit = fgetc(file);

        if (digit == '\n'){
            *endofline_ptr = true;
        }
    }
    return num;
}

char* readWord(FILE* file, bool* endofline_ptr){
    char *word = malloc(sizeof(char));
    int wordSize = 1;
    char c;
    while (!(('a' <= (c = fgetc(file)) && c <= 'z') ||
             ('A' <= c && c <= 'Z'))); // skip till first letter or stop when \n

    while (('a' <= c && c <= 'z')||('A' <= c && c <= 'Z')) {
        word = realloc(word, ++wordSize * sizeof(char));
        word[wordSize - 2] = c;
        c=fgetc(file);

        if (c == '\n'){ // indecator to end of line
            *endofline_ptr = true;
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
    student_ptr->studentID = readStringIntoLong(students, &eol);
    student_ptr->totalCredits = (int)readStringIntoLong(students, &eol);
    student_ptr->gpa = (int)readStringIntoLong(students, &eol);
    student_ptr->name = readWord(students, &eol);
    student_ptr->surname = readWord(students, &eol);
    student_ptr->city = readWord(students, &eol);
    student_ptr->department = readWord(students, &eol);
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

    
    while(!feof(students)){
        Student* cur_student;
        cur_student = createStudent(students);
        if (cur_student == NULL || enqueue(q, cur_student) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(q, STUDENTS_Q);
            return NULL;
        }
    }

    return q;
}

Student* findStudent(Queue students, Node* startingPos, long studentID){
    Node* curNode = startingPos;
    Student* curStudent;
    while (curStudent != NULL){
        curStudent = curNode->element_ptr;
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
    long hackerID;
    Node* hackerNode = students->head; // starting point
    bool* endoffile = false;
    while (!feof(hackers)){
        hackerID = readStringIntoLong(hackers, endoffile);
        hackerNode->element_ptr = (Student*)findStudent(students, hackerNode, hackerID);
        if (fillHackerInfo((Student*)hackerNode->element_ptr, hackers) == HACKENROLLMENT_ALLOC_FAILED){
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
        if (cur_course == NULL || enqueue(q, cur_course) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(q, COURSES_Q);
            return NULL;
        }
    }

    return q;
}

Course* findCourse(Queue courses, Node* startingPos, long courseNum){
    Node* curNode = startingPos;
    Course* curCourse;
    while (curCourse != NULL){
        curCourse = curNode->element_ptr;
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
    if (sys == NULL)    return HACKENROLLMENT_BAD_PARAM;
    Node* curCourseNumNode;
    long curCourseNum;
    Course* curCourse;
    Node* curStudentNode = findStudentHacker(sys->studentsQueue, curStudentNode);;
    Student* curStudent;
    Hacker* curHacker;
        while(curStudentNode != NULL && curStudentNode->element_ptr != NULL){
            // find and improve positions of hackers by using enqueue on them only
                curStudent = (Student*)(curStudentNode->element_ptr);
            // for Hacker's desired courses
                curHacker = (Hacker*)(curStudent->hackerAlt);
                curCourseNumNode = curHacker->desiredCoursesNums->head;
                while (curCourseNumNode != NULL && curCourseNumNode->element_ptr != NULL){
                    curCourseNum = *((long*)(curCourseNumNode->element_ptr));
                    curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
                    if (curCourse->courseQueue == NULL && IsraeliQueueEnqueue(curCourse->courseQueue, curStudent) != ISRAELIQUEUE_SUCCESS){
                        return HACKENROLLMENT_ALLOC_FAILED;
                    }
                    curCourseNumNode = curCourseNumNode->next;
                }
                curStudentNode = findStudentHacker(sys->studentsQueue, curStudentNode);
        }

        return HACKENROLLMENT_SUCCESS;
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
    Course* curCourse;
    bool eol = false;
    FriendshipFunction fArr[]={NULL};
    long curCourseNum, cur_studentID;
    while(!feof(queues)){
        curCourseNum = readStringIntoLong(queues, &eol);
        curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
        curCourse->courseQueue = IsraeliQueueCreate(fArr, NULL, FRIENDHIP_THRESHLOD, RIVALRY_THRESHLOD);
        while(!eol){
            cur_studentID = readStringIntoLong(queues, &eol);
            IsraeliQueueEnqueue(curCourse->courseQueue, findStudent(sys->studentsQueue, sys->studentsQueue->head, cur_studentID));
        } 
    }
    return sys;
}

void printOut(EnrollmentSystem sys, FILE* out){
    Node* studentNode;
    Node* courseNode = sys->coursesQueue->head;
    Course* curCourse = (Course*)(courseNode->element_ptr);
    long cur_studentID;
    IsraeliQueue qClone = IsraeliQueueClone(curCourse->courseQueue);
    while(courseNode){
        fprintf(out, "%ld", curCourse->courseNum);
        studentNode = (Node*)(IsraeliQueueDequeue(curCourse->courseQueue));
        while(studentNode){
            cur_studentID = *(long*)((studentNode->element_ptr));
            fprintf(out, " %ld", cur_studentID);
            studentNode = studentNode->next;
        }
        courseNode = courseNode->next; 
        fprintf(out, "\n");
    }
    IsraeliQueueDestroy(qClone);
    return;
}

HackEnrollmentError findDissatisfied(EnrollmentSystem sys, long* hackerID_ptr){
    if (sys == NULL || sys->studentsQueue == NULL)    return HACKENROLLMENT_BAD_PARAM;
    Node* hackerNode = findStudentHacker(sys->studentsQueue, sys->studentsQueue->head);
    while (hackerNode != NULL){
    // while STUDENT_NODE != NULL
        // find hacker
        
        // courses queue
            // queue 1
                // clone
                // dequeue until HACKER FOUND or SIZE == 0
                // destroy clone
        // while SATISFIED < 2
    // SATISFIED == 1, check HEAD == LAST
    // if DISSATISFIED RETURN NULL, else CONTINUE
    }
}

void hackEnrollment(EnrollmentSystem sys, FILE* out){
    // improve positions for HACKERS on a COPY
    // check if ALL HACKERS are satisfied
    
    long dissatisfiedHackerID = -1;
    if(findDissatisfied(sys, &dissatisfiedHackerID) != HACKENROLLMENT_SUCCESS){
        return;
    }
    else if (dissatisfiedHackerID != -1){
        fprintf(out, "Cannot satisfy constraints for %ld", dissatisfiedHackerID);
    }
    // print result
}