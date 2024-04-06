#ifndef GROUP_H
#define GROUP_H

#include"groupuser.hpp"
#include<string>
#include<vector>
using namespace std;

class Group
{
public:
    Group(string name="",string desc="")
    {
        this->groupname=name;
        this->groupdesc=desc;
    }

    void setId(int id){this->groupid=id;}
    
    void setName(string name){this->groupname=name;}
    
    void setDesc(string desc){this->groupdesc=desc;}

    int getId(){return this->groupid;}

    string getName(){return this->groupname;}

    string getDesc(){return this->groupdesc;}

    vector<GroupUser> getMember(){return this->groupmember;}
private:
    int groupid;

    string groupname;
    
    string groupdesc;
    
    vector<GroupUser> groupmember;
};
#endif