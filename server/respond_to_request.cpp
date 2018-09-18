#include <respond_to_request.h>

string add_seeder(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request){
    string file_name = request[0];
    string SHA_hash = request[1];
    string client_ip_and_port = request[2];
    if(present(hash_vs_seeder_ip_port,SHA_hash)){
        hash_vs_seeder_ip_port[SHA_hash].insert(client_ip_and_port);
    }
    else{
        set<string> for_insertion;
        for_insertion.insert(client_ip_and_port);
        hash_vs_seeder_ip_port[SHA_hash] = for_insertion;
    }
    //update seeder_list_file
    //write to log
    cerr<<"Successfully added to tracker"<<endl;
    Message message({"SUCCESS","Your Entry has been recorded"});
    return message.encode_message();
}

string remove_seeder(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request){
    string SHA_hash = request[0];
    string client_ip_and_port = request[1];
    Message message;
    debug(client_ip_and_port);
    if(present( hash_vs_seeder_ip_port,SHA_hash)){
        hash_vs_seeder_ip_port[SHA_hash].erase(client_ip_and_port);
        if( hash_vs_seeder_ip_port[SHA_hash].empty()){
            hash_vs_seeder_ip_port.erase(SHA_hash);
        }
        cerr<<"Successfully removed from tracker"<<endl;
        message.reload({"SUCCESS","Your have been removed from the Seeder List"});
    }
    else {
        cerr<<"Entry was not on tracker"<<endl;
        message.reload({"ERROR","You were not a seeder"});
    }
    //Update seeder_list_file
    return message.encode_message();
}

string provide_seeder_list(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request){
    string SHA_hash = request[0];
    Message message;
    vector<string> result;
    if(!present( hash_vs_seeder_ip_port,SHA_hash)){
        message.reload({"ERROR","Requested file not found"});
        return message.encode_message();
    }
    result.push_back("SUCCESS");
    TRV( hash_vs_seeder_ip_port[SHA_hash]){
        result.push_back(it);
    }
    cerr<<"Successfully given list"<<endl;
    message.reload(result);
    return message.encode_message();
}
