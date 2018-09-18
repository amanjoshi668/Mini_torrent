#include <respond_to_request_client.h>

string send_details(map<string, torrent_for_map > &details_of_file, string &file_name){
    vector<string> result;
    Message message;
    if(!present(details_of_file, file_name)){
        cerr<<"The requested file is not present"<<endl;
        message.reload({"ERROR","I don't have this file"});
    }
    else{
        result.push_back("SUCCESS");
        REP(0,details_of_file[file_name].part_of_file.size()){
            if(details_of_file[file_name].part_of_file[i] == 1){
                string temp = to_string(i);
                temp = string(sizeof(lo)-temp.length(),'0').append(temp);
                result.push_back(temp);
            }
        }
        message.reload(result);
        cerr<<"Successfully given details of file"<<endl;
    }
    return message.encode_message();
}

string send_file(map<string, torrent_for_map > &details_of_file, vector<string> &request){
    vector<string> result;
    string file_name = request[1];
    lo part_number = stoll(request[2]);
    Message message;
    if(!present(details_of_file, file_name)){
        cerr<<"The requested file is not present"<<endl;
        message.reload({"ERROR","I don't have this file"});
        return message.encode_message();
    }
    string location = details_of_file[file_name].location;
    FILE *fp = fopen(location.c_str(), "rb");
    fseek(fp, part_number*BUFFER_SIZE, SEEK_SET);
    char buffer[BUFFER_SIZE];
    lo bytes_read = fread(buffer,1,BUFFER_SIZE, fp);
    if(bytes_read<0){
        cerr<<"The requested file is not present"<<endl;
        message.reload({"ERROR","I don't have this file"});
        return message.encode_message();
    }
    message.reload({"SUCCESS",string(buffer)});
    return message.encode_message();
}
//.././///fdsfjadsifadshfila