#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include<vector>
#include<string>
using namespace std;
class OfflinemessageModel
{
public:
    //插入离线信息
    bool insert(int account,string msg);
    //查询用户离线信息
    vector<string> query(int account);
    //删除离线信息
    bool remove(int account);
};

#endif