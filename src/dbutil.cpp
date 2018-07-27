#include <stdlib.h>
#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

class DBUtil{

  sql::Driver *driver;
  sql::Connection *con;
  sql::Statement *stmt;
  sql::ResultSet *res;
  public:
    DBUtil()
    {
        cout<<"Begin constructor";
try {
  

  /* Create a connection */
  driver = get_driver_instance();
  con = driver->connect("tcp://127.0.0.1:3306", "root", "123");
  /* Connect to the MySQL test database */
  con->setSchema("test");
  cout<<"Connection established sucessfully!";
} catch (sql::SQLException &e) {
  cout << "# ERR: SQLException in " << __FILE__;
  cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
  cout << "# ERR: " << e.what();
  cout << " (MySQL error code: " << e.getErrorCode();
  cout << ", SQLState: " << e.getSQLState() << " )" << endl;
}    }
string fetchQueryC(string table,string column,string condition)
{
    stmt = this->con->createStatement();
    string query="select "+column+" from "+table+" where "+condition;
    string result=this->executeQuery(query,column);
    return result;
}
string fetchQuery(string table,string column)
{
    
    string query="select "+column+" from "+table;
    string result=this->executeQuery(query,column);
    return result;
}
string executeQuery(string query,string column)
{
    stmt = this->con->createStatement();
    cout<<"Run Query: "<<query<<endl;
    res = stmt->executeQuery(query);
    string result=" ";
    while(res->next())
    {
        result=result+res->getString(column)+",\n";
    }
    result.pop_back();
    return result;

}
bool runDML(string query)
{
    stmt = this->con->createStatement();
    cout<<"Run DML Query: "<<query<<endl;
    bool result = stmt->execute(query);
    return result;
}

};

class DBActions{
    DBUtil instance;
    string emailid;
    public:
    DBActions()
    {
        //instance=new DBUtil();
        //cout<<"Constructor initialized";
    }
    bool createUser(string name,string password,string emailid)
    {
        string firstname="",lastname="";
        if (name.find(" ") != std::string::npos) {
               int point=name.find(" ");
               firstname=name.substr(0,point);
               lastname=name.substr(point+1);
               }
            else
            {
                firstname=name;
            }
        string query="insert into users (firstname,lastname,emailid,password) values ('";
        query=query+firstname+"','"+lastname+"','"+emailid+"','"+password+"')";
        bool result=this->instance.runDML(query);
        return result;

    }
    bool checkExists(string condition)
    {
        string fromDB=this->instance.fetchQueryC("users","count(*)",condition);
        if (fromDB.find("0") != std::string::npos){
            return false;
        }
        return true;
    }
    bool checkEmailid(string emailid)
    {
        string condition="emailid='"+emailid+"'";
        bool result=this->checkExists(condition);
        return result;
    }
    bool checkUser(string firstname)
    {
        string condition="firstname='"+firstname+"'";
        bool result=this->checkExists(condition);
        return result;
    }
    bool getLockedStatus(string emailid)
    {
        string condition="emailid='"+emailid+"'";
        string fromDB=this->instance.fetchQueryC("users","locked",condition);\
        cout<<"Lock Status: "<<fromDB;
        if (fromDB.find("1") != std::string::npos){
            return true;
        }
        return false;
    }
    bool getActiveStatus(string emailid)
    {
        string condition="emailid='"+emailid+"'";
        string fromDB=this->instance.fetchQueryC("users","active",condition);
        cout<<"Active Status: "<<fromDB;
        if (fromDB.find("1") != std::string::npos){
            return true;
        }
        return false;
    }

    string findAllNames()
    {
        string result=this->instance.fetchQuery("users","firstname");
        return result;
    }
    string findAllEmails()
    {
        string result=this->instance.fetchQuery("users","emailid");
        return result;
    }
    
    string findAccessLogs(string emailid)
    {
        string userid=this->instance.fetchQueryC("users","id","emailid='"+emailid+"'");
        std::string::size_type sz;   // alias of size_t
        int i_dec = std::stoi (userid,&sz);
        string i_string=std::to_string(i_dec);
        string result=this->instance.fetchQueryC("auditlog","actionstring","userid="+i_string);
        return result;
    }
    bool updateField(string column,string value, string emailid)
    {
        string query="update users set "+column+"='"+value+"' where emailid='"+emailid+"'";
        bool result=this->instance.runDML(query);
        return !result;
    }
    bool isAdminAccess(string emailid)
    {
        string condition="userid=(select id from users where emailid='"+emailid+"')";
        string fromDB=this->instance.fetchQueryC("access","count(*)",condition);
        if (fromDB.find("1") != std::string::npos){
            return true;
        }
        return false;
    }
    bool updatePassword(string emailid,string newPass)
    {
        string retreivedOldPass=this->instance.fetchQueryC("users","password","emailid='"+emailid+"'");
        if(retreivedOldPass.compare(newPass)==0)
        {
            return false;

        }
        bool status=this->updateField("password",newPass,emailid);
        return status;


    }
    bool updateFirstname(string emailid,string newName)
    {
        string oldName=this->instance.fetchQueryC("users","firstname","emailid='"+emailid+"'");
        if(oldName.compare(newName)==0)
        {
            return false;

        }
        bool status=this->updateField("firstname",newName,emailid);
        return status;
    }

    bool updateLastname(string emailid,string newName)
    {
        string oldName=this->instance.fetchQueryC("users","lastname","emailid='"+emailid+"'");
        if(oldName.compare(newName)==0)
        {
            return false;

        }
        bool status=this->updateField("lastname",newName,emailid);
        return status;
    }
    bool activateUser(string emailid)
    {
        if(!this->getActiveStatus(emailid))
        {
            bool status=this->updateField("active","1",emailid);
            return status;
        }
        return false;
    }
    bool unlockUser(string emailid)
    {
        if(!this->getLockedStatus(emailid))
        {
            bool status=this->updateField("locked","0",emailid);
            return status;
        }
        return false;
    }
    bool lockUser(string emailid)
    {
        if(this->getLockedStatus(emailid))
        {
            bool status=this->updateField("locked","1",emailid);
            return status;
        }
        return false;
    }
    bool deleteUser(string emailid)
    {
        if(!checkEmailid(emailid))
        return false;
        string query="delete from users where emailid='"+emailid+"'";
        bool status=this->instance.runDML(query);
        return !status;
    }
    bool resetPassword(string emailid)
    {
        return this->updatePassword(emailid,"");
    }

};

int main()
{
    /*DBUtil instance;
    
    cout<<instance.fetchQuery("users","emailid");
    cout<<"After instance gen.";*/
    DBActions object;
    //cout<<object.checkExists("admin@maxca.com");
    cout<<object.createUser("Navroz Charania","Password","fuelscud99@gmail.com");
    cout<<object.updateFirstname("fuelscud99@gmail.com","Max")<<endl;
    cout<<object.updateLastname("fuelscud99@gmail.com","Bang")<<endl;
    cout<<object.updatePassword("fuelscud99@gmail.com","churan")<<endl;
    cout<<object.checkUser("Max")<<endl;
    cout<<object.checkUser("Navroz")<<endl;
    cout<<object.checkEmailid("fuelscud99@gmail.com")<<endl;
    cout<<object.findAllEmails()<<endl;
    cout<<object.findAllNames()<<endl;
    cout<<object.activateUser("fuelscud99@gmail.com")<<endl;
    cout<<object.activateUser("xyz.com")<<endl;
    cout<<object.lockUser("fuelscud99@gmail.com")<<endl;
    cout<<object.unlockUser("fuelscud99@gmail.com")<<endl;
    cout<<object.resetPassword("fuelscud99@gmail.com")<<endl;
    cout<<object.deleteUser("fuelscud99@gmail.com");


    

}