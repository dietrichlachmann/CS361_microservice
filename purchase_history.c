#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>

#define INOT_EVENT_SIZE (sizeof(struct inotify_event))

int f_des, w_des;
time_t t;

int main(){
	// INOTIFY STUFF
	char inot_buffer[64];
	int length = 0;
	int i = 0;
	// path to watch
	char path_to_watch[64];
	// file to watch
	char file_to_watch[64];

	// FILE STUFF
	FILE* f_ptr;
	char f_buff[256];
	int line_count = 1;
	float running_total = 0.00;

	// get path variables and stuff
	getcwd(path_to_watch, 64);
	strcpy(file_to_watch, "purchase_history.txt");

	// setup inotify variables
	f_des = inotify_init();
	w_des = inotify_add_watch(f_des, path_to_watch, IN_MODIFY | IN_CREATE | IN_DELETE);
	// check that path is being watched correctly
	if(w_des == -1){
		printf("Could not watch: %s\n", path_to_watch);
	}
	else{
		printf("Microservice started, watching: %s for file: %s\n", path_to_watch, file_to_watch);
	}
	
	while(1){
		i = 0;
		// get events into length
		length = read(f_des, inot_buffer, 64);

		// iterate through events
		while(i < length){
			struct inotify_event* event = (struct inotify_event*) &inot_buffer[i];
			// check for event flagged in add_watch function
			if(event->len){
				if(event->mask & IN_CREATE){	
					// if the file created is what we are looking for, read it
					if(strcmp(event->name, file_to_watch) == 0){
						//printf("File %s was created\n", event->name);
						f_ptr = fopen(file_to_watch, "r");
						if(f_ptr == NULL){
							printf("Error reading file!\n");
						}
						time(&t);
						printf("\nYour purchase history as of %s \n", ctime(&t));
						while(fgets(f_buff, sizeof(f_buff), f_ptr)){
							if(line_count % 2 != 0){
								printf("%s", f_buff);
								line_count++;
							}
							else{
								running_total += atof(f_buff);
								line_count++;
							}
						}
						printf("The total you have spent is: $%.2f\n", running_total);
						line_count = 1;
						running_total = 0.00;
						remove(file_to_watch);
						fclose(f_ptr);
					}
				}
				else if(event->mask & IN_MODIFY){
					//printf("File %s was modified\n", event->name);
				}
				else if(event->mask & IN_DELETE){
					//printf("File %s was deleted\n", event->name);
				}
			}
			i += INOT_EVENT_SIZE + event->len;
		}
	}
	return 0;
}
