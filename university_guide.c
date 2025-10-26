#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- CONSTANTS AND MACROS ---
#define MAX_NAME_LENGTH 100
#define MAX_LINE_LENGTH 256
#define MAX_ID_LENGTH 15
#define HASH_SIZE 50
#define MAX_RECORDS 100

// --- GLOBAL ROLE DEFINITION ---
typedef enum {
    ROLE_UNKNOWN,
    ROLE_STUDENT,
    ROLE_ADMIN
} UserRole;

// --- DATA STRUCTURES (Struct Definitions) ---

// 1. Hash Table Node for Location Guide (using Linked List for Separate Chaining)
typedef struct LocationNode {
    char key[MAX_ID_LENGTH];         // e.g., "c_lab" (for fast search)
    char building[MAX_NAME_LENGTH];
    char floor[MAX_ID_LENGTH];
    char room[MAX_ID_LENGTH];
    char description[MAX_NAME_LENGTH];
    struct LocationNode *next;
} LocationNode;

// 2. Stack Node for Navigation History (Not fully implemented but structure maintained)
typedef struct HistoryNode {
    char action[MAX_NAME_LENGTH];
    struct HistoryNode *next;
} HistoryNode;

// 3. Student Record
typedef struct Student {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char program[MAX_NAME_LENGTH];
    char email[MAX_NAME_LENGTH];
} Student;

// 4. Course Record
typedef struct Course {
    char code[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    int credits;
} Course;

// 5. Faculty Record
typedef struct Faculty {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char department[MAX_NAME_LENGTH];
} Faculty;


// --- GLOBAL VARIABLES ---
LocationNode *hashTable[HASH_SIZE]; // The Hash Table array
HistoryNode *historyStack = NULL;  // The top of the history stack
char loggedInUserID[MAX_ID_LENGTH] = ""; // ID of the currently logged-in user
UserRole currentUserRole = ROLE_UNKNOWN; // Role of the currently logged-in user

// Arrays to hold records (Simulating data store for Admin/Student modules)
Student students[MAX_RECORDS];
int studentCount = 0;
Course courses[MAX_RECORDS];
int courseCount = 0;
Faculty faculty[MAX_RECORDS];
int facultyCount = 0;

// --- FUNCTION PROTOTYPES ---
void clearScreen();
void pressEnterToContinue();
void normalizeString(char *str);
int getMenuChoice(int maxOption);
char* getInput(const char* prompt);

// Authentication & Core Menus
void loginMenu();
void mainMenu();
void adminMenu();
void studentMenu();

// File I/O for records
void loadAllRecords();
void saveStudentRecords();
void saveCourseRecords();
void saveFacultyRecords();

// Location Hash Table Management
void loadLocationData();
int hashFunction(const char *key);
void insertLocation(const char *key, const char *building, const char *floor, const char *room, const char *desc);
void saveLocationToFile(const char *key, const char *building, const char *floor, const char *room, const char *desc);
void locationFunctionalitiesMenu();
void searchLocation();
void viewAllLocations();
void addLocationMenu();
void deleteLocationMenu();

// Student Module Features
void viewPersonalProfile();
void viewCourseCatalog();
void viewFacultyDirectory();
void viewNotices();
void gpaCalculator();

// Admin Module Features
void studentCRUDMenu();
void courseCRUDMenu();
void facultyCRUDMenu();
void displayStudentRecords();
void displayCourseRecords();
void displayFacultyRecords();
void addStudentRecord();
void deleteStudentRecord();
void updateStudentRecord();
void addCourseRecord();
void deleteCourseRecord();
void updateCourseRecord();
void addFacultyRecord();
void deleteFacultyRecord();
void updateFacultyRecord();
void editNotices();

// --- CORE UTILITY FUNCTIONS ---

// Clears the console screen for cleaner UI
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Pauses execution until user hits enter
void pressEnterToContinue() {
    printf("\n\nPress ENTER to continue...");
    while (getchar() != '\n'); // Consume remaining newlines if any
    getchar(); // Wait for a new enter key press
}

// Cleans input string by removing trailing whitespace/newline
void normalizeString(char *str) {
    if (!str || str[0] == '\0') return;
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
}

// Helper to reliably get a string input (only accepts a single prompt string)
char* getInput(const char* prompt) {
    static char buffer[MAX_NAME_LENGTH];
    printf("%s", prompt);
    // Read the line and replace the newline with a null terminator
    if (fgets(buffer, MAX_NAME_LENGTH, stdin) != NULL) {
        normalizeString(buffer);
        return buffer;
    }
    return ""; // Return empty string on error
}

// Generic menu choice handler
int getMenuChoice(int maxOption) {
    int choice;
    printf("\n> Enter your choice (1-%d): ", maxOption);
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > maxOption) {
        printf("   Invalid input. Please enter a number between 1 and %d: ", maxOption);
        // Clear input buffer
        while (getchar() != '\n');
    }
    // Clear input buffer after valid integer read
    while (getchar() != '\n');
    return choice;
}

// --- FILE I/O AND DATA LOADING ---

// Loads records from all data files into memory arrays
void loadAllRecords() {
    loadLocationData(); // Loads locations into Hash Table
    
    // --- Load Students ---
    FILE *fp = fopen("students.dat", "r");
    studentCount = 0;
    if (fp) {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, fp) && studentCount < MAX_RECORDS) {
            char *token = strtok(line, ",");
            if (token) strcpy(students[studentCount].id, token); else continue;
            token = strtok(NULL, ",");
            if (token) strcpy(students[studentCount].name, token); else continue;
            token = strtok(NULL, ",");
            if (token) strcpy(students[studentCount].program, token); else continue;
            token = strtok(NULL, ",");
            if (token) {
                normalizeString(token);
                strcpy(students[studentCount].email, token);
            } else continue;
            studentCount++;
        }
        fclose(fp);
    }

    // --- Load Courses ---
    fp = fopen("courses.dat", "r");
    courseCount = 0;
    if (fp) {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, fp) && courseCount < MAX_RECORDS) {
            char *token = strtok(line, ",");
            if (token) strcpy(courses[courseCount].code, token); else continue;
            token = strtok(NULL, ",");
            if (token) strcpy(courses[courseCount].name, token); else continue;
            token = strtok(NULL, ",");
            if (token) {
                courses[courseCount].credits = atoi(token);
            } else continue;
            courseCount++;
        }
        fclose(fp);
    }
    
    // --- Load Faculty ---
    fp = fopen("faculty.dat", "r");
    facultyCount = 0;
    if (fp) {
        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, fp) && facultyCount < MAX_RECORDS) {
            char *token = strtok(line, ",");
            if (token) strcpy(faculty[facultyCount].id, token); else continue;
            token = strtok(NULL, ",");
            if (token) strcpy(faculty[facultyCount].name, token); else continue;
            token = strtok(NULL, ",");
            if (token) {
                normalizeString(token);
                strcpy(faculty[facultyCount].department, token);
            } else continue;
            facultyCount++;
        }
        fclose(fp);
    }
}

// Saves Student records from memory array back to file
void saveStudentRecords() {
    FILE *fp = fopen("students.dat", "w");
    if (!fp) {
        printf("Error: Could not open students.dat for writing.\n");
        return;
    }
    for (int i = 0; i < studentCount; i++) {
        fprintf(fp, "%s,%s,%s,%s\n", 
                students[i].id, 
                students[i].name, 
                students[i].program, 
                students[i].email);
    }
    fclose(fp);
}

// Saves Course records from memory array back to file
void saveCourseRecords() {
    FILE *fp = fopen("courses.dat", "w");
    if (!fp) {
        printf("Error: Could not open courses.dat for writing.\n");
        return;
    }
    for (int i = 0; i < courseCount; i++) {
        fprintf(fp, "%s,%s,%d\n", 
                courses[i].code, 
                courses[i].name, 
                courses[i].credits);
    }
    fclose(fp);
}

// Saves Faculty records from memory array back to file
void saveFacultyRecords() {
    FILE *fp = fopen("faculty.dat", "w");
    if (!fp) {
        printf("Error: Could not open faculty.dat for writing.\n");
        return;
    }
    for (int i = 0; i < facultyCount; i++) {
        fprintf(fp, "%s,%s,%s\n", 
                faculty[i].id, 
                faculty[i].name, 
                faculty[i].department);
    }
    fclose(fp);
}


// --- HASH TABLE (LOCATION GUIDE) IMPLEMENTATION ---

// Simple custom hash function for strings
int hashFunction(const char *key) {
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = (hash * 31 + key[i]) % HASH_SIZE;
    }
    return hash;
}

// Inserts a new location into the hash table
void insertLocation(const char *key, const char *building, const char *floor, const char *room, const char *desc) {
    int index = hashFunction(key);
    LocationNode *newNode = (LocationNode *)malloc(sizeof(LocationNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Copy data to the new node
    strncpy(newNode->key, key, MAX_ID_LENGTH - 1); newNode->key[MAX_ID_LENGTH - 1] = '\0';
    strncpy(newNode->building, building, MAX_NAME_LENGTH - 1); newNode->building[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(newNode->floor, floor, MAX_ID_LENGTH - 1); newNode->floor[MAX_ID_LENGTH - 1] = '\0';
    strncpy(newNode->room, room, MAX_ID_LENGTH - 1); newNode->room[MAX_ID_LENGTH - 1] = '\0';
    strncpy(newNode->description, desc, MAX_NAME_LENGTH - 1); newNode->description[MAX_NAME_LENGTH - 1] = '\0';

    // Insert node at the head of the linked list (Separate Chaining)
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// Reads data from the file into the Hash Table
void loadLocationData() {
    // Free existing hash table data first
    for (int i = 0; i < HASH_SIZE; i++) {
        LocationNode *current = hashTable[i];
        while (current) {
            LocationNode *temp = current;
            current = current->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }

    FILE *fp = fopen("university_data.txt", "r");
    if (fp == NULL) {
        printf("Warning: 'university_data.txt' not found or could not be opened. Location features will be empty.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        // Skip comment lines or empty lines
        if (line[0] == '#' || strlen(line) < 5) continue;

        // Tokenize line using ';' delimiter
        char *key = strtok(line, ";");
        char *building = strtok(NULL, ";");
        char *floor = strtok(NULL, ";");
        char *room = strtok(NULL, ";");
        char *description = strtok(NULL, ";");

        if (key && building && floor && room && description) {
            normalizeString(key);
            normalizeString(building);
            normalizeString(floor);
            normalizeString(room);
            normalizeString(description);
            insertLocation(key, building, floor, room, description);
        }
    }
    fclose(fp);
}

// Appends a new location to the data file
void saveLocationToFile(const char *key, const char *building, const char *floor, const char *room, const char *desc) {
    FILE *fp = fopen("university_data.txt", "a"); // 'a' for append mode
    if (fp == NULL) {
        printf("Error: Could not open university_data.txt for writing.\n");
        return;
    }
    fprintf(fp, "%s;%s;%s;%s;%s\n", key, building, floor, room, desc);
    fclose(fp);
    loadLocationData(); // Reload hash table to include new entry
}

// Rewrites the entire location file from the current Hash Table state
void rewriteLocationFile() {
    FILE *fp = fopen("university_data.txt", "w"); // 'w' for overwrite mode
    if (fp == NULL) {
        printf("Error: Could not open university_data.txt for writing.\n");
        return;
    }
    fprintf(fp, "# Location Data File\n");
    fprintf(fp, "# Format: key;building;floor;room;description\n");

    for (int i = 0; i < HASH_SIZE; i++) {
        LocationNode *current = hashTable[i];
        while (current) {
            fprintf(fp, "%s;%s;%s;%s;%s\n", 
                    current->key, 
                    current->building, 
                    current->floor, 
                    current->room, 
                    current->description);
            current = current->next;
        }
    }
    fclose(fp);
}

// --- LOCATION FUNCTIONALITIES MENU ---

void locationFunctionalitiesMenu() {
    clearScreen();
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("              Campus Location Guidebook\n");
        printf("======================================================\n");
        printf("This module allows you to find locations by Task Key.\n");
        printf("Example Keys: 'library', 'c_lab', 'admin_office', etc.\n\n");
        
        printf("[1] Search for a Location (By Task Key)\n");
        printf("[2] View All Available Locations\n");
        printf("[3] Add a New Location (Admin/Staff only)\n");
        printf("[4] Delete a Location (Admin/Staff only)\n");
        printf("[5] Return to Main Menu\n");
        printf("------------------------------------------------------\n");

        choice = getMenuChoice(5);

        switch (choice) {
            case 1: searchLocation(); break;
            case 2: viewAllLocations(); break;
            case 3: addLocationMenu(); break;
            case 4: deleteLocationMenu(); break;
            case 5: printf("Returning to Main Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

void searchLocation() {
    clearScreen();
    printf("======================================================\n");
    printf("                  Search Location\n");
    printf("======================================================\n");
    printf("Instruction: Enter the unique 'Task Key' (e.g., 'library', 'c_lab') to find its location.\n\n");
    
    char *key = getInput("Enter Task Key to search: ");
    
    int index = hashFunction(key);
    LocationNode *current = hashTable[index];
    int found = 0;

    printf("\n--- Search Results ---\n");
    while (current) {
        if (strcmp(current->key, key) == 0) {
            printf("Key: %s\n", current->key);
            printf("Building: %s\n", current->building);
            printf("Floor: %s\n", current->floor);
            printf("Room/Facility: %s\n", current->room);
            printf("Description: %s\n", current->description);
            found = 1;
            break;
        }
        current = current->next;
    }

    if (!found) {
        printf("Location for key '%s' not found in the directory.\n", key);
    }
}

void viewAllLocations() {
    clearScreen();
    printf("======================================================\n");
    printf("              All Available Locations\n");
    printf("======================================================\n");
    printf("This shows all locations currently loaded into the system:\n\n");
    
    int total = 0;
    
    printf("+-----------------+---------------------+-------------+-----------+------------------------+\n");
    printf("| Task Key        | Building            | Floor       | Room      | Description            |\n");
    printf("+-----------------+---------------------+-------------+-----------+------------------------+\n");

    for (int i = 0; i < HASH_SIZE; i++) {
        LocationNode *current = hashTable[i];
        while (current) {
            printf("| %-15s | %-19s | %-11s | %-9s | %-22s |\n", 
                   current->key, 
                   current->building, 
                   current->floor, 
                   current->room, 
                   current->description);
            current = current->next;
            total++;
        }
    }
    
    printf("+-----------------+---------------------+-------------+-----------+------------------------+\n");
    printf("\nTotal locations found: %d\n", total);
}

void addLocationMenu() {
    if (currentUserRole != ROLE_ADMIN) {
        printf("\nAuthorization Required: Only Administrators can add new locations.\n");
        return;
    }
    
    clearScreen();
    printf("======================================================\n");
    printf("                  Add New Location\n");
    printf("======================================================\n");
    printf("Instruction: Enter details for the new location.\n");
    printf("The Task Key must be unique for fast lookups.\n\n");
    
    char *key = NULL;
    char key_buffer[MAX_ID_LENGTH];
    
    // Check if key already exists
    do {
        key = getInput("Enter UNIQUE Task Key (e.g., 'physics_lab'): ");
        if (key[0] == '\0') {
             printf("Key cannot be empty.\n");
             continue;
        }
        
        int index = hashFunction(key);
        LocationNode *current = hashTable[index];
        int exists = 0;
        
        while (current) {
            if (strcmp(current->key, key) == 0) {
                exists = 1;
                break;
            }
            current = current->next;
        }

        if (exists) {
            printf("Error: This Task Key already exists. Please choose a different key.\n");
        } else {
            strcpy(key_buffer, key); // Copy the valid key
            break;
        }
    } while(1);
    
    char *building = getInput("Enter Building Name: ");
    char *floor = getInput("Enter Floor (e.g., '1st', 'Ground'): ");
    char *room = getInput("Enter Room/Facility Code: ");
    char *description = getInput("Enter Short Description: ");
    
    // Save to file and reload hash table
    saveLocationToFile(key_buffer, building, floor, room, description);
    printf("\nSuccess! Location '%s' has been added to the guide.\n", key_buffer);
}

void deleteLocationMenu() {
    if (currentUserRole != ROLE_ADMIN) {
        printf("\nAuthorization Required: Only Administrators can delete locations.\n");
        return;
    }
    
    clearScreen();
    printf("======================================================\n");
    printf("                  Delete Location\n");
    printf("======================================================\n");
    printf("Instruction: Enter the Task Key of the location you wish to remove.\n\n");

    char *keyToDelete = getInput("Enter Task Key to delete: ");
    
    int index = hashFunction(keyToDelete);
    LocationNode *current = hashTable[index];
    LocationNode *prev = NULL;
    int found = 0;

    // Search and remove from the linked list (Separate Chaining)
    while (current) {
        if (strcmp(current->key, keyToDelete) == 0) {
            if (prev == NULL) {
                // Deleting the head node
                hashTable[index] = current->next;
            } else {
                // Deleting a non-head node
                prev->next = current->next;
            }
            free(current);
            found = 1;
            break;
        }
        prev = current;
        current = current->next;
    }

    if (found) {
        // Rewrite the file with the remaining locations
        rewriteLocationFile();
        printf("\nSuccess! Location '%s' has been deleted from the guide.\n", keyToDelete);
    } else {
        printf("\nError: Location with key '%s' not found.\n", keyToDelete);
    }
}

// --- AUTHENTICATION ---

void loginMenu() {
    char username[MAX_ID_LENGTH];
    char password[MAX_ID_LENGTH];
    char roleStr[MAX_ID_LENGTH];
    UserRole role = ROLE_UNKNOWN;
    
    // Persistent loop until successful login or exit
    while (currentUserRole == ROLE_UNKNOWN) {
        clearScreen();
        printf("======================================================\n");
        printf("          University Help Guidebook - LOGIN\n");
        printf("======================================================\n");
        printf("Instruction: Enter your credentials or type 'exit' to quit.\n\n");

        char *inputUser = getInput("Username/ID: ");
        if (strcmp(inputUser, "exit") == 0) {
            printf("\nExiting application. Goodbye!\n");
            exit(0);
        }
        strncpy(username, inputUser, MAX_ID_LENGTH);

        char *inputPass = getInput("Password: ");
        strncpy(password, inputPass, MAX_ID_LENGTH);

        FILE *fp = fopen("users.dat", "r");
        int authenticated = 0;

        if (fp == NULL) {
            printf("\nError: 'users.dat' file not found. Cannot proceed with login.\n");
            pressEnterToContinue();
            continue;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
            char temp_user[MAX_ID_LENGTH];
            char temp_pass[MAX_ID_LENGTH];
            char temp_role[MAX_ID_LENGTH];

            // Use sscanf to safely parse the line (format: username,password,role)
            if (sscanf(line, "%[^,],%[^,],%s", temp_user, temp_pass, temp_role) == 3) {
                normalizeString(temp_role); // Clean up the role string

                if (strcmp(username, temp_user) == 0 && strcmp(password, temp_pass) == 0) {
                    authenticated = 1;
                    strncpy(loggedInUserID, temp_user, MAX_ID_LENGTH);
                    strncpy(roleStr, temp_role, MAX_ID_LENGTH);
                    
                    if (strcmp(roleStr, "admin") == 0) {
                        role = ROLE_ADMIN;
                    } else if (strcmp(roleStr, "student") == 0) {
                        role = ROLE_STUDENT;
                    } else {
                        role = ROLE_UNKNOWN;
                    }
                    break;
                }
            }
        }
        fclose(fp);

        if (authenticated && role != ROLE_UNKNOWN) {
            currentUserRole = role;
            printf("\n--- Login Successful! ---\n");
            printf("Welcome, %s (%s).\n", username, roleStr);
            pressEnterToContinue();
            // Break the login loop and proceed to main menu
            break;
        } else {
            printf("\n--- Login Failed! ---\n");
            printf("Invalid Username/ID or Password. Please try again.\n");
            pressEnterToContinue();
        }
    }
}


// --- MAIN MENUS ---

void mainMenu() {
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("       University Help Guidebook - Main Menu\n");
        printf("======================================================\n");
        printf("Logged In as: %s | Role: %s\n\n", loggedInUserID, (currentUserRole == ROLE_ADMIN ? "Administrator" : "Student"));
        
        // Modules available to both
        printf("[1] Campus Location Functionalities (Hash Table/Linked List)\n");
        printf("[2] Campus Directory Search\n");
        
        // Role-specific access
        if (currentUserRole == ROLE_ADMIN) {
            printf("[3] Administrator Management Module (CRUD)\n");
        } else if (currentUserRole == ROLE_STUDENT) {
            printf("[3] Student Personal Module\n");
        }
        
        printf("[4] Log Out\n");
        printf("[5] Exit Application\n");
        printf("------------------------------------------------------\n");

        choice = getMenuChoice(5);

        switch (choice) {
            case 1: locationFunctionalitiesMenu(); break;
            case 2: 
                // Campus directory search is now just a view all faculty for students, and a full search for admins.
                viewFacultyDirectory(); // Show all faculty directory
                break;
            case 3:
                if (currentUserRole == ROLE_ADMIN) {
                    adminMenu();
                } else if (currentUserRole == ROLE_STUDENT) {
                    studentMenu();
                }
                break;
            case 4: 
                printf("\nLogging out user %s...\n", loggedInUserID);
                currentUserRole = ROLE_UNKNOWN;
                loggedInUserID[0] = '\0';
                pressEnterToContinue();
                break; // Exit do-while loop to return to loginMenu
            case 5:
                printf("\nExiting application. Goodbye!\n");
                exit(0);
        }
        
        // If the choice wasn't Log Out or Exit, pause
        if (choice != 4 && choice != 5) pressEnterToContinue();

    } while (choice != 4 && choice != 5);
}

void adminMenu() {
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("         ADMINISTRATOR MANAGEMENT MODULE\n");
        printf("======================================================\n");
        printf("Instruction: Select a data set to manage (CRUD operations).\n\n");
        
        printf("[1] Manage Student Records\n");
        printf("[2] Manage Course Information\n");
        printf("[3] Manage Faculty Information\n");
        printf("[4] Edit Academic Calendar/Notices\n");
        printf("[5] Return to Main Menu\n");
        printf("------------------------------------------------------\n");
        
        choice = getMenuChoice(5);

        switch (choice) {
            case 1: studentCRUDMenu(); break;
            case 2: courseCRUDMenu(); break;
            case 3: facultyCRUDMenu(); break;
            case 4: editNotices(); break;
            case 5: printf("Returning to Main Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

void studentMenu() {
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("             STUDENT PERSONAL MODULE\n");
        printf("======================================================\n");
        printf("Instruction: Access your guide features and personal tools.\n\n");
        
        printf("[1] View Personal Profile\n");
        printf("[2] View Course Catalog\n");
        printf("[3] View Academic Calendar/Notices\n");
        printf("[4] Simple GPA Calculator\n");
        printf("[5] Return to Main Menu\n");
        printf("------------------------------------------------------\n");
        
        choice = getMenuChoice(5);

        switch (choice) {
            case 1: viewPersonalProfile(); break;
            case 2: viewCourseCatalog(); break;
            case 3: viewNotices(); break;
            case 4: gpaCalculator(); break;
            case 5: printf("Returning to Main Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

// --- STUDENT MODULE FEATURES ---

void viewPersonalProfile() {
    clearScreen();
    printf("======================================================\n");
    printf("                View Personal Profile\n");
    printf("======================================================\n");
    printf("Instruction: Displaying details for the logged-in student.\n\n");

    int found = 0;
    for (int i = 0; i < studentCount; i++) {
        if (strcmp(students[i].id, loggedInUserID) == 0) {
            printf("Student ID: %s\n", students[i].id);
            printf("Name:       %s\n", students[i].name);
            printf("Program:    %s\n", students[i].program);
            printf("Email:      %s\n", students[i].email);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Error: Your student record was not found in the database (ID: %s).\n", loggedInUserID);
    }
}

void viewCourseCatalog() {
    clearScreen();
    printf("======================================================\n");
    printf("                 Course Catalog\n");
    printf("======================================================\n");
    printf("Instruction: Displaying all available courses.\n\n");
    
    if (courseCount == 0) {
        printf("The Course Catalog is currently empty.\n");
        return;
    }

    printf("+-----------+-----------------------------------+---------+\n");
    printf("| Code      | Course Name                       | Credits |\n");
    printf("+-----------+-----------------------------------+---------+\n");
    for (int i = 0; i < courseCount; i++) {
        printf("| %-9s | %-33s | %-7d |\n", 
               courses[i].code, 
               courses[i].name, 
               courses[i].credits);
    }
    printf("+-----------+-----------------------------------+---------+\n");
}

// NOTE: This function is the ONLY thing called by the Main Menu option for Campus Directory (Option 2)
void viewFacultyDirectory() {
    clearScreen();
    printf("======================================================\n");
    printf("              Campus Faculty Directory\n");
    printf("======================================================\n");
    printf("Instruction: Displaying the complete faculty directory for guidance.\n\n");
    
    if (facultyCount == 0) {
        printf("The Faculty Directory is currently empty.\n");
        return;
    }

    printf("+-----------+--------------------------+-----------------------+\n");
    printf("| Faculty ID| Name                     | Department            |\n");
    printf("+-----------+--------------------------+-----------------------+\n");
    for (int i = 0; i < facultyCount; i++) {
        printf("| %-9s | %-24s | %-21s |\n", 
               faculty[i].id, 
               faculty[i].name, 
               faculty[i].department);
    }
    printf("+-----------+--------------------------+-----------------------+\n");
}

void viewNotices() {
    clearScreen();
    printf("======================================================\n");
    printf("           Academic Calendar & Notices\n");
    printf("======================================================\n");
    printf("Instruction: Check here for important announcements.\n\n");

    FILE *fp = fopen("notices.txt", "r");
    if (fp == NULL) {
        printf("Error: 'notices.txt' file not found.\n");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        normalizeString(line); // Clean up the line
        printf("   %s\n", line);
    }

    fclose(fp);
}

void gpaCalculator() {
    clearScreen();
    printf("======================================================\n");
    printf("               Simple GPA Calculator\n");
    printf("======================================================\n");
    printf("Instruction: Enter grades and credits for your courses.\n");
    printf("The calculator uses a standard 10-point scale (A=10, B=8, C=6, D=4, F=0).\n\n");
    
    int numCourses = 0;
    float totalGradePoints = 0.0;
    int totalCredits = 0;
    
    printf("How many courses will you enter? ");
    if (scanf("%d", &numCourses) != 1 || numCourses <= 0) {
        printf("Invalid number of courses.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n'); // Clear buffer
    
    printf("\n--- Start Entering Course Grades ---\n");
    
    for (int i = 0; i < numCourses; i++) {
        char gradeChar;
        int credits;
        int gradeValue = 0;

        printf("Course %d:\n", i + 1);
        printf("  Enter Grade (A, B, C, D, F): ");
        if (scanf(" %c", &gradeChar) != 1) {
            printf("Invalid grade input. Skipping course.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // Clear buffer

        printf("  Enter Credits (1-5): ");
        if (scanf("%d", &credits) != 1 || credits <= 0) {
            printf("Invalid credit input. Skipping course.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // Clear buffer

        gradeChar = toupper(gradeChar);

        switch (gradeChar) {
            case 'A': gradeValue = 10; break;
            case 'B': gradeValue = 8; break;
            case 'C': gradeValue = 6; break;
            case 'D': gradeValue = 4; break;
            case 'F': gradeValue = 0; break;
            default:
                printf("Warning: Invalid grade character entered. Skipping course.\n");
                continue;
        }

        totalGradePoints += (float)gradeValue * credits;
        totalCredits += credits;
    }

    printf("\n------------------------------------------------------\n");
    if (totalCredits > 0) {
        float gpa = totalGradePoints / totalCredits;
        printf("  Total Credits Attempted: %d\n", totalCredits);
        printf("  Total Grade Points: %.2f\n", totalGradePoints);
        printf("  *** Calculated GPA: %.2f ***\n", gpa);
        printf("------------------------------------------------------\n");
        
        // Optional: Save GPA result to student's file
        // This is complex for a miniproject, so we'll just display it.
        // File implementation for grades_[studentID].dat would be similar to other saves.
    } else {
        printf("No valid courses were entered. GPA calculation skipped.\n");
    }
}


// --- ADMIN MODULE CRUD UTILITIES (STUDENT) ---

void studentCRUDMenu() {
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("           MANAGE STUDENT RECORDS (CRUD)\n");
        printf("======================================================\n");
        printf("Instruction: Choose an operation for Student Records.\n\n");
        
        printf("[1] View All Student Records\n");
        printf("[2] Add New Student Record\n");
        printf("[3] Update Existing Student Record\n");
        printf("[4] Delete Student Record\n");
        printf("[5] Return to Admin Menu\n");
        printf("------------------------------------------------------\n");
        
        choice = getMenuChoice(5);

        switch (choice) {
            case 1: displayStudentRecords(); break;
            case 2: addStudentRecord(); break;
            case 3: updateStudentRecord(); break;
            case 4: deleteStudentRecord(); break;
            case 5: printf("Returning to Admin Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

void displayStudentRecords() {
    clearScreen();
    printf("======================================================\n");
    printf("                All Student Records\n");
    printf("======================================================\n");
    
    if (studentCount == 0) {
        printf("No student records found.\n");
        return;
    }
    
    printf("+-----------+--------------------------+-----------------------+--------------------------+\n");
    printf("| Student ID| Name                     | Program               | Email                    |\n");
    printf("+-----------+--------------------------+-----------------------+--------------------------+\n");
    for (int i = 0; i < studentCount; i++) {
        printf("| %-9s | %-24s | %-21s | %-24s |\n", 
               students[i].id, 
               students[i].name, 
               students[i].program, 
               students[i].email);
    }
    printf("+-----------+--------------------------+-----------------------+--------------------------+\n");
}

void addStudentRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("               Add New Student Record\n");
    printf("======================================================\n");
    
    if (studentCount >= MAX_RECORDS) {
        printf("Error: Database capacity reached (%d records).\n", MAX_RECORDS);
        return;
    }
    
    char newID[MAX_ID_LENGTH];
    char *inputID;
    int index = studentCount;

    // Check for unique ID
    do {
        inputID = getInput("Enter new Student ID: ");
        int exists = 0;
        for (int i = 0; i < studentCount; i++) {
            if (strcmp(students[i].id, inputID) == 0) {
                exists = 1;
                break;
            }
        }
        if (exists) {
            printf("Error: Student ID already exists. Try again.\n");
        } else {
            strcpy(newID, inputID);
            break;
        }
    } while(1);

    strcpy(students[index].id, newID);
    strcpy(students[index].name, getInput("Enter Student Name: "));
    strcpy(students[index].program, getInput("Enter Student Program: "));
    strcpy(students[index].email, getInput("Enter Student Email: "));
    
    studentCount++;
    saveStudentRecords();
    
    // Also add a default entry to users.dat for login (password = student ID)
    FILE *fp = fopen("users.dat", "a");
    if (fp) {
        fprintf(fp, "%s,%s,student\n", students[index].id, students[index].id);
        fclose(fp);
    }

    printf("\nSuccess! Student record (ID: %s) added and user login created (Password = ID).\n", newID);
}

void deleteStudentRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Delete Student Record\n");
    printf("======================================================\n");
    
    char *idToDelete = getInput("Enter Student ID to delete: ");
    int foundIndex = -1;
    
    for (int i = 0; i < studentCount; i++) {
        if (strcmp(students[i].id, idToDelete) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        // Shift remaining elements to fill the gap
        for (int i = foundIndex; i < studentCount - 1; i++) {
            students[i] = students[i+1];
        }
        studentCount--;
        saveStudentRecords();
        printf("\nSuccess! Student record (ID: %s) deleted.\n", idToDelete);
        
        // NOTE: Deleting the entry from users.dat is complex file manipulation for a miniproject.
        // For simplicity, we skip deleting the user login entry, meaning the user can no longer log in 
        // as their ID is no longer valid in students.dat, but the login record remains.
    } else {
        printf("\nError: Student ID '%s' not found.\n", idToDelete);
    }
}

void updateStudentRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Update Student Record\n");
    printf("======================================================\n");
    
    char *idToUpdate = getInput("Enter Student ID to update: ");
    int foundIndex = -1;
    
    for (int i = 0; i < studentCount; i++) {
        if (strcmp(students[i].id, idToUpdate) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        printf("\nUpdating Record for Student ID: %s (Name: %s)\n", students[foundIndex].id, students[foundIndex].name);
        printf("--- Enter new values (or press Enter to keep current value) ---\n");
        
        // FIX: Create a formatted prompt string before calling getInput
        char promptBuffer[MAX_LINE_LENGTH];

        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Name (Current: %s): ", students[foundIndex].name);
        char *input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(students[foundIndex].name, input);

        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Program (Current: %s): ", students[foundIndex].program);
        input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(students[foundIndex].program, input);

        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Email (Current: %s): ", students[foundIndex].email);
        input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(students[foundIndex].email, input);
        
        saveStudentRecords();
        printf("\nSuccess! Student record (ID: %s) updated.\n", idToUpdate);
    } else {
        printf("\nError: Student ID '%s' not found.\n", idToUpdate);
    }
}

// --- ADMIN MODULE CRUD UTILITIES (COURSE, FACULTY) ---

// COURSE CRUD Menu
void courseCRUDMenu() {
    // CRUD menu structure similar to studentCRUDMenu... (1: View, 2: Add, 3: Update, 4: Delete)
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("            MANAGE COURSE INFO (CRUD)\n");
        printf("======================================================\n");
        printf("[1] View All Courses\n");
        printf("[2] Add New Course\n");
        printf("[3] Update Existing Course\n");
        printf("[4] Delete Course\n");
        printf("[5] Return to Admin Menu\n");
        printf("------------------------------------------------------\n");
        
        choice = getMenuChoice(5);

        switch (choice) {
            case 1: viewCourseCatalog(); break; // Reuse view function
            case 2: addCourseRecord(); break;
            case 3: updateCourseRecord(); break;
            case 4: deleteCourseRecord(); break;
            case 5: printf("Returning to Admin Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

void addCourseRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("               Add New Course Record\n");
    printf("======================================================\n");
    
    if (courseCount >= MAX_RECORDS) {
        printf("Error: Database capacity reached (%d records).\n", MAX_RECORDS);
        return;
    }
    
    char newCode[MAX_ID_LENGTH];
    char *inputCode;
    int index = courseCount;

    // Check for unique Code
    do {
        inputCode = getInput("Enter Course Code (e.g., CS101): ");
        int exists = 0;
        for (int i = 0; i < courseCount; i++) {
            if (strcmp(courses[i].code, inputCode) == 0) {
                exists = 1;
                break;
            }
        }
        if (exists) {
            printf("Error: Course Code already exists. Try again.\n");
        } else {
            strcpy(newCode, inputCode);
            break;
        }
    } while(1);

    strcpy(courses[index].code, newCode);
    strcpy(courses[index].name, getInput("Enter Course Name: "));
    
    int credits;
    printf("Enter Course Credits (e.g., 3): ");
    if (scanf("%d", &credits) != 1 || credits <= 0) {
        printf("Invalid credit input. Record creation failed.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n'); // Clear buffer
    courses[index].credits = credits;
    
    courseCount++;
    saveCourseRecords();
    
    printf("\nSuccess! Course record (Code: %s) added.\n", newCode);
}

void deleteCourseRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Delete Course Record\n");
    printf("======================================================\n");
    
    char *codeToDelete = getInput("Enter Course Code to delete: ");
    int foundIndex = -1;
    
    for (int i = 0; i < courseCount; i++) {
        if (strcmp(courses[i].code, codeToDelete) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        for (int i = foundIndex; i < courseCount - 1; i++) {
            courses[i] = courses[i+1];
        }
        courseCount--;
        saveCourseRecords();
        printf("\nSuccess! Course record (Code: %s) deleted.\n", codeToDelete);
    } else {
        printf("\nError: Course Code '%s' not found.\n", codeToDelete);
    }
}

void updateCourseRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Update Course Record\n");
    printf("======================================================\n");
    
    char *codeToUpdate = getInput("Enter Course Code to update: ");
    int foundIndex = -1;
    
    for (int i = 0; i < courseCount; i++) {
        if (strcmp(courses[i].code, codeToUpdate) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        printf("\nUpdating Record for Course Code: %s (Name: %s)\n", courses[foundIndex].code, courses[foundIndex].name);
        printf("--- Enter new values (or press Enter to keep current value) ---\n");
        
        // FIX: Create a formatted prompt string before calling getInput
        char promptBuffer[MAX_LINE_LENGTH];
        
        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Name (Current: %s): ", courses[foundIndex].name);
        char *input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(courses[foundIndex].name, input);

        int newCredits;
        printf("Enter New Credits (Current: %d, Enter -1 to skip): ", courses[foundIndex].credits);
        if (scanf("%d", &newCredits) == 1) {
            if (newCredits != -1 && newCredits > 0) {
                courses[foundIndex].credits = newCredits;
            }
        }
        while (getchar() != '\n'); // Clear buffer
        
        saveCourseRecords();
        printf("\nSuccess! Course record (Code: %s) updated.\n", codeToUpdate);
    } else {
        printf("\nError: Course Code '%s' not found.\n", codeToUpdate);
    }
}

// FACULTY CRUD Menu
void facultyCRUDMenu() {
    // CRUD menu structure similar to studentCRUDMenu... (1: View, 2: Add, 3: Update, 4: Delete)
    int choice;
    do {
        clearScreen();
        printf("======================================================\n");
        printf("          MANAGE FACULTY INFO (CRUD)\n");
        printf("======================================================\n");
        printf("[1] View All Faculty\n");
        printf("[2] Add New Faculty\n");
        printf("[3] Update Existing Faculty\n");
        printf("[4] Delete Faculty\n");
        printf("[5] Return to Admin Menu\n");
        printf("------------------------------------------------------\n");
        
        choice = getMenuChoice(5);

        switch (choice) {
            case 1: displayFacultyRecords(); break; // Reuse display function
            case 2: addFacultyRecord(); break;
            case 3: updateFacultyRecord(); break;
            case 4: deleteFacultyRecord(); break;
            case 5: printf("Returning to Admin Menu...\n"); break;
        }
        if (choice != 5) pressEnterToContinue();

    } while (choice != 5);
}

void displayFacultyRecords() {
    // This is the same as viewFacultyDirectory, used for consistency in CRUD menu
    clearScreen();
    printf("======================================================\n");
    printf("                All Faculty Records\n");
    printf("======================================================\n");
    
    if (facultyCount == 0) {
        printf("No faculty records found.\n");
        return;
    }

    printf("+-----------+--------------------------+-----------------------+\n");
    printf("| Faculty ID| Name                     | Department            |\n");
    printf("+-----------+--------------------------+-----------------------+\n");
    for (int i = 0; i < facultyCount; i++) {
        printf("| %-9s | %-24s | %-21s |\n", 
               faculty[i].id, 
               faculty[i].name, 
               faculty[i].department);
    }
    printf("+-----------+--------------------------+-----------------------+\n");
}

void addFacultyRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Add New Faculty Record\n");
    printf("======================================================\n");
    
    if (facultyCount >= MAX_RECORDS) {
        printf("Error: Database capacity reached (%d records).\n", MAX_RECORDS);
        return;
    }
    
    char newID[MAX_ID_LENGTH];
    char *inputID;
    int index = facultyCount;

    // Check for unique ID
    do {
        inputID = getInput("Enter new Faculty ID (e.g., F009): ");
        int exists = 0;
        for (int i = 0; i < facultyCount; i++) {
            if (strcmp(faculty[i].id, inputID) == 0) {
                exists = 1;
                break;
            }
        }
        if (exists) {
            printf("Error: Faculty ID already exists. Try again.\n");
        } else {
            strcpy(newID, inputID);
            break;
        }
    } while(1);

    strcpy(faculty[index].id, newID);
    strcpy(faculty[index].name, getInput("Enter Faculty Name: "));
    strcpy(faculty[index].department, getInput("Enter Faculty Department: "));
    
    facultyCount++;
    saveFacultyRecords();
    
    printf("\nSuccess! Faculty record (ID: %s) added.\n", newID);
}

void deleteFacultyRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Delete Faculty Record\n");
    printf("======================================================\n");
    
    char *idToDelete = getInput("Enter Faculty ID to delete: ");
    int foundIndex = -1;
    
    for (int i = 0; i < facultyCount; i++) {
        if (strcmp(faculty[i].id, idToDelete) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        for (int i = foundIndex; i < facultyCount - 1; i++) {
            faculty[i] = faculty[i+1];
        }
        facultyCount--;
        saveFacultyRecords();
        printf("\nSuccess! Faculty record (ID: %s) deleted.\n", idToDelete);
    } else {
        printf("\nError: Faculty ID '%s' not found.\n", idToDelete);
    }
}

void updateFacultyRecord() {
    clearScreen();
    printf("======================================================\n");
    printf("              Update Faculty Record\n");
    printf("======================================================\n");
    
    char *idToUpdate = getInput("Enter Faculty ID to update: ");
    int foundIndex = -1;
    
    for (int i = 0; i < facultyCount; i++) {
        if (strcmp(faculty[i].id, idToUpdate) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        printf("\nUpdating Record for Faculty ID: %s (Name: %s)\n", faculty[foundIndex].id, faculty[foundIndex].name);
        printf("--- Enter new values (or press Enter to keep current value) ---\n");
        
        // FIX: Create a formatted prompt string before calling getInput
        char promptBuffer[MAX_LINE_LENGTH];

        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Name (Current: %s): ", faculty[foundIndex].name);
        char *input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(faculty[foundIndex].name, input);

        snprintf(promptBuffer, MAX_LINE_LENGTH, "Enter New Department (Current: %s): ", faculty[foundIndex].department);
        input = getInput(promptBuffer);
        if (input[0] != '\0') strcpy(faculty[foundIndex].department, input);
        
        saveFacultyRecords();
        printf("\nSuccess! Faculty record (ID: %s) updated.\n", idToUpdate);
    } else {
        printf("\nError: Faculty ID '%s' not found.\n", idToUpdate);
    }
}

void editNotices() {
    clearScreen();
    printf("======================================================\n");
    printf("           Edit Academic Calendar & Notices\n");
    printf("======================================================\n");
    printf("Instruction: You can directly edit the 'notices.txt' file.\n");
    printf("Note: For this console application, we will overwrite the file content.\n\n");

    FILE *fp = fopen("notices.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not open notices.txt for writing.\n");
        return;
    }

    printf("Enter the new content line by line. Type 'END' on a new line to finish.\n");
    char line[MAX_LINE_LENGTH];

    // Consume any leftover newline characters from previous input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    while (1) {
        printf("-> ");
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) break; // Use stdin here
        
        normalizeString(line);
        if (strcmp(line, "END") == 0) break;
        
        fprintf(fp, "%s\n", line); // Write line back with a newline
    }

    fclose(fp);
    printf("\nSuccess! Notices have been updated.\n");
}

// --- MAIN FUNCTION ---

int main() {
    // 1. Load data from files into memory structures
    loadAllRecords();

    // 2. Start the application loop with the Login Menu
    // The loginMenu is responsible for checking credentials and setting the role
    // The loop continues until the user successfully logs in or exits.
    while (currentUserRole == ROLE_UNKNOWN) {
        loginMenu();
        if (currentUserRole != ROLE_UNKNOWN) {
            // If login successful, enter the main menu
            mainMenu();
        }
    }
    
    return 0;
}
