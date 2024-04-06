#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
using namespace std;
class GroupModel
{
public:
    bool createGroup(Group&group);

    bool joinGroup(int account,int groupid,string role);

    bool leftGroup(int account,int groupid);

    vector<Group>queryUsergroup(int account);

    vector<int>queryGroupmember(int account,int groupid);
};
#endif