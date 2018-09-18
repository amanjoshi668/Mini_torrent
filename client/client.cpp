#include <message.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <mutex>
#include <netdb.h>
#include <create_torrent.h>

vector<string> split_string(string str){
    string temp;
    stringstream iss(str);
    vector<string> result;
    while(iss>>temp){
        result.push_back(temp);
    }
    return result;
}

int client_socket_fd;
map<string, torrent_for_map> details_of_file;
mutex details_of_file_mutex;
mutex log_file_descriptor;

set<string> downloaded_files;
set<string> currently_downloading_files;

mutex downloaded_files_mutex;
mutex currently_downloading_files_mutex;

void manage_download_file(vector<string> list_of_clients){
    while(1);
}

void share(string client_ip, string Tracker_1_url, string Tracker_2_url, string filename, string mtorrent_name){
    auto generated_torrent = generate_torrent(Tracker_1_url, Tracker_2_url, filename, mtorrent_name);
    string SHA_hash = generated_torrent.location;
    generated_torrent.location = filename;
    details_of_file_mutex.lock();
    details_of_file[SHA_hash] = generated_torrent;
    details_of_file_mutex.unlock();
    Message message({"SHARE",filename, SHA_hash, client_ip});
    string encoded_message = message.encode_message();
    //debug(encoded_message);
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if(response.empty()){
        log_file_descriptor.lock();
        cerr<<"Server is not responding"<<endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr<<response;
    log_file_descriptor.unlock();
    return;
}

void remove(string client_ip, string mtorrent_name){
    mtorrent entry_to_delete;
    ifstream fin;
    fin.open(mtorrent_name,ios::in);
    fin>>entry_to_delete;
    unlink(mtorrent_name.c_str());
    details_of_file_mutex.lock();
    details_of_file.erase(entry_to_delete.SHA_hash);
    details_of_file_mutex.unlock();
    Message message({"REMOVE", entry_to_delete.SHA_hash, client_ip});
    string encoded_message = message.encode_message();
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if(response.empty()){
        log_file_descriptor.lock();
        cerr<<"Server is not responding"<<endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr<<response;
    log_file_descriptor.unlock();
    return;
}

void get_file(string path_to_mtorrent_file, string destination_path){
    mtorrent entry_to_download;
    ifstream fin;
    fin.open(path_to_mtorrent_file, ios::in);
    fin>>entry_to_download;
    Message message({"SEEDERLIST", entry_to_download.SHA_hash });
    string encoded_message = message.encode_message();
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if(response.empty()){
        log_file_descriptor.lock();
        cerr<<"Server is not responding"<<endl;
        log_file_descriptor.unlock();
        return;
    }
    else if(response[0] == "ERROR"){
        log_file_descriptor.lock();
        cerr<<response<<endl;
        log_file_descriptor.unlock();
        return;
    }
    else if(response[0] == "CLOSE"){
        log_file_descriptor.lock();
        cerr<<"Connection Successfully terminated"<<endl;
        log_file_descriptor.unlock();
        return;
    }
    response[0] = entry_to_download.SHA_hash;
    response.push_back(destination_path);

    currently_downloading_files_mutex.lock();
    currently_downloading_files.insert(destination_path);
    currently_downloading_files_mutex.unlock();
    cout<<response;
    std :: thread T(manage_download_file, destination_path);
    T.detach();
    return;
}

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
        send(new_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    }
}

void create_server(string server_ip){
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
    port_no = stoi(server_ip.substr(server_ip.find_last_of(':')+1));//set port no.

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
        
        std :: thread T(handle_incoming_request, new_socket_fd, &client_address);
        T.detach();
    }

    close(socket_fd);    
}

int main(int argc, char*argv[]){

    //take the command line arguments
    string client_ip = string(argv[1]);
    string tracker_1_url = string(argv[2]);
    string tracker_2_url = string(argv[3]);
    string log_file = string(log_file);
    string tracker_url=tracker_1_url;

    ///////Create a Server
    std :: thread T(create_server, client_ip);
    T.detach();


    ////////

    int port_no;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket_fd < 0){
        log_file_descriptor.lock();
        cerr<<"ERROR in creating socket"<<endl;
        log_file_descriptor.unlock();
    }

    port_no = stoi(tracker_url.substr(tracker_url.find_last_of(':')+1));
    string tracker_name = tracker_url.substr(0,tracker_url.find_last_of(':'));

    sockaddr_in server_address;
    struct hostent *server = gethostbyname(tracker_name.c_str());
	server_address.sin_family = AF_INET; // host byte order
	server_address.sin_port = htons(port_no); // short, network byte order
	server_address.sin_addr = *((struct in_addr *)server->h_addr);
	memset(&(server_address.sin_zero), 0, 8); // zero the rest of the 
    
    if(connect(client_socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0 ){
        log_file_descriptor.lock();
        cerr<<"ERROR in connecting to server"<<endl;
        log_file_descriptor.unlock();
    }

    cout<<"Welcome"<<endl;
    cout<<"Enter share , download, ";
    string input;
    while(true){
        getline(cin,input);
        vector<string> input_commands = split_string(input);
        if(input_commands[0] == "share"){
            share(client_ip, tracker_1_url, tracker_2_url, input_commands[1], input_commands[2]);
        }
        else if(input_commands[0] == "remove"){
            remove(client_ip, input_commands[1]);
        }
        else if(input_commands[0] == "get"){
            get_file(input_commands[1], input_commands[2]);
        }
    }
    close(client_socket_fd);
    return 0;
}