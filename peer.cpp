// peer.cpp
#include "config.hpp"

Peer peer;
pthread_mutex_t plock = PTHREAD_MUTEX_INITIALIZER;

void* receiving_thread(void* port) {
    pthread_mutex_lock(&plock);
    std::cout << "Peer receiving thread initiated!" << std::endl;
    pthread_mutex_unlock(&plock);
    char buffer[MAX_MSG_LEN];
    bzero(buffer, sizeof(buffer));
    char sender_name[MAX_NAME_LEN], message[MAX_MSG_LEN];
    bzero(sender_name, sizeof(sender_name));
    bzero(message, sizeof(message));

    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(*(int*)port);
    peer_addr.sin_addr.s_addr = INADDR_ANY;

    int peer_fd, new_socket, option = 1;
    if((peer_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(peer_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if(bind(peer_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    while(true){
        if(listen(peer_fd, MAX_QUEUED_REQUESTS) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
        if((new_socket = accept(peer_fd, nullptr, nullptr)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        read(new_socket, buffer, sizeof(buffer));
        strncpy(sender_name, buffer, sizeof(sender_name));
        strncpy(message, buffer + sizeof(sender_name), sizeof(message)-sizeof(sender_name));

        pthread_mutex_lock(&plock);
        std::cout << "[ " << sender_name << " ]: " << message << std::endl;
        pthread_mutex_unlock(&plock);
        close(new_socket);
        bzero(buffer, sizeof(buffer));
        bzero(sender_name, sizeof(sender_name));
        bzero(message, sizeof(message));
    }
}
void* sending_thread(void* port){
    std::cout << "Peer sending thread initiated!" << std::endl;
    char buffer[MAX_MSG_LEN], input_buffer[MAX_MSG_LEN], peer_name[MAX_NAME_LEN], receiver_name[MAX_NAME_LEN], ip_addr[MAX_IP_LEN], port_str[MAX_PORT_LENGTH];
    bzero(buffer, sizeof(buffer));
    bzero(input_buffer, sizeof(input_buffer));
    bzero(peer_name, sizeof(peer_name));
    bzero(receiver_name, sizeof(receiver_name));
    bzero(ip_addr, sizeof(ip_addr));
    bzero(port_str, sizeof(port_str));

    std::cout << "Enter your name: ";
    std::cout.flush();
    if(std::cin.peek() == '\n') {
        std::cin.ignore();
    }
    std::cin.getline(peer_name, sizeof(peer_name));

    std::string peer_file_name = "./peer_storage/peer_" + std::string(peer_name) + ".txt";

    //Set connection with tracker
    int tracker_fd, receiver_fd;
    struct sockaddr_in tracker_addr;
    tracker_addr.sin_family = AF_INET;
    tracker_addr.sin_port = htons(available_ports[MAX_CLIENT_PORTS%available_ports.size()]);

    if((tracker_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    if(inet_pton(AF_INET, TRACKER_IP, &tracker_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    if(connect(tracker_fd, (struct sockaddr*)&tracker_addr, sizeof(tracker_addr)) < 0) {
        perror("Connection to tracker failed");
        exit(EXIT_FAILURE);
    }

    std::ifstream peer_file(peer_file_name);
    if(peer_file.good()){
        //Retrieve old port
        peer_file.getline(port_str, sizeof(port_str));
        *(int*)port = atoi(port_str);
        std::cout << "Welcome back! Continuing on port: " << *(int*)port << std::endl;

        //Retrieve old contacts
        for(int i = 0; !peer_file.eof() && i < MAX_CONTACTS; ++i) {
            peer.contacts[i] = new char[MAX_NAME_LEN];
            peer.contacts_ip[i] = new char[MAX_IP_LEN];
            peer_file.getline(peer.contacts[i], MAX_NAME_LEN);
            peer_file.getline(peer.contacts_ip[i], MAX_IP_LEN);
            peer_file.getline(port_str, MAX_PORT_LENGTH);
            peer.contacts_port[i] = std::atoi(port_str);
        }
    }
    else{
        buffer[0] = 'R'; // Register
        strncpy(buffer + 1, peer_name, sizeof(peer_name));
        get_my_ip(ip_addr);
        strncpy(buffer + 1 + sizeof(peer_name), ip_addr, sizeof(ip_addr));
        send(tracker_fd, buffer, sizeof(buffer), 0);
        read(tracker_fd, buffer, sizeof(buffer));
        *(int*) port = std::atoi(buffer);
        std::cout << "Welcome! You are registered on port: " << *(int*)port << std::endl;
    }
    close(tracker_fd);
    pthread_mutex_unlock(&plock);

    //Sending sockaddr setup
    struct sockaddr_in receiver_addr;
    receiver_addr.sin_family = AF_INET;
    sleep(0); // Allow time for the receiving thread to set up
    while(true){
        std::cout << "> ";
        bzero(input_buffer, sizeof(input_buffer));
        bzero(buffer, sizeof(buffer));
        if(std::cin.peek() == '\n') {
            std::cin.ignore();
        }
        std::cin.getline(input_buffer, sizeof(input_buffer));

        // COMMANDS:
        // Add/add : Adds the given peer to contacts
        //          '> Add Vishal'
        // Send/send : Prompts user to enter a message which is then sent to provided user
        //          '> Send Vishal'
        //          '> Please enter your message: Hello!'
        // Exit/exit : Exits the program
        //          '> Exit'

        if(input_buffer[0] == 'E' || input_buffer[0] == 'e') {
            std::ofstream peer_storage_file;
            peer_storage_file.open(peer_file_name);
            peer_storage_file << *(int*) port;
            for(int i = 0; i < MAX_CONTACTS; ++i){
                if(peer.contacts[i]){
                    peer_storage_file << '\n' << peer.contacts[i] << '\n' << peer.contacts_ip[i] << '\n' << peer.contacts_port[i];
                    peer_storage_file.flush();
                    delete []peer.contacts[i];
                    delete []peer.contacts_ip[i];
                }
            }
            exit(EXIT_SUCCESS);
        }
        if((tracker_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Socket creation error: tracker_fd");
            exit(EXIT_FAILURE);
        }
        if((receiver_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Socket creation error: receiver_fd");
            exit(EXIT_FAILURE);
        }
        if(connect(tracker_fd, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr)) < 0){
            perror("Connection to tracker failed");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&plock);
        if(input_buffer[0] == 'A' || input_buffer[0] == 'a'){
            //Expected command : Add/add
            int free_index;
            for(free_index = 0; free_index < MAX_CONTACTS; ++free_index){
                if(!peer.contacts[free_index]) break;
            }
            if(free_index == MAX_CONTACTS){
                std::cerr << "Already established " << MAX_CONTACTS << "contacts." << std::endl;
                close(receiver_fd);
                close(tracker_fd);
                continue;
            }
            buffer[0] = 'G';
            peer.contacts[free_index] = new char[MAX_NAME_LEN];
            peer.contacts_ip[free_index] = new char[MAX_IP_LEN];
            strncpy(peer.contacts[free_index], input_buffer+4, MAX_NAME_LEN);
            strncpy(buffer+1, input_buffer+4, MAX_NAME_LEN);
            send(tracker_fd, buffer, sizeof(buffer), 0);
            bzero(buffer, sizeof(buffer));
            bzero(port_str, sizeof(port_str));
            if(read(tracker_fd, buffer, sizeof(buffer)) < 0){
                perror("Error in reading back from tracker");
                exit(EXIT_FAILURE);
            }
            strncpy(peer.contacts_ip[free_index], buffer, MAX_IP_LEN);
            strncpy(port_str, buffer+MAX_IP_LEN, MAX_PORT_LENGTH);
            peer.contacts_port[free_index] = std::atoi(port_str);
            std::cout << "Successfully added " << peer.contacts[free_index] << std::endl;
        }
        else if(input_buffer[0] == 'S' || input_buffer[0] == 's'){
            strncpy(receiver_name, input_buffer+5, MAX_NAME_LEN);
            int receiver_index;
            for(receiver_index = 0; receiver_index < MAX_CONTACTS; ++receiver_index){
                if(strlen(peer.contacts[receiver_index]) != 0 && strcmp(peer.contacts[receiver_index], receiver_name) == 0) break;
            }
            if(receiver_index == MAX_CONTACTS){
                std::cerr << "Given receiver doesn't exist in your contact list" << std::endl;
                close(tracker_fd);
                close(receiver_fd);
                continue;
            }
            receiver_addr.sin_port = htons(peer.contacts_port[receiver_index]);
            if(inet_pton(AF_INET, peer.contacts_ip[receiver_index], &receiver_addr.sin_addr) < 0){
                perror("Invalid IP address of receiver in send operation");
                exit(EXIT_FAILURE);
            }
            if(connect(receiver_fd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0){
                perror("Connection failure in send operation");
                exit(EXIT_FAILURE);
            }
            bzero(buffer, sizeof(buffer));
            strncpy(buffer, peer_name, sizeof(peer_name));
            std::cout << "Please enter your message: ";
            if(std::cin.peek() == '\n'){
                std::cin.ignore();
            }
            std::cin.getline(buffer+sizeof(peer_name), MAX_MSG_LEN - MAX_NAME_LEN);
            send(receiver_fd, buffer, sizeof(buffer), 0);
        }
        close(tracker_fd);
        close(receiver_fd);
        pthread_mutex_unlock(&plock);
    }
}
int main() {
    for(int i = 0; i < MAX_CONTACTS; ++i) {
        peer.contacts[i] = nullptr;
        peer.contacts_ip[i] = nullptr;
        peer.contacts_port[i] = 0;
    }

    int cur_port;
    pthread_t sender, receiver;
    pthread_mutex_lock(&plock);

    pthread_create(&sender, nullptr, sending_thread, (void*)&cur_port);
    pthread_create(&receiver, nullptr, receiving_thread, (void*)&cur_port);

    pthread_join(sender, nullptr);
    pthread_join(receiver, nullptr);
    return 0;
}