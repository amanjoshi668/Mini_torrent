#include <message_server.h>
#include <respond_to_request_server.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <mutex>

map<string, set<string> > hash_vs_seeder_ip_port;
mutex hash_vs_seeder_ip_port_mutex;
mutex log_file_descriptor;
int other_tracker_descriptor;

void handle_connection(int new_socket_fd,sockaddr_in *client_addrress){
    Message message;
    string encoded_message;
    while(true){
        auto request = message.decode_message(new_socket_fd);
        cout<<"request decoded"<<endl;
        if(request.empty()){
            log_file_descriptor.lock();
            cerr<<"Didnot recieved the message properly from client : "<<inet_ntoa(client_addrress->sin_addr)<<":"<<ntohs(client_addrress->sin_port)<<endl;
            log_file_descriptor.unlock();
            message.reload({"ERROR","didn't recieve the message properly"});
            encoded_message = message.encode_message();
        }
        if(request[0] == "CLOSE"){
            log_file_descriptor.lock();
            cerr<<"Connection gracefully closed by client : "<<inet_ntoa(client_addrress->sin_addr)<<":"<<ntohs(client_addrress->sin_port)<<endl;
            log_file_descriptor.unlock();
            close(new_socket_fd);
            //message.reload({"ERROR","didn't recieve the message properly"});
            return;
        }
        if(request[0] == "SHARE"){
            log_file_descriptor.lock();
            cerr<<"SHAREING on request of "<<inet_ntoa(client_addrress->sin_addr)<<":"<<ntohs(client_addrress->sin_port)<<endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();
            
            encoded_message = add_seeder(hash_vs_seeder_ip_port,request);
        
            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        else if(request[0] == "REMOVE"){
            log_file_descriptor.lock();
            cerr<<"REMOVING on request of "<<inet_ntoa(client_addrress->sin_addr)<<":"<<ntohs(client_addrress->sin_port)<<endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();
            
            encoded_message = remove_seeder(hash_vs_seeder_ip_port,request);
            
            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        else if(request[0] == "SEEDERLIST"){
            log_file_descriptor.lock();
            cerr<<"PROVIDING SEEDERLIST on request of "<<inet_ntoa(client_addrress->sin_addr)<<":"<<ntohs(client_addrress->sin_port)<<endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();
            
            encoded_message = provide_seeder_list(hash_vs_seeder_ip_port,request);
            
            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        //debug(encoded_message);

        hash_vs_seeder_ip_port_mutex.lock();
        TRV(hash_vs_seeder_ip_port){
            cerr<<it.X<<":::::";
            for(auto &it2: it.Y){
                cerr<<it2<<"    &    ";
            }
            cerr<<endl;
        }
        hash_vs_seeder_ip_port_mutex.unlock();
        cerr<<encoded_message<<endl;
        send(new_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    }
}

int main(int argc, char* argv[]){
    
    //take the arguments
    string my_tracker_url = string(argv[1]);
    string other_tracker_url = string(argv[2]);
    string seeder_list_file = string(argv[3]);
    string log_file = string(argv[4]);
    //////

    int socket_fd;
    int port_no;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd<0){
        log_file_descriptor.lock();
        cerr<<"ERROR in creating socket"<<endl;
        log_file_descriptor.unlock();
    }

    int reuse_port = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));

    sockaddr_in server_address;//Server Address
    bzero((char *) &server_address, sizeof(server_address));
    port_no = stoi(my_tracker_url.substr(my_tracker_url.find_last_of(':')+1));//set port no.

    /////Set the server details

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_no);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (sockaddr *) &server_address, sizeof(server_address)) <0){
        log_file_descriptor.lock();
        cerr<<"Could not bind the server to the given port"<<endl;
        log_file_descriptor.unlock();
    }

    unsigned int back_log_size = 5;
    listen(socket_fd, back_log_size);//Server Listening
    log_file_descriptor.lock();    
    cerr<<"Server opened on port: "<<port_no<<endl;
    log_file_descriptor.unlock();

    while(true){
        int new_socket_fd;
        unsigned int client_length;
        sockaddr_in client_address;
        client_length = sizeof(sockaddr_in);

        //Blocak until a client connects
        new_socket_fd = accept(socket_fd, (sockaddr *) &client_address, &client_length);

        if(new_socket_fd < 0){
            log_file_descriptor.lock();
            cerr<<"ERROR on accept: "<<endl;
            log_file_descriptor.unlock();
        }
        else{
            log_file_descriptor.lock();
            cerr<<"Connected to "<<inet_ntoa(client_address.sin_addr)<<":"<<ntohs(client_address.sin_port)<<endl;
            log_file_descriptor.unlock();
        }
        
        std :: thread T(handle_connection, new_socket_fd, &client_address);
        T.detach();
    }

    close(socket_fd);    

    return 0;
}