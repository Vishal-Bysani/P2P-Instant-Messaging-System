#include "config.hpp"

Tracker tracker;
pthread_mutex_t lock;

void* start_tracker(void* port){
    pthread_mutex_lock(&lock);
    std::cout << "Tracker initiated with address : " << TRACKER_IP << ", port : " << *(int*) port << std::endl;
    pthread_mutex_unlock(&lock);
    char buffer[MAX_MSG_LEN], client_ipaddr[MAX_IP_LEN], client_name[MAX_NAME_LEN], port_string[MAX_PORT_LENGTH];
    bzero(buffer, sizeof(buffer));
    bzero(client_ipaddr, sizeof(client_ipaddr));
    bzero(client_name, sizeof(client_name));
    bzero(port_string, sizeof(port_string));
    int tracker_fd, new_socket, option = 1, addrlen;

    // Initializing tracker thread
    struct sockaddr_in tracker_addr;
    tracker_addr.sin_family = AF_INET;
    tracker_addr.sin_port = htons(*(int*) port);
    tracker_addr.sin_addr.s_addr = INADDR_ANY;

    for(;;){
        if ((tracker_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(tracker_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        if (bind(tracker_fd, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if(listen(tracker_fd, 10) < 0){
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(tracker_fd, NULL, NULL)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        read(new_socket, buffer, sizeof(buffer));
        pthread_mutex_lock(&lock);
        // MESSAGE SHOULD START WITH MESSAGE IDENTIFIER:
        // 'R' FOR NEW USER REGISTRATION
        // 'G' FOR OBTAINING A PEER'S IP ADDRESS
        // 'D' FOR DELETING CURRENT USER
        // 'L' FOR LOGGING OFF
        if (buffer[0] == 'R')
        {
            int free_directory_entry;
            for(free_directory_entry = 0; free_directory_entry < MAX_USERS; free_directory_entry++){
                if(!tracker.names[free_directory_entry]) break;
            }
            if(free_directory_entry >= MAX_USERS){
                perror("No free entries left in the directory");
                exit(EXIT_FAILURE);
            }
            tracker.names[free_directory_entry] = new char[MAX_NAME_LEN];
            tracker.ips[free_directory_entry] = new char[MAX_IP_LEN];
            strncpy(tracker.names[free_directory_entry], buffer + 1, MAX_NAME_LEN);
            strncpy(tracker.ips[free_directory_entry], buffer + 1 + MAX_NAME_LEN, MAX_IP_LEN);
            tracker.ports[free_directory_entry] = client_available_ports.back();
            client_available_ports.pop_back();
            snprintf(port_string, sizeof(port_string), "%d", tracker.ports[free_directory_entry]);
            send(new_socket, port_string, sizeof(port_string), MSG_CONFIRM);
            std::cout << "User " << tracker.names[free_directory_entry] << " succesfully registered in P2PText" << std::endl;
        }
        else if(buffer[0] == 'G'){
            strncpy(client_name, buffer+1, MAX_NAME_LEN);
            int query_peer;
            for(query_peer=0; query_peer<MAX_USERS; query_peer++){
                if(!strcmp(tracker.names[query_peer], client_name)) break;
            }
            if(query_peer >= MAX_USERS){
                perror("Demanded user not found");
                exit(EXIT_FAILURE);
            }
            char response_buffer[MAX_MSG_LEN];
            strncpy(response_buffer, tracker.ips[query_peer], MAX_IP_LEN);
            snprintf(response_buffer + MAX_IP_LEN, MAX_PORT_LENGTH, "%d", tracker.ports[query_peer]);
            if(send(new_socket, response_buffer, sizeof(response_buffer), 0) < 0){
                perror("Error in sending ip address");
                exit(EXIT_FAILURE);
            }
        }
        pthread_mutex_unlock(&lock);
        bzero(buffer, sizeof(buffer));
        bzero(client_ipaddr, sizeof(client_ipaddr));
        bzero(client_name, sizeof(client_name));
        close(tracker_fd);
        close(new_socket);
    }
}


int main() {
    
    pthread_t tracker_set[available_ports.size()];

    for(int i=0; i<available_ports.size(); i++){
        pthread_create(&tracker_set[i], NULL, start_tracker, (void*)&available_ports[i]);
    }

    for(int i=0; i<available_ports.size(); i++){
        pthread_join(tracker_set[i], NULL);
    }

}