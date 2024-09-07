#include <stdio.h>
#include <string.h>
#include <time.h>

#define FILENAME "user1.txt"
#define ADMIN_FILE "admin.txt"
#define SESSION_FILE "completed_sessions.txt"  // File for completed charging sessions
#define SLOT_FILE "slots.dat"
#define MAX 100
#define NUM_SLOTS 5

typedef struct {
    char username[MAX];
    char vehicle[MAX];
    int slotID;
    double timeSpent;
    double cost;
} ChargingSession;

typedef struct {
    char username[MAX];
    char password[MAX];
    char vehicle_type[MAX];
    char email[MAX];
} User;

typedef struct {
    int slotID;
    int isAvailable;  // 1 for available, 0 for booked
    time_t startTime;
    char bookedBy[MAX];  // Track the username who booked the slot
} ChargingSlot;

ChargingSlot slots[NUM_SLOTS];  // Array to store charging slots
User currentUser;               // Store logged-in user details
int bookedSlotID = -1;          // Track the ID of the currently booked slot

// Function declarations
void registerUser();
int loginUser();
void evChargerMenu();
void initializeSlots();
void check_charger_availability();
int book_charging_slot();
void start_charging(int slotID);
void stop_charging(int slotID);
void loadSlotStates();
void saveSlotStates();
void adminMenu();
void viewReport();
void registerAdmin();
int loginAdmin();
void sortSessionsByCost(ChargingSession *sessions, int n);

int main() {
    int isAdmin = 0;     // Admin or User

    loadSlotStates(); // Load slot states at the start

    while (1) {  // Main loop
        printf("Are you an Admin or User?\n1. Admin\n2. User\n3. Exit\n");
        scanf("%d", &isAdmin);

        if (isAdmin == 1) {
            int adminChoice;
            while (1) {
                printf("1. Register as Admin\n2. Login as Admin\n3. Exit\n");
                scanf("%d", &adminChoice);

                switch (adminChoice) {
                    case 1:
                        registerAdmin();
                        break;
                    case 2:
                        if (loginAdmin()) {
                            adminMenu();
                        } else {
                            printf("Invalid admin credentials.\n");
                        }
                        break;
                    case 3:
                        printf("Returning to the main menu...\n");
                        break;
                    default:
                        printf("Invalid choice.\n");
                }
                if (adminChoice == 3) break;
            }
        } else if (isAdmin == 2) {
            int userChoice;
            while (1) {
                printf("\n1. Register\n2. Login\n3. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &userChoice);

                switch (userChoice) {
                    case 1:
                        registerUser();
                        break;
                    case 2:
                        if (loginUser()) {
                            printf("Login successful!\n");
                            evChargerMenu();
                        } else {
                            printf("Invalid username or password.\n");
                        }
                        break;
                    case 3:
                        printf("Returning to the main menu...\n");
                        break;
                    default:
                        printf("Invalid choice. Please try again.\n");
                }
                if (userChoice == 3) break;
            }
        } else if (isAdmin == 3) {
            printf("Exiting the program...\n");
            saveSlotStates();
            return 0;
        } else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}

// Function to register a user and store their details in a file
void registerUser() {
    FILE *fp;
    User user;

    printf("\n-----------------\n");
    printf("Enter username: ");
    scanf("%s", user.username);
    printf("Enter vehicle type: ");
    scanf("%s", user.vehicle_type);
    printf("Enter email: ");
    scanf("%s", user.email);
    printf("Enter password: ");
    scanf("%s", user.password);
    printf("\n-----------------\n");

    fp = fopen(FILENAME, "a");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(fp, "%s %s %s %s\n", user.username, user.password, user.vehicle_type, user.email);
    fclose(fp);

    printf("\nRegistration successful!\n");
}

// Function to register an admin
void registerAdmin() {
    FILE *fp;
    User admin;

    printf("\n-----------------\n");
    printf("Enter admin username: ");
    scanf("%s", admin.username);
    printf("Enter admin password: ");
    scanf("%s", admin.password);
    printf("\n-----------------\n");

    fp = fopen(ADMIN_FILE, "a");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(fp, "%s %s\n", admin.username, admin.password);
    fclose(fp);

    printf("\nAdmin registration successful!\n");
}

// Function to log in an admin
int loginAdmin() {
    FILE *fp;
    char username[MAX], password[MAX];
    int found = 0;
    User admin;

    printf("\n-----------------\n");
    printf("Enter admin username: ");
    scanf("%s", username);
    printf("Enter admin password: ");
    scanf("%s", password);
    printf("\n-----------------\n");

    fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) {
        perror("Error opening admin file");
        return 0;
    }

    while (fscanf(fp, "%s %s", admin.username, admin.password) != EOF) {
        if (strcmp(username, admin.username) == 0 && strcmp(password, admin.password) == 0) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

// Admin menu for viewing reports
void adminMenu() {
    int choice;
    while (1) {
        printf("\nAdmin Menu:\n");
        printf("1. View Charging Report\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewReport();
                break;
            case 2:
                printf("Returning to the main menu...\n");
                return;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

// Function to generate a report of completed charging sessions, sorted by cost
void viewReport() {
    FILE *fp;
    ChargingSession sessions[MAX];
    int count = 0;
    char buffer[MAX];
    char username[MAX], vehicle[MAX];
    int slotID;
    double timeSpent, cost;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL) {
        perror("Error opening session file");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "User: %[^,], Vehicle: %[^,], Slot: %d, Time: %lf seconds, Cost: $%lf",
                   username, vehicle, &slotID, &timeSpent, &cost) == 5) {
            strcpy(sessions[count].username, username);
            strcpy(sessions[count].vehicle, vehicle);
            sessions[count].slotID = slotID;
            sessions[count].timeSpent = timeSpent;
            sessions[count].cost = cost;
            count++;
        } else {
            printf("Error parsing line: %s", buffer);
        }
    }
    fclose(fp);

    // Sort the sessions in descending order of cost
    sortSessionsByCost(sessions, count);

    // Display the sorted report
    printf("\nCompleted Charging Sessions Report (sorted by cost):\n");
    printf("--------------------------------------------------\n");
    printf("User\t\tVehicle\t\tSlot\tCost\tTime\n");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%s\t%s\t%d\t$%.2f\t%.0f seconds\n",
               sessions[i].username, sessions[i].vehicle, sessions[i].slotID,
               sessions[i].cost, sessions[i].timeSpent);
    }
    printf("--------------------------------------------------\n");
}

// Function to sort charging sessions in descending order of cost
void sortSessionsByCost(ChargingSession *sessions, int n) {
    ChargingSession temp;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (sessions[j].cost < sessions[j + 1].cost) {
                temp = sessions[j];
                sessions[j] = sessions[j + 1];
                sessions[j + 1] = temp;
            }
        }
    }
}

// Function to log in a user
int loginUser() {
    FILE *fp;
    char username[MAX], password[MAX];
    int found = 0;
    User user;

    printf("\n-----------------\n");
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    printf("\n-----------------\n");

    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 0;
    }

    while (fscanf(fp, "%s %s %s %s", user.username, user.password, user.vehicle_type, user.email) != EOF) {
        if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0) {
            currentUser = user;  // Store the logged-in user details
            found = 1;
            break;
        }
    }

    fclose(fp);

    // Restore the slot booked by the user, if any
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slots[i].isAvailable == 0 && strcmp(slots[i].bookedBy, currentUser.username) == 0) {
            bookedSlotID = slots[i].slotID;  // Restore their booked slot ID
            break;
        }
    }

    return found;
}

// Function to initialize the charging slots at the start of the program
void initializeSlots() {
    for (int i = 0; i < NUM_SLOTS; i++) {
        slots[i].slotID = i + 1;
        slots[i].isAvailable = 1;  // All slots are available initially
        slots[i].startTime = 0;
        strcpy(slots[i].bookedBy, "");  // No one has booked the slot
    }

    saveSlotStates();  // Save the initial slot states to a file
}

// Function to check charger availability
void check_charger_availability() {
    printf("\nAvailable Charging Slots:\n");
    printf("Slot ID\tStatus\n");
    for (int i = 0; i < NUM_SLOTS; i++) {
        printf("%d\t%s\n", slots[i].slotID, slots[i].isAvailable ? "Available" : "Booked");
    }
}

// Function to book a charging slot for the user
int book_charging_slot() {
    if (bookedSlotID != -1) {
        printf("You have already booked slot %d.\n", bookedSlotID);
        return bookedSlotID;
    }

    int slotChoice;
    check_charger_availability();
    printf("\nEnter slot ID to book: ");
    scanf("%d", &slotChoice);

    if (slotChoice < 1 || slotChoice > NUM_SLOTS || !slots[slotChoice - 1].isAvailable) {
        printf("Invalid slot choice or slot already booked.\n");
        return -1;
    }

    // Mark the slot as booked
    slots[slotChoice - 1].isAvailable = 0;
    strcpy(slots[slotChoice - 1].bookedBy, currentUser.username);
    printf("Slot %d booked successfully!\n", slotChoice);

    saveSlotStates();  // Save updated slot states
    return slotChoice;  // Return the booked slot ID
}

// Function to start charging at a booked slot
void start_charging(int slotID) {
    time(&slots[slotID - 1].startTime);  // Record the current time as the start time
    printf("Charging started at slot %d...\n", slotID);
}

// Function to stop charging at a booked slot and calculate cost
void stop_charging(int slotID) {
    time_t stopTime;
    time(&stopTime);  // Record the current time as the stop time
    double timeSpent = difftime(stopTime, slots[slotID - 1].startTime);  // Calculate the time spent charging

    // Calculate cost based on charging time (e.g., $0.05 per second)
    double cost = timeSpent * 0.05;

    printf("Charging stopped at slot %d. Total time: %.0f seconds. Total cost: $%.2f\n",
           slotID, timeSpent, cost);

    // Log the session to the completed sessions file
    FILE *fp = fopen(SESSION_FILE, "a");
    if (fp == NULL) {
        perror("Error opening session file");
        return;
    }

    fprintf(fp, "User: %s, Vehicle: %s, Slot: %d, Time: %.0f seconds, Cost: $%.2f\n",
            currentUser.username, currentUser.vehicle_type, slotID, timeSpent, cost);
    fclose(fp);

    // Mark the slot as available again
    slots[slotID - 1].isAvailable = 1;
    slots[slotID - 1].startTime = 0;
    strcpy(slots[slotID - 1].bookedBy, "");  // Clear the bookedBy field

    saveSlotStates();  // Save updated slot states
}

// Function to save the states of all charging slots to a file
void saveSlotStates() {
    FILE *fp = fopen(SLOT_FILE, "wb");
    if (fp == NULL) {
        perror("Error opening slot file");
        return;
    }

    fwrite(slots, sizeof(ChargingSlot), NUM_SLOTS, fp);
    fclose(fp);
}

// Function to load the states of all charging slots from a file
void loadSlotStates() {
    FILE *fp = fopen(SLOT_FILE, "rb");
    if (fp == NULL) {
        // If the file does not exist, initialize the slots
        initializeSlots();
        return;
    }

    fread(slots, sizeof(ChargingSlot), NUM_SLOTS, fp);
    fclose(fp);
}

// EV charger menu
void evChargerMenu() {
    int choice;
    while (1) {
        printf("\nEV Charger Menu:\n");
        printf("1. Check for charger availability\n");
        printf("2. Book charging slot\n");
        printf("3. Start charging\n");
        printf("4. Stop charging\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                check_charger_availability();
                break;
            case 2:
                bookedSlotID = book_charging_slot();  // Assign the booked slot ID
                break;
            case 3:
                if (bookedSlotID != -1) {
                    start_charging(bookedSlotID);  // Allow charging if a slot is booked
                } else {
                    printf("Please book a slot first!\n");
                }
                break;
            case 4:
                if (bookedSlotID != -1) {
                    stop_charging(bookedSlotID);  // Stop charging and release the slot
                    bookedSlotID = -1;  // Reset booked slot ID after stopping charging
                } else {
                    printf("Please start charging first!\n");
                }
                break;
            case 5:
                printf("Returning to the main menu...\n");
                return;  // Return to the main menu
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}
