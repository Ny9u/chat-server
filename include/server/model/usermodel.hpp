#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

class UserModel
{
public:
    //插入数据
    bool insert(User &user);
    //查询数据
    User query_id(int id);
    
    User query_account(int account);
    
    User query_name(string name);
    //更新用户数据
    bool update(User &user);
};

#endif