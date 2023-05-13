#include "HackEnrollment.h"
#include "IsraeliQueue.h"

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
    if (!q || !item)   return HACKENROLLMENT_BAD_PARAM;

    Node* node = createNode(item);
    if (node == NULL){
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    if (q->head == NULL){ // first node
        q->head = node;
        q->last = node;
    }
    else{
        q->last->next = node;
        q->last = node;
    }

    return HACKENROLLMENT_SUCCESS;
}

void destroyStudent(Student* student);
void destroyHacker(Hacker* hacker);
void destroyCourse(Course* course);
void dequeueQueue(Queue q, QueueType typeQ);
void destroyQueue(Queue q, QueueType typeQ);

void dequeueQueue(Queue q, QueueType typeQ){
    if (q == NULL || (q->head) == NULL)  return;

    Node* tmp = q->head; // save HEAD for dequeue
    q->head = tmp->next; // remove HEAD from queue

    // free element_ptr
    if (typeQ == STUDENTS_Q){
        destroyStudent((Student*)(tmp->element_ptr));
    }
    else if (typeQ == COURSES_Q){
        destroyCourse((Course*)(tmp->element_ptr));
    }

    free(tmp); // free HEAD
    return;
}

void destroyQueue(Queue q, QueueType typeQ){
    if (!q) return; // already freed

    while (q->head != NULL){
        dequeueQueue(q, typeQ);
    }
    free(q);
}

long readStringIntoLong(FILE *file, bool* endofline_ptr){
    if (!file || !endofline_ptr)  return -1; // bad parameters

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
    if (!file || !endofline_ptr)  return NULL; // bad parameters

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
Student* createStudent(FILE* students){
    if (!students)  return NULL; // bad parameter
    
    // <Student ID> <Total Credits> <GPA> <Name> <Surname> <City> <Department>\n
    bool eol = false;
    Student* student_ptr = (Student*)malloc(sizeof(Student));
    student_ptr->studentID = readStringIntoLong(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->totalCredits = (int)readStringIntoLong(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->gpa = (int)readStringIntoLong(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->name = readWord(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->surname = readWord(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->city = readWord(students, &eol);
    if (eol){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    eol = false;

    student_ptr->department = readWord(students, &eol);
    if (!(student_ptr->department)){ destroyStudent(student_ptr); return NULL; } // line ended prematurely
    student_ptr->hackerAlt = NULL;

    return student_ptr;
}

void destroyStudent(Student* student){
    if (!student) return; // already freed
    if (!(student->name))         free(student->name);;
    if (!(student->surname))      free(student->surname);
    if (!(student->city))         free(student->city);
    if (!(student->department))   free(student->department);
    destroyHacker(student->hackerAlt);
    free(student);
    return;
}

Queue createStudentsQueue(FILE *students){
    if (!students)  return NULL; // bad parameter
    Queue q = (Queue)malloc(sizeof(Queue_t));
    if (q == NULL)  return NULL;

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
    if (!students || !startingPos)  return NULL; // bad parameter

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
Queue createHackerCoursesQueue(FILE* hackers){
    if (!hackers)  return NULL; // bad parameter

    Queue desiredQueuesNums = (Queue)malloc(sizeof(Queue_t));
    if (desiredQueuesNums == NULL)  return NULL;

    desiredQueuesNums->head = NULL;
    desiredQueuesNums->last = NULL;

    long* courseNum_ptr;
    bool endofline = false;
    while (!feof(hackers)){
        courseNum_ptr = (long*)malloc(sizeof(long));
        if (!courseNum_ptr){
            destroyQueue(desiredQueuesNums, DEFAULT_Q);
            return NULL;
        }
        *courseNum_ptr = readStringIntoLong(hackers, &endofline);
        if ((*courseNum_ptr) == -1)  break;
        if (enqueue(desiredQueuesNums, courseNum_ptr) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(desiredQueuesNums, COURSES_Q);
            return NULL;
        }
        if (endofline)  break;
    }

    return desiredQueuesNums;
}

Queue createHackerFriends(FILE* hackers){
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
        if (!friendID_ptr){
            destroyQueue(friendsIDs, DEFAULT_Q);
            return NULL;
        }
        *friendID_ptr = readStringIntoLong(hackers, &endofline);
        if ((*friendID_ptr) == -1)  break;
        if (enqueue(friendsIDs, friendID_ptr) == HACKENROLLMENT_ALLOC_FAILED){
            destroyQueue(friendsIDs, DEFAULT_Q);
            return NULL;
        }
        if (endofline)  break;
    }

    return friendsIDs;
}

Queue createHackerRivals(FILE* hackers){
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
        if (!rivalID_ptr){
            destroyQueue(rivalsIDs, DEFAULT_Q);
            return NULL;
        }
        *rivalID_ptr = readStringIntoLong(hackers, &endofline);
        if ((*rivalID_ptr) == -1)  break;
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
    hacker->hackerAlt->desiredCoursesNums = createHackerCoursesQueue(hackers);
    if (hacker->hackerAlt->desiredCoursesNums == NULL){
        free(hacker->hackerAlt);
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    hacker->hackerAlt->friendsIDs = createHackerFriends(hackers);
    if (hacker->hackerAlt->friendsIDs == NULL){
        free(hacker->hackerAlt);
        destroyQueue(hacker->hackerAlt->desiredCoursesNums, DEFAULT_Q);
        return HACKENROLLMENT_ALLOC_FAILED;
    }
    hacker->hackerAlt->rivalsIDs = createHackerRivals(hackers);
    if (hacker->hackerAlt->rivalsIDs == NULL){
        free(hacker->hackerAlt);
        destroyQueue(hacker->hackerAlt->desiredCoursesNums, DEFAULT_Q);
        destroyQueue(hacker->hackerAlt->friendsIDs, DEFAULT_Q);
        return HACKENROLLMENT_ALLOC_FAILED;
    }

    return HACKENROLLMENT_SUCCESS;
}

void destroyHacker(Hacker* hacker){
    if (!hacker) return; // already freed
    destroyQueue(hacker->desiredCoursesNums, DEFAULT_Q);
    destroyQueue(hacker->friendsIDs, DEFAULT_Q);
    destroyQueue(hacker->rivalsIDs, DEFAULT_Q);
    free(hacker);
}

HackEnrollmentError insertHackersInfo(Queue students, FILE* hackers){
    if (!students || !hackers) return HACKENROLLMENT_BAD_PARAM;

    long hackerID; Student* hacker;
    bool eol = false;
    while (!feof(hackers)){
        hackerID = readStringIntoLong(hackers, &eol);
        if (eol && hackerID == -1) break;
        eol = false;
        hacker = (Student*)findStudent(students, students->head, hackerID);
        if (fillHackerInfo(hacker, hackers) == HACKENROLLMENT_ERROR){
            return HACKENROLLMENT_ERROR;
        }
    }

    return HACKENROLLMENT_SUCCESS;
}


// COURSES
Course* createCourse(FILE *courses){
    if (!courses) return NULL; // bad parameter

    bool eol = false;
    Course *course_ptr = (Course*)malloc(sizeof(Course));
    if (course_ptr == NULL)  return NULL;

    course_ptr->courseNum = readStringIntoLong(courses, &eol);
    if (eol){ // line ended prematurely, bad parameter
        destroyCourse(course_ptr);
        return NULL;
    }
    eol = false;

    course_ptr->size = (int)readStringIntoLong(courses, &eol);
    if (course_ptr->size == -1){ // line ended prematurely, bad parameter
        destroyCourse(course_ptr);
        return NULL;
    }

    FriendshipFunction* fArr = {NULL};
    course_ptr->courseQueue = IsraeliQueueCreate(fArr, NULL, FRIENDHIP_THRESHLOD, RIVALRY_THRESHLOD);
    if (!(course_ptr->courseQueue)){ // alloc error
        destroyCourse(course_ptr);
        return NULL;
    }

    return course_ptr;
}

void destroyCourse(Course* course){
    if (!course) return; // already freed
    IsraeliQueueDestroy(course->courseQueue);
    free(course);
    return;
}

Queue createCoursesQueue(FILE *courses){
    if (!courses)  return NULL; // bad parameter

    Queue q = (Queue)malloc(sizeof(Queue_t));
    if (q == NULL)  return NULL;

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
    if (!courses || !startingPos)  return NULL; // bad parameters

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
    if (!students || !startingPos)  return NULL; // bad parameters

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
                    if (!curCourse || !(curCourse->courseQueue) || IsraeliQueueEnqueue(curCourse->courseQueue, curStudent) != ISRAELIQUEUE_SUCCESS){
                        return HACKENROLLMENT_ERROR;
                    }
                    curCourseNumNode = curCourseNumNode->next;
                }
                curStudentNode = findStudentHacker(sys->studentsQueue, curStudentNode->next);
        }

        return HACKENROLLMENT_SUCCESS;
}

bool isEnrolled(IsraeliQueue courseQueue, int courseSize, Student* wanted){
    if (!courseQueue || !wanted)  return false; // bad parameters;

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
    long curCourseNum; Course* curCourse;
    IsraeliQueue curCourseQueue_cpy;
    
    while (curCourseNumNode != NULL || (*enroll_counter_ptr) < 2){
        if (!curCourseNumNode->element_ptr)  return HACKENROLLMENT_ERROR;
        curCourseNum = *((long*)(curCourseNumNode->element_ptr));
        curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
            if (!curCourse)  return HACKENROLLMENT_ERROR;

        curCourseQueue_cpy = IsraeliQueueClone(curCourse->courseQueue);
            if (!curCourseQueue_cpy)  return HACKENROLLMENT_ALLOC_FAILED;

        if (isEnrolled(curCourseQueue_cpy, (int)(curCourse->size), student)){
            (*(enroll_counter_ptr))++;
        }
        IsraeliQueueDestroy(curCourseQueue_cpy);
        curCourseNumNode = curCourseNumNode->next;
    }
    return HACKENROLLMENT_SUCCESS;
}

HackEnrollmentError findDissatisfied(EnrollmentSystem sys, long* hackerID_ptr){
    if (sys == NULL)                                    return HACKENROLLMENT_BAD_PARAM;
    if (!(sys->studentsQueue) || !(sys->coursesQueue))  return HACKENROLLMENT_ERROR;

    // VARIABLES
        // HACKER
        int enroll_counter = 0;
        Node* hackerNode = findStudentHacker(sys->studentsQueue, sys->studentsQueue->head);
        Student* hacker; Hacker* hacker_alt;
        // COURSE
        Node* curCourseNumNode; long curCourseNum; Course* curCourse;

    // FIND IF ALL HACKERS ARE SATISFIED
    while (hackerNode != NULL){
        if (hackerNode->element_ptr != NULL) return HACKENROLLMENT_ERROR;
        // find hacker
            hacker = (Student*)(hackerNode->element_ptr);
            hacker_alt = hacker->hackerAlt;
            if (!hacker_alt || !(hacker_alt->desiredCoursesNums))
                return HACKENROLLMENT_ERROR;
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
                    if (!(hacker->studentID))  return HACKENROLLMENT_ERROR;
                    *hackerID_ptr = *(long*)(hacker->studentID);
                    return HACKENROLLMENT_SUCCESS;
                }
        // go to next hacker
            hackerNode = findStudentHacker(sys->studentsQueue, hackerNode);
            enroll_counter = 0;
    }

    return HACKENROLLMENT_SUCCESS;
}

HackEnrollmentError printOut(EnrollmentSystem sys, FILE* out){
    if (!sys || !out)           return HACKENROLLMENT_BAD_PARAM;
    if (!(sys->coursesQueue))   return HACKENROLLMENT_ERROR;

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
        if (!curCourse || !(curCourse->courseQueue)) return HACKENROLLMENT_ERROR;

        fprintf(out, "%ld", curCourse->courseNum);
        studentNode = (Node*)(IsraeliQueueDequeue(curCourse->courseQueue));
        while(studentNode){
            student = (Student*)(studentNode->element_ptr);
            if (!student)   return HACKENROLLMENT_ERROR;
            fprintf(out, " %ld", student->studentID);
            studentNode = studentNode->next;
        }
        fprintf(out, "\n");
        courseNode = courseNode->next; 
    }

    return HACKENROLLMENT_SUCCESS;
}


// Friendship
// finds if the wanted ID is a part of the given queue of student IDs
bool findStudentInIDQueue(Queue studentsIDs, long wantedID){
    if (!studentsIDs) return false; // bad parameters

    Node* curStudentIDNode = studentsIDs->head;
    long* curStudentID;
    while (curStudentIDNode){
        curStudentID = (long*)(curStudentIDNode->element_ptr);
        if (!curStudentID) return false; // error
        if ((*curStudentID) == wantedID){
            return true;
        }
        curStudentIDNode = curStudentIDNode->next;
    }
    return false;
}

// returns the absolute value of a long
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
    if (!(Student*)student1 || !(Student*)student2) return 0; // bad parameters

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

// finds the difference (positive) between the ASCII values of the names of the two students
// returns -1 if bad parameters
int findNameAsciiDifference(void* student1, void* student2){
    if (!student1 || !student2) return -1;
    long value1 = findStringAscii(((Student*)student1)->name);
    long value2 = findStringAscii(((Student*)student2)->name);
    
    return ((int)(absL(value1 - value2)));
}

// finds the difference (positive) between the IDs of the two students
// returns -1 if bad parameters
int findIDDifference(void* student1, void* student2){
    if (!(Student*)student1 || !(Student*)student2) return -1;
    return ((int)absL( (((Student*)student1)->studentID) - (((Student*)student2)->studentID)) ); // |id1 - id2|
}





// Functions
void destroyEnrollment(EnrollmentSystem sys){
    if (!sys) return; // already freed
    destroyQueue(sys->coursesQueue, COURSES_Q);
    destroyQueue(sys->studentsQueue, STUDENTS_Q);
    free(sys);
}

EnrollmentSystem createEnrollment(FILE *students, FILE *courses, FILE *hackers){
    if (!students || !courses || !hackers)  return NULL; // bad parameters

    // Create a new EnrollmentSystem
    EnrollmentSystem enrollment = (EnrollmentSystem)malloc(sizeof(EnrollmentSystem_t));
    if (enrollment == NULL)  return NULL;

    // Initialize and Fill up the students and courses queues
    enrollment->studentsQueue = createStudentsQueue(students);
    if (enrollment->studentsQueue == NULL){
        destroyEnrollment(enrollment);
        return NULL;
    }

    enrollment->coursesQueue = createCoursesQueue(courses);
    if (enrollment->coursesQueue == NULL){
        destroyEnrollment(enrollment);
        return NULL;
    }

    // Fill up hackers details
    if (insertHackersInfo(enrollment->studentsQueue, hackers) == HACKENROLLMENT_ALLOC_FAILED){
        destroyEnrollment(enrollment);
        return NULL;
    }

    return enrollment;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues){
    if (!sys || !(sys->coursesQueue) || !(sys->studentsQueue) || !queues){ // bad parameters
        destroyEnrollment(sys); // preventing memory leakage
        return NULL;
    }

    Course* curCourse; long curCourseNum;
    bool eol = false;
    Student* curStudent; long cur_studentID;
    while(!feof(queues)){
        curCourseNum = readStringIntoLong(queues, &eol);
        if (eol && curCourseNum == -1) break; // end
        eol = false; // reset for eol
        curCourse = findCourse(sys->coursesQueue, sys->coursesQueue->head, curCourseNum);
        if(!curCourse || !(curCourse->courseQueue)){ // error
            destroyEnrollment(sys); // preventing memory leakage
            return NULL;
        }
        while(!eol){
            cur_studentID = readStringIntoLong(queues, &eol);
            if (eol && cur_studentID == -1) break; // end
            curStudent = findStudent(sys->studentsQueue, sys->studentsQueue->head, cur_studentID);
            if (IsraeliQueueEnqueue(curCourse->courseQueue, curStudent) != ISRAELIQUEUE_SUCCESS){
                destroyEnrollment(sys); // preventing memory leakage
                return NULL;
            }

        }
        eol = false; // reset for eol
        if (IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, areFriendsAccordingToHacker) != ISRAELIQUEUE_SUCCESS
        ||  IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, findNameAsciiDifference)
        ||  IsraeliQueueAddFriendshipMeasure(curCourse->courseQueue, findIDDifference)){
                destroyEnrollment(sys); // preventing memory leakage
                return NULL;
        }


    }
    return sys;
}

HackEnrollmentError hackEnrollment(EnrollmentSystem sys, FILE* out){
    if (!sys || !out)  return HACKENROLLMENT_BAD_PARAM;

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