#include <message_server.h>
#ifndef RESPOND_TO_REQUEST_SERVER_H
#define RESPOND_TO_REQUEST_SERVER_H

string add_seeder(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request);
string remove_seeder(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request);
string provide_seeder_list(map<string, set<string> > &hash_vs_seeder_ip_port, vector<string> &request);

#endif