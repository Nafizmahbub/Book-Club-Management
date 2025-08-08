#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_ROOMS 3
#define NUM_RESOURCES 5
#define NUM_CLUBS 4

typedef struct {
    int id;
    int available;
    int tables;   
    int chairs;   
} Room;

typedef struct {
    int id;
    int available;
} Resource;

Room rooms[NUM_ROOMS];
Resource resources[NUM_RESOURCES];
pthread_mutex_t room_lock;
pthread_mutex_t resource_lock;
pthread_mutex_t catalog_lock;  
sem_t available_rooms;

void* book_club_activity(void* arg) {
    int club_id = *(int*)arg;

    sem_wait(&available_rooms);

    pthread_mutex_lock(&room_lock);
    int room_id = -1;
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (rooms[i].available) {
            rooms[i].available = 0;
            room_id = rooms[i].id;
            printf("Book club %d reserved room %d with %d tables and %d chairs\n", 
                   club_id, room_id, rooms[i].tables, rooms[i].chairs);
           
            rooms[i].tables = 0;
            rooms[i].chairs = 0;
            break;
        }
    }
    pthread_mutex_unlock(&room_lock);

  
    pthread_mutex_lock(&resource_lock);
    printf("Book club %d allocating resources:\n", club_id);
    for (int i = 0; i < NUM_RESOURCES; i++) {
        if (resources[i].available) {
            resources[i].available = 0;
            printf("  Resource %d allocated\n", resources[i].id);
        }
    }
    pthread_mutex_unlock(&resource_lock);

    
    sleep(2);


    pthread_mutex_lock(&resource_lock);
    printf("Book club %d releasing resources:\n", club_id);
    for (int i = 0; i < NUM_RESOURCES; i++) {
        if (!resources[i].available) {
            resources[i].available = 1;
            printf("  Resource %d released\n", resources[i].id);
        }
    }
    pthread_mutex_unlock(&resource_lock);

   
    pthread_mutex_lock(&room_lock);
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (rooms[i].id == room_id) {
            rooms[i].available = 1;
            rooms[i].tables = 3;  
            rooms[i].chairs = 10; 
            printf("Book club %d released room %d and restored tables and chairs\n", 
                   club_id, room_id);
            break;
        }
    }
    pthread_mutex_unlock(&room_lock);

    sem_post(&available_rooms);

    pthread_mutex_lock(&catalog_lock);
    printf("Book club %d updating catalog after activity\n", club_id);
    pthread_mutex_unlock(&catalog_lock);

    pthread_exit(NULL);
}

int main() {
   
    for (int i = 0; i < NUM_ROOMS; i++) {
        rooms[i].id = i + 1;
        rooms[i].available = 1;
        rooms[i].tables = 3; 
        rooms[i].chairs = 10;  
    }
    for (int i = 0; i < NUM_RESOURCES; i++) {
        resources[i].id = i + 1;
        resources[i].available = 1;
    }

    pthread_mutex_init(&room_lock, NULL);
    pthread_mutex_init(&resource_lock, NULL);
    pthread_mutex_init(&catalog_lock, NULL);
    sem_init(&available_rooms, 0, NUM_ROOMS); 

   
    pthread_t threads[NUM_CLUBS];
    int club_ids[NUM_CLUBS];
    for (int i = 0; i < NUM_CLUBS; i++) {
        club_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, book_club_activity, &club_ids[i]);
    }

   
    for (int i = 0; i < NUM_CLUBS; i++) {
        pthread_join(threads[i], NULL);
    }

 
    pthread_mutex_destroy(&room_lock);
    pthread_mutex_destroy(&resource_lock);
    pthread_mutex_destroy(&catalog_lock);
    sem_destroy(&available_rooms);

    printf("All book clubs have completed their activities.\n");
    return 0;
}
