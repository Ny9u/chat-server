#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User{
public:
    User(int _account=-1,string _name="",string _password="",string _state="offline")
    {
        this->account=_account;
        this->name=_name;
        this->password=_password;
        this->state=_state;
    }
    
    int getId(){return this->id;}

    string getName(){return this->name;}

    int getAccount(){return this->account;}
    
    string getPassword(){return this->password;}
    
    string getState(){return this->state;}

    void setId(int _id){this->id=_id;}
    
    void setName(string _name){this->name=_name;};

    void setAccount(int _account){this->account=_account;}

    void setPassword(string _password){this->password=_password;}

    void setState(string _state){this->state=_state;}

private:
    int id;
    string name;
    int account;
    string password;
    string state;
};

#endif