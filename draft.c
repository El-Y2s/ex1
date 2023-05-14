#include "HackEnrollment.h"


int main(){
    FILE* students = fopen("/home/elias.noufi/ex1/ExampleTest/students.txt", "r");
    FILE* courses = fopen("/home/elias.noufi/ex1/ExampleTest/courses.txt", "r");
    FILE* hackers = fopen("/home/elias.noufi/ex1/ExampleTest/hackers.txt", "r");
    FILE* queues = fopen("/home/elias.noufi/ex1/ExampleTest/queues.txt", "r");
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
    fclose(out);
    fclose(students);
    fclose(hackers);
    fclose(queues);
    fclose(courses);
	return 0;
}