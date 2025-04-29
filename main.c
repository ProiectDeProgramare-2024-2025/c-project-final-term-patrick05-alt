#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ANSI color codes
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RED "\x1b[31m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_RESET "\x1b[0m"

#define MAX_LINE 256
#define MAX_FIELD 100

#define PHOTOS_FILE "photos.txt"
#define ALBUMS_FILE "albums.txt"
#define TEMP_FILE "temp.txt"

// Struct to store photo data
struct Photo {
    char name[MAX_FIELD];
    int size;
};

// Struct to store album data
struct Album {
    char name[MAX_FIELD];
    char photos[MAX_LINE]; // Semicolon-separated photo names
};

// Function prototypes
void clear_input_buffer();
void clearScreen();
void pressKeyAndClear();
void initializePhotos();
void initializeAlbums();
void displayAvailablePhotos();
int getPhotoSize(const char *photoName);
void createAlbum();
void displayAlbums();
void addPhotoToAlbum();
void removePhotoFromAlbum();
void deleteAlbum();
void updateAlbum();
void header();
void menu(int op);

// Clears the input buffer for safe input handling.
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Clears the console screen.
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Waits for user input with a prompt and then clears the console screen.
void pressKeyAndClear() {
    printf("\nPress any key to continue...");
    clear_input_buffer();
    getchar();
    clearScreen();
}

// Prepopulates the photos file if it doesn't exist.
void initializePhotos() {
    FILE *fp = fopen(PHOTOS_FILE, "r");
    if (fp == NULL) { 
        fp = fopen(PHOTOS_FILE, "w");
        if (fp == NULL) {
            printf("Error: Cannot create %s\n", PHOTOS_FILE);
            exit(1);
        }
        fprintf(fp, "Sunset.jpg,512\n");
        fprintf(fp, "Mountains.png,1024\n");
        fprintf(fp, "Beach.bmp,750\n");
        fprintf(fp, "Cityscape.jpg,640\n");
        fprintf(fp, "Forest.png,860\n");
        fclose(fp);
        printf("'%s' not found. Prepopulated with sample photos.\n", PHOTOS_FILE);
    } else {
        fclose(fp);
    }
}

// Ensures albums file exists. If not, creates an empty file.
void initializeAlbums() {
    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (fp == NULL) { 
        fp = fopen(ALBUMS_FILE, "w");
        if (fp == NULL) {
            printf("Error: Cannot create %s\n", ALBUMS_FILE);
            exit(1);
        }
        fclose(fp);
        printf("'%s' not found. Created empty albums file.\n", ALBUMS_FILE);
    } else {
        fclose(fp);
    }
}

// Displays the available photos list with colored photo names.
void displayAvailablePhotos() {
    FILE *fp = fopen(PHOTOS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open %s\n", PHOTOS_FILE);
        return;
    }

    char line[MAX_LINE];
    printf("\nAvailable Photos:\n");
    printf("%-20s %-10s\n", "Photo Name", "Size (KB)");
    printf("-------------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        char photoName[MAX_FIELD];
        int size;
        if (sscanf(line, "%[^,],%d", photoName, &size) == 2)
            printf("%s%-20s%s %-10d\n", COLOR_GREEN, photoName, COLOR_RESET, size);
    }
    printf("-------------------------------------\n");
    fclose(fp);
}

// Looks up a photo's size from the photos file.
int getPhotoSize(const char *photoName) {
    FILE *fp = fopen(PHOTOS_FILE, "r");
    if (!fp)
        return 0;

    char line[MAX_LINE], name[MAX_FIELD];
    int size = 0;
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%[^,],%d", name, &size) == 2) {
            if (strcmp(name, photoName) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found ? size : 0;
}

// Creates a new album with an empty photo list.
void createAlbum() {
    clearScreen();
    char albumName[MAX_FIELD];
    printf("Enter new album name: ");
    scanf("%99s", albumName);
    clear_input_buffer();

    if (strlen(albumName) == 0) {
        printf("Error: Album name cannot be empty.\n");
        pressKeyAndClear();
        return;
    }

    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (fp) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), fp)) {
            char existingAlbum[MAX_FIELD];
            if (sscanf(line, "%[^,]", existingAlbum) == 1) {
                if (strcmp(existingAlbum, albumName) == 0) {
                    printf("An album with name '%s' already exists.\n", albumName);
                    fclose(fp);
                    pressKeyAndClear();
                    return;
                }
            }
        }
        fclose(fp);
    }

    fp = fopen(ALBUMS_FILE, "a");
    if (!fp) {
        printf("Error: Cannot open album file for writing.\n");
        pressKeyAndClear();
        return;
    }

    fprintf(fp, "%s,\n", albumName);
    fclose(fp);
    printf("✅ Album '%s' created.\n", albumName);
    pressKeyAndClear();
}

// Displays all albums, their photos, and total size with colored elements.
void displayAlbums() {
    clearScreen();
    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open album file for reading.\n");
        pressKeyAndClear();
        return;
    }

    char line[MAX_LINE];
    printf("\n%-20s %-30s %-10s\n", "Album Name", "Photos", "Total Size(KB)");
    printf("-----------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        char albumName[MAX_FIELD];
        char photosList[MAX_LINE];
        if (sscanf(line, "%[^,],%[^\n]", albumName, photosList) < 1)
            continue;
        int totalSize = 0;
        if (strlen(photosList) > 0) {
            char tempList[MAX_LINE];
            strcpy(tempList, photosList);
            char *token = strtok(tempList, ";");
            while (token != NULL) {
                int size = getPhotoSize(token);
                totalSize += size;
                token = strtok(NULL, ";");
            }
        }
        char photosForDisplay[MAX_LINE] = "";
        char *commaPos = strchr(line, ',');
        if (commaPos && strlen(commaPos + 1) > 0)
            strncpy(photosForDisplay, commaPos + 1, sizeof(photosForDisplay));
        photosForDisplay[strcspn(photosForDisplay, "\n")] = '\0';
        if (strlen(photosForDisplay) == 0)
            strcpy(photosForDisplay, "None");

        printf("%s%-20s%s %-30s %s%-10d%s\n", COLOR_BLUE, albumName, COLOR_RESET, photosForDisplay, COLOR_RED, totalSize, COLOR_RESET);
    }
    printf("-----------------------------------------------------------------------\n");
    fclose(fp);
    pressKeyAndClear();
}

// Adds a photo to an existing album.
void addPhotoToAlbum() {
    clearScreen();
    char albumName[MAX_FIELD];
    printf("Enter album name to modify: ");
    scanf("%99s", albumName);
    clear_input_buffer();

    if (strlen(albumName) == 0) {
        printf("Error: Album name cannot be empty.\n");
        pressKeyAndClear();
        return;
    }

    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open album file for reading.\n");
        pressKeyAndClear();
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        printf("Error: Cannot open temporary file.\n");
        fclose(fp);
        pressKeyAndClear();
        return;
    }
    
    int foundAlbum = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp)) {
        char currentAlbum[MAX_FIELD];
        char photosList[MAX_LINE] = "";
        if (sscanf(line, "%[^,],%[^\n]", currentAlbum, photosList) < 1) {
            fprintf(temp, "%s", line);
            continue;
        }

        if (strcmp(currentAlbum, albumName) == 0) {
            foundAlbum = 1;
            displayAvailablePhotos();
            char photoName[MAX_FIELD];
            printf("Enter photo name to add: ");
            scanf("%99s", photoName);
            clear_input_buffer();
            
            if (strlen(photoName) == 0) {
                printf("Error: Photo name cannot be empty.\n");
                fprintf(temp, "%s", line);
            }
            else if (getPhotoSize(photoName) == 0) {
                printf("Photo '%s' not found in available photos.\n", photoName);
                fprintf(temp, "%s", line);
            }
            else {
                char updatedLine[MAX_LINE] = "";
                if (strlen(photosList) > 0)
                    snprintf(updatedLine, sizeof(updatedLine), "%s,%s;%s\n", currentAlbum, photosList, photoName);
                else
                    snprintf(updatedLine, sizeof(updatedLine), "%s,%s\n", currentAlbum, photoName);
                printf("✅ Added photo '%s' to album '%s'.\n", photoName, currentAlbum);
                fprintf(temp, "%s", updatedLine);
            }
        } else {
            fprintf(temp, "%s", line);
        }
    }
    fclose(fp);
    fclose(temp);

    if (!foundAlbum) {
        printf("Album '%s' not found.\n", albumName);
        remove(TEMP_FILE);
    } else {
        remove(ALBUMS_FILE);
        rename(TEMP_FILE, ALBUMS_FILE);
    }
    pressKeyAndClear();
}

// Removes a photo from an album.
void removePhotoFromAlbum() {
    clearScreen();
    char albumName[MAX_FIELD];
    printf("Enter album name to modify: ");
    scanf("%99s", albumName);
    clear_input_buffer();

    if (strlen(albumName) == 0) {
        printf("Error: Album name cannot be empty.\n");
        pressKeyAndClear();
        return;
    }

    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open album file for reading.\n");
        pressKeyAndClear();
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        printf("Error: Cannot open temporary file.\n");
        fclose(fp);
        pressKeyAndClear();
        return;
    }

    char line[MAX_LINE];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char currentAlbum[MAX_FIELD];
        char photosList[MAX_LINE] = "";
        if (sscanf(line, "%[^,],%[^\n]", currentAlbum, photosList) < 1) {
            fprintf(temp, "%s", line);
            continue;
        }

        if (strcmp(currentAlbum, albumName) == 0) {
            found = 1;
            printf("Current photos: %s\n", (strlen(photosList) > 0) ? photosList : "None");
            char photoName[MAX_FIELD];
            printf("Enter photo name to remove: ");
            scanf("%99s", photoName);
            clear_input_buffer();

            char newPhotos[MAX_LINE] = "";
            char *token = strtok(photosList, ";");
            int first = 1;
            while (token != NULL) {
                if (strcmp(token, photoName) != 0) {
                    if (!first)
                        strcat(newPhotos, ";");
                    strcat(newPhotos, token);
                    first = 0;
                }
                token = strtok(NULL, ";");
            }
            fprintf(temp, "%s,%s\n", currentAlbum, newPhotos);
            printf("✅ Removed photo '%s' from album '%s'.\n", photoName, currentAlbum);
        } else {
            fprintf(temp, "%s", line);
        }
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        printf("Album '%s' not found.\n", albumName);
        remove(TEMP_FILE);
    } else {
        remove(ALBUMS_FILE);
        rename(TEMP_FILE, ALBUMS_FILE);
    }
    pressKeyAndClear();
}

// Deletes an album entirely.
void deleteAlbum() {
    clearScreen();
    char albumName[MAX_FIELD];
    printf("Enter album name to delete: ");
    scanf("%99s", albumName);
    clear_input_buffer();

    if (strlen(albumName) == 0) {
        printf("Error: Album name cannot be empty.\n");
        pressKeyAndClear();
        return;
    }

    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open album file for reading.\n");
        pressKeyAndClear();
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        printf("Error: Cannot open temporary file.\n");
        fclose(fp);
        pressKeyAndClear();
        return;
    }

    char line[MAX_LINE];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char currentAlbum[MAX_FIELD];
        if (sscanf(line, "%[^,]", currentAlbum) < 1) {
            fprintf(temp, "%s", line);
            continue;
        }
        if (strcmp(currentAlbum, albumName) != 0)
            fprintf(temp, "%s", line);
        else
            found = 1;
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        printf("Album '%s' not found.\n", albumName);
        remove(TEMP_FILE);
    } else {
        remove(ALBUMS_FILE);
        rename(TEMP_FILE, ALBUMS_FILE);
        printf("✅ Album '%s' deleted.\n", albumName);
    }
    pressKeyAndClear();
}

// Renames an album.
void updateAlbum() {
    clearScreen();
    char albumName[MAX_FIELD];
    printf("Enter album name to update: ");
    scanf("%99s", albumName);
    clear_input_buffer();

    if (strlen(albumName) == 0) {
        printf("Error: Album name cannot be empty.\n");
        pressKeyAndClear();
        return;
    }

    FILE *fp = fopen(ALBUMS_FILE, "r");
    if (!fp) {
        printf("Error: Cannot open album file for reading.\n");
        pressKeyAndClear();
        return;
    }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) {
        printf("Error: Cannot open temporary file.\n");
        fclose(fp);
        pressKeyAndClear();
        return;
    }

    char line[MAX_LINE];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char currentAlbum[MAX_FIELD];
        char photosList[MAX_LINE] = "";
        if (sscanf(line, "%[^,],%[^\n]", currentAlbum, photosList) < 1) {
            fprintf(temp, "%s", line);
            continue;
        }
        if (strcmp(currentAlbum, albumName) == 0) {
            found = 1;
            char newName[MAX_FIELD];
            printf("Enter new album name: ");
            scanf("%99s", newName);
            clear_input_buffer();
            if (strlen(newName) == 0) {
                printf("Error: New name cannot be empty. Keeping album unchanged.\n");
                fprintf(temp, "%s", line);
            } else {
                fprintf(temp, "%s,%s\n", newName, photosList);
                printf("✅ Album '%s' renamed to '%s'.\n", currentAlbum, newName);
            }
        } else {
            fprintf(temp, "%s", line);
        }
    }
    fclose(fp);
    fclose(temp);

    if (!found) {
        printf("Album '%s' not found.\n", albumName);
        remove(TEMP_FILE);
    } else {
        remove(ALBUMS_FILE);
        rename(TEMP_FILE, ALBUMS_FILE);
    }
    pressKeyAndClear();
}

// Displays the main menu options.
void header() {
    printf("\n========== Photo Gallery System ==========\n");
    printf("1 - Create Album\n");
    printf("2 - Display Albums\n");
    printf("3 - Add Photo to Album\n");
    printf("4 - Remove Photo from Album\n");
    printf("5 - Delete Album\n");
    printf("6 - Rename Album\n");
    printf("7 - Display Available Photos\n");
    printf("0 - Exit\n");
    printf("============================================\n");
}

// Processes the user’s choice.
void menu(int op) {
    switch (op) {
        case 1: createAlbum(); break;
        case 2: displayAlbums(); break;
        case 3: addPhotoToAlbum(); break;
        case 4: removePhotoFromAlbum(); break;
        case 5: deleteAlbum(); break;
        case 6: updateAlbum(); break;
        case 7: {
            clearScreen();
            displayAvailablePhotos();
            pressKeyAndClear();
            break;
        }
        default: break;
    }
}

int main() {
    clearScreen();
    initializePhotos();
    initializeAlbums();
    printf("Photo Gallery System initialized.\n");
    pressKeyAndClear();

    int option;
    do {
        header();
        printf("Enter option (0-7): ");
        scanf("%d", &option);
        clear_input_buffer();
        if (option < 0 || option > 7) {
            printf("Invalid option. Please enter a number between 0 and 7.\n");
            pressKeyAndClear();
            continue;
        }
        menu(option);
    } while (option != 0);
    printf("Exiting the Photo Gallery System.\n");
    return 0;
}