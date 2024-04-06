#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include"user.hpp"
#include<vector>
using namespace std;
class FriendModel
{
public:
    //添加好友
    bool insert(int usercnt,int friendcnt);
    //查询好友
    vector<User> queryFriendlist(int account);
    //删除好友
    bool remove(int usercnt,int friendcnt);
};
#endif
