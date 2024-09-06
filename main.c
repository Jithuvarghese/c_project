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

int main() {
    int choice;
    int keepRunning = 1; // Flag to control the program flow
    int isAdmin = 0;      // Admin or User

    printf("Are you an Admin or User?\n1. Admin\n2. User\n");
    scanf("%d", &isAdmin);

    if (isAdmin == 1) {
        // Admin registration and login flow
        printf("1. Register as Admin\n2. Login as Admin\n");
        scanf("%d", &choice);

        if (choice == 1) {
            registerAdmin();
        } else if (choice == 2) {
            if (loginAdmin()) {
                adminMenu();  // Access admin options
            } else {
                printf("Invalid admin credentials.\n");
            }
        } else {
            printf("Invalid choice.\n");
        }
    } else {
        // Load the slot states from file for users
        loadSlotStates();

        while (keepRunning) {
            printf("\n1. Register\n");
            printf("2. Login\n");
            printf("3. Exit\n");  // Option to exit
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    registerUser();
                    break;
                case 2:
                    if (loginUser()) {
                        printf("Login successful!\n");
                        evChargerMenu();  // Forward to EV charger menu after successful login
                    } else {
                        printf("Invalid username or password.\n");
                    }
                    break;
                case 3:
                    printf("Exiting the program...\n");
                    // Save the slot states to file
                    saveSlotStates();
                    keepRunning = 0;  // Exit the loop
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
            }
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

    // Store full user details in the file
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

    // Store admin details in the admin file
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
    int keepRunning = 1;

    while (keepRunning) {
        printf("\nAdmin Menu:\n");
        printf("1. View Charging Report\n");
        printf("2. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewReport();  // View report of completed charging sessions
                break;
            case 2:
                printf("Exiting Admin Menu...\n");
                keepRunning = 0;  // Exit admin menu loop
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}


// Function to generate a report of completed charging sessions
void viewReport() {
    FILE *fp;
    char buffer[MAX];
    int vehicleCount = 0;
    double totalRevenue = 0;
    char username[MAX], vehicle[MAX];
    int slotID;
    double timeSpent, cost;

    // Open the completed sessions file
    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL) {
        perror("Error opening session file");
        return;
    }

    printf("\nCompleted Charging Sessions Report:\n");
    printf("--------------------------------------------------\n");
    printf("User\t\tVehicle\t\tSlot\tTime\tCost\n");
    printf("--------------------------------------------------\n");

    // Read each line from the file and parse the information
    while (fgets(buffer, sizeof(buffer), fp)) {
        // Use sscanf to extract data from each line
        if (sscanf(buffer, "User: %[^,], Vehicle: %[^,], Slot: %d, Time: %lf seconds, Cost: $%lf",
                   username, vehicle, &slotID, &timeSpent, &cost) == 5) {
            printf("%s\t%s\t%d\t%.0f seconds\t$%.2f\n", username, vehicle, slotID, timeSpent, cost);
            vehicleCount++;
            totalRevenue += cost;
        } else {
            printf("Error parsing line: %s", buffer);  // Handle any parsing errors
        }
    }

    printf("--------------------------------------------------\n");
    printf("Total vehicles charged: %d\n", vehicleCount);
    printf("Total revenue: $%.2f\n", totalRevenue);

    fclose(fp);
}



//--------------------------------user functions-----------------------------//



// Function to log in a user by checking username and password
int loginUser() {
    FILE *fp;
    char username[MAX], password[MAX];
    int found = 0;

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

    while (fscanf(fp, "%s %s %s %s", currentUser.username, currentUser.password, currentUser.vehicle_type, currentUser.email) != EOF) {
        if (strcmp(username, currentUser.username) == 0 && strcmp(password, currentUser.password) == 0) {
            found = 1;
            break;
        }
    }

    fclose(fp);

    // Check if this user has an already booked slot
    bookedSlotID = -1;  // Reset bookedSlotID before checking
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (strcmp(slots[i].bookedBy, currentUser.username) == 0) {
            bookedSlotID = slots[i].slotID;  // Assign the booked slot ID to the user
            printf("You have previously booked Slot %d.\n", bookedSlotID);
            break;
        }
    }

    return found;
}


// EV Charger Menu
void evChargerMenu() {
    int choice;
    int keepRunning = 1;  // Flag to control the EV charger menu flow

    while (keepRunning) {
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
                printf("Exiting EV Charger Menu...\n");
                keepRunning = 0;  // Exit EV charger menu loop
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}

void saveSlotStates() {
    FILE *fp = fopen(SLOT_FILE, "wb");
    if (fp == NULL) {
        perror("Error opening slots file");
        return;
    }
    fwrite(slots, sizeof(ChargingSlot), NUM_SLOTS, fp);
    fclose(fp);
}

// Load slot states from file
void loadSlotStates() {
    FILE *fp = fopen(SLOT_FILE, "rb");
    if (fp == NULL) {
        // If the file does not exist, initialize slots
        initializeSlots();
        return;
    }
    fread(slots, sizeof(ChargingSlot), NUM_SLOTS, fp);
    fclose(fp);
}

// Initialize charging slots to available (free)
void initializeSlots() {
    for (int i = 0; i < NUM_SLOTS; i++) {
        slots[i].slotID = i + 1;  // Slot IDs start from 1
        slots[i].isAvailable = 1; // Mark all slots as available initially
        slots[i].startTime = 0;   // Initialize startTime to 0
        slots[i].bookedBy[0] = '\0';  // Initialize the bookedBy field to empty
    }
}

// Check for charger availability
void check_charger_availability() {
    printf("\nCharging Slot Availability:\n");
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slots[i].isAvailable) {
            printf("Slot %d is available.\n", slots[i].slotID);
        } else {
            printf("Slot %d is booked by %s.\n", slots[i].slotID, slots[i].bookedBy);
        }
    }
}

// Book a charging slot
int book_charging_slot() {
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slots[i].isAvailable) {
            slots[i].isAvailable = 0;  // Mark slot as booked
            slots[i].startTime = 0;    // Reset the start time
            strcpy(slots[i].bookedBy, currentUser.username);  // Track who booked the slot
            printf("Slot %d has been successfully booked!\n", slots[i].slotID);
            return slots[i].slotID;
        }
    }
    printf("No slots available for booking.\n");
    return -1;  // Return -1 if no slots are available
}

// Start charging at a booked slot
void start_charging(int slotID) {
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slots[i].slotID == slotID && !slots[i].isAvailable && strcmp(slots[i].bookedBy, currentUser.username) == 0) {
            slots[i].startTime = time(NULL);  // Record the start time
            printf("Charging started at slot %d.\n", slotID);
            return;
        }
    }
    printf("Error: Slot %d is not booked by you or is not available.\n", slotID);
}

// Stop charging, calculate the cost, and store the completed session in a file
void stop_charging(int slotID) {
    double ratePerSecond = 0.05;  // Charging rate (e.g., $0.05 per second)
    FILE *fp;

    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slots[i].slotID == slotID && !slots[i].isAvailable && strcmp(slots[i].bookedBy, currentUser.username) == 0) {
            time_t stopTime = time(NULL);
            double timeSpent = difftime(stopTime, slots[i].startTime);
            double expense = timeSpent * ratePerSecond;

            // Free the slot after charging is complete
            slots[i].isAvailable = 1;
            slots[i].startTime = 0;
            slots[i].bookedBy[0] = '\0';  // Clear the booking username

            printf("Charging stopped at slot %d.\n", slotID);
            printf("Time spent: %.0f seconds\n", timeSpent);
            printf("Total cost: $%.2f\n", expense);

            // Log the completed session details to a file
            fp = fopen(SESSION_FILE, "a");
            if (fp == NULL) {
                perror("Error opening session file");
                return;
            }

            // Log the session information: username, vehicle, slot ID, time spent, cost
            fprintf(fp, "User: %s, Vehicle: %s, Slot: %d, Time: %.0f seconds, Cost: $%.2f\n",
                    currentUser.username, currentUser.vehicle_type, slotID, timeSpent, expense);

            fclose(fp);

            // Save the updated slot states to the file
            saveSlotStates();
            return;
        }
    }
    printf("Error: Slot %d is not currently charging by you.\n", slotID);
}