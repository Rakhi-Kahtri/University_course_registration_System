#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Student basic info
int student_id[10];              // IDs: 1, 2, 3...
char department[10][10];         // "CS" or "SE"
int semester[10];                // 1 to 8
int registration_order[10];      // Order number

// Each student's courses (stored as strings separated by comma)
char course1[10][30];            // First course
char course2[10][30];            // Second course  
char course3[10][30];            // Third course
int course_count[10];            // How many courses selected

int total_students = 0;
int pipe_fd[2];

pthread_mutex_t cs_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t se_lock = PTHREAD_MUTEX_INITIALIZER;

// Course seats
int cs_seats[8][3];
int se_seats[8][3];

// Course names for CS
char cs_sem1_courses[3][30] = {"PF","ICT","Math"};
char cs_sem2_courses[3][30] = {"OOP","DS","Stats"};
char cs_sem3_courses[3][30] = {"DB","OS","Algo"};
char cs_sem4_courses[3][30] = {"CN","SE","AI"};
char cs_sem5_courses[3][30] = {"Compiler","AI","Web"};
char cs_sem6_courses[3][30] = {"ML","Cloud","Mobile"};
char cs_sem7_courses[3][30] = {"Security","DevOps","Blockchain"};
char cs_sem8_courses[3][30] = {"FYP","Research","Ethics"};

// Course names for SE
char se_sem1_courses[3][30] = {"PF","ICT","Math"};
char se_sem2_courses[3][30] = {"OOP","ReqEng","Stats"};
char se_sem3_courses[3][30] = {"DB","OS","Design"};
char se_sem4_courses[3][30] = {"Testing","SE","AI"};
char se_sem5_courses[3][30] = {"Agile","Web","Cloud"};
char se_sem6_courses[3][30] = {"ML","Mobile","DevOps"};
char se_sem7_courses[3][30] = {"Security","QA","Blockchain"};
char se_sem8_courses[3][30] = {"FYP","Research","Ethics"};

// Get course name by department and semester
char* get_cs_course(int sem, int index) {
    if(sem == 1) return cs_sem1_courses[index];
    if(sem == 2) return cs_sem2_courses[index];
    if(sem == 3) return cs_sem3_courses[index];
    if(sem == 4) return cs_sem4_courses[index];
    if(sem == 5) return cs_sem5_courses[index];
    if(sem == 6) return cs_sem6_courses[index];
    if(sem == 7) return cs_sem7_courses[index];
    return cs_sem8_courses[index];
}

char* get_se_course(int sem, int index) {
    if(sem == 1) return se_sem1_courses[index];
    if(sem == 2) return se_sem2_courses[index];
    if(sem == 3) return se_sem3_courses[index];
    if(sem == 4) return se_sem4_courses[index];
    if(sem == 5) return se_sem5_courses[index];
    if(sem == 6) return se_sem6_courses[index];
    if(sem == 7) return se_sem7_courses[index];
    return se_sem8_courses[index];
}

void* scheduler(void* arg) {
    int idx;
    
    while(read(pipe_fd[0], &idx, sizeof(int)) > 0) {
        
        printf("\n[Scheduler] Processing Student %d (Order: %d)\n", 
               student_id[idx], registration_order[idx]);
        
        int sem = semester[idx] - 1;
        
        // Lock both departments
        if(strcmp(department[idx], "CS") == 0) {
            pthread_mutex_lock(&cs_lock);
            printf("  [Locked CS]\n");
            sleep(1);
            
            printf("  [Trying SE lock...]\n");
            pthread_mutex_lock(&se_lock);
            printf("  [Locked SE]\n");
        } else {
            pthread_mutex_lock(&se_lock);
            printf("  [Locked SE]\n");
            sleep(1);
            
            printf("  [Trying CS lock...]\n");
            pthread_mutex_lock(&cs_lock);
            printf("  [Locked CS]\n");
        }
        
        printf("\nStudent %d (%s - Sem %d)\n", 
               student_id[idx], department[idx], semester[idx]);
        
        // Register courses
        int count = course_count[idx];
        char* courses[3] = {course1[idx], course2[idx], course3[idx]};
        
        for(int i = 0; i < count; i++) {
            if(strcmp(department[idx], "CS") == 0) {
                // CS student
                for(int j = 0; j < 3; j++) {
                    char* course_name = get_cs_course(semester[idx], j);
                    if(strcmp(course_name, courses[i]) == 0) {
                        if(cs_seats[sem][j] > 0) {
                            cs_seats[sem][j]--;
                            printf("  REGISTERED: %s (Seats: %d)\n", 
                                   courses[i], cs_seats[sem][j]);
                        } else {
                            printf("  FAILED: %s (Full)\n", courses[i]);
                        }
                        break;
                    }
                }
            } else {
                // SE student
                for(int j = 0; j < 3; j++) {
                    char* course_name = get_se_course(semester[idx], j);
                    if(strcmp(course_name, courses[i]) == 0) {
                        if(se_seats[sem][j] > 0) {
                            se_seats[sem][j]--;
                            printf("  REGISTERED: %s (Seats: %d)\n", 
                                   courses[i], se_seats[sem][j]);
                        } else {
                            printf("  FAILED: %s (Full)\n", courses[i]);
                        }
                        break;
                    }
                }
            }
        }
        
        pthread_mutex_unlock(&cs_lock);
        pthread_mutex_unlock(&se_lock);
        printf("  [Unlocked both]\n");
    }
    return NULL;
}

int main() {
    pipe(pipe_fd);
    
    pthread_t thread;
    pthread_create(&thread, NULL, scheduler, NULL);
    
    printf("Enter number of students: ");
    while(scanf("%d", &total_students) != 1 || total_students < 1 || total_students > 10) {
        while(getchar() != '\n');
        printf("  Invalid! Enter 1-10: ");
    }
    
    int seats;
    printf("Enter seats per course: ");
    while(scanf("%d", &seats) != 1 || seats < 1) {
        while(getchar() != '\n');
        printf("  Invalid! Enter positive number: ");
    }
    
    // Set seats
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 3; j++) {
            cs_seats[i][j] = seats;
            se_seats[i][j] = seats;
        }
    }
    
    // Get student info
    for(int i = 0; i < total_students; i++) {
        student_id[i] = i + 1;
        
        printf("\n=== Student %d ===\n", i + 1);
        
        // Get department
        while(1) {
            printf("Department (CS/SE): ");
            scanf("%s", department[i]);
            
            // Make uppercase
            for(int j = 0; department[i][j]; j++) {
                if(department[i][j] >= 'a' && department[i][j] <= 'z') {
                    department[i][j] = department[i][j] - 32;
                }
            }
            
            if(strcmp(department[i], "CS") == 0 || strcmp(department[i], "SE") == 0) {
                break;
            }
            printf("  Invalid! Enter CS or SE.\n");
        }
        
        // Get semester
        printf("Semester (1-8): ");
        while(scanf("%d", &semester[i]) != 1 || semester[i] < 1 || semester[i] > 8) {
            // Clear input buffer
            while(getchar() != '\n');
            printf("  Invalid! Enter 1-8: ");
        }
        
        registration_order[i] = i + 1;
        printf("  → Order: %d (First-Come-First-Serve)\n", registration_order[i]);
        
        int sem = semester[i] - 1;
        
        // Check seats available
        int available = 0;
        if(strcmp(department[i], "CS") == 0) {
            for(int j = 0; j < 3; j++) {
                available += cs_seats[sem][j];
            }
        } else {
            for(int j = 0; j < 3; j++) {
                available += se_seats[sem][j];
            }
        }
        
        if(available == 0) {
            printf("\n⚠️  All courses FULL!\n");
            course_count[i] = 0;
            continue;
        }
        
        // Show courses
        printf("\nAvailable Courses:\n");
        if(strcmp(department[i], "CS") == 0) {
            for(int j = 0; j < 3; j++) {
                printf("%d) %s\n", j+1, get_cs_course(semester[i], j));
            }
        } else {
            for(int j = 0; j < 3; j++) {
                printf("%d) %s\n", j+1, get_se_course(semester[i], j));
            }
        }
        
        printf("How many courses (1-3): ");
        while(scanf("%d", &course_count[i]) != 1 || course_count[i] < 1 || course_count[i] > 3) {
            while(getchar() != '\n');
            printf("  Invalid! Enter 1-3: ");
        }
        
        // Get courses
        for(int c = 0; c < course_count[i]; c++) {
            int choice;
            int valid = 0;
            
            while(!valid) {
                printf("Select course %d: ", c + 1);
                
                if(scanf("%d", &choice) != 1) {
                    while(getchar() != '\n');
                    printf("  Invalid! Enter a number.\n");
                    continue;
                }
                
                if(choice < 1 || choice > 3) {
                    printf("  Invalid!\n");
                    continue;
                }
                
                char temp[30];
                int cur_seats;
                
                if(strcmp(department[i], "CS") == 0) {
                    strcpy(temp, get_cs_course(semester[i], choice - 1));
                    cur_seats = cs_seats[sem][choice - 1];
                } else {
                    strcpy(temp, get_se_course(semester[i], choice - 1));
                    cur_seats = se_seats[sem][choice - 1];
                }
                
                if(cur_seats <= 0) {
                    printf("  Full! Choose another.\n");
                    continue;
                }
                
                // Check duplicate
                int duplicate = 0;
                if(c >= 1 && strcmp(course1[i], temp) == 0) duplicate = 1;
                if(c >= 2 && strcmp(course2[i], temp) == 0) duplicate = 1;
                
                if(duplicate) {
                    printf("  Already selected!\n");
                } else {
                    if(c == 0) strcpy(course1[i], temp);
                    if(c == 1) strcpy(course2[i], temp);
                    if(c == 2) strcpy(course3[i], temp);
                    
                    printf("  Added: %s\n", temp);
                    valid = 1;
                }
            }
        }
    }
    
    printf("\n=== First-Come-First-Serve ===\n");
    printf("Processing order:\n");
    for(int i = 0; i < total_students; i++) {
        printf("  %d. Student %d (%s Sem %d)\n", 
               i+1, student_id[i], department[i], semester[i]);
    }
    
    // Send to scheduler
    for(int i = 0; i < total_students; i++) {
        if(course_count[i] > 0) {
            write(pipe_fd[1], &i, sizeof(int));
        }
    }
    
    close(pipe_fd[1]);
    pthread_join(thread, NULL);
    
    printf("\n=== Done ===\n");
    return 0;
}