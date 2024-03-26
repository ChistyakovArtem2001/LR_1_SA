#include "user.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS users (id SERIAL,"
                        << "first_name VARCHAR(256) NOT NULL,"
                        << "last_name VARCHAR(256) NOT NULL,"
                        << "login VARCHAR(256) NOT NULL,"
                        << "password VARCHAR(256) NOT NULL,"
                        << "email VARCHAR(256) NULL,"
                        << "title VARCHAR(1024) NULL);",
                now;
        }

        catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::ConnectionFailedException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("title", _title);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.title() = object->getValue<std::string>("title");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    std::optional<long> User::auth(std::string &login, std::string &password)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            long id;
            select << "SELECT id FROM users where login=$1 and password=$2",
                into(id),
                use(login),
                use(password),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return id;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }
    std::optional<User> User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;
            select << "SELECT id, first_name, last_name, email, title,login,password FROM users where id=$1",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return a;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }



       std::optional<User> User::delete_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User removed_user;
            // Выборка данных пользователя перед удалением
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE id=$1",
            into(removed_user._id),
            into(removed_user._first_name),
            into(removed_user._last_name),
            into(removed_user._email),
            into(removed_user._title),
            into(removed_user._login),
            into(removed_user._password),
            use(id),
            range(0, 1);
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
            {
                Poco::Data::Statement remove(session);
                remove << "DELETE FROM users WHERE id = $1",
                use(id);
                remove.execute();
            }
            return removed_user;
        }
    catch (Poco::Data::PostgreSQL::ConnectionException &e)
    {
        std::cout << "connection:" << e.what() << std::endl;
        throw;
    }
    catch (Poco::Data::PostgreSQL::StatementException &e)
    {
        std::cout << "statement:" << e.what() << std::endl;
        throw;
    }
    }


    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            first_name += "%";
            last_name += "%";
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users where first_name LIKE $1 and last_name LIKE $2",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                use(first_name),
                use(last_name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);
// Тут хэшируем пароль
        unsigned char hash[SHA256_DIGEST_LENGTH];
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        const EVP_MD *md = EVP_sha256();

        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, _password.c_str(), _password.length());
        EVP_DigestFinal_ex(mdctx, hash, NULL);
        EVP_MD_CTX_free(mdctx);

        // и переводим в шестнадцатеричный формат
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        std::string hashed_password = ss.str();

            insert << "INSERT INTO users (first_name,last_name,email,title,login,password) VALUES($1, $2, $3, $4, $5, $6)",
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_title),
                use(_login),
                use(hashed_password);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LASTVAL()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }


    std::optional<User> User::update_to_mysql (
        long id, 
        std::string first_name, 
        std::string last_name,
        std::string login, 
        std::string password,
        std::string email,
        std::string title
    )
    {
        try
        {
             Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement select(session);
        User update_user;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        const EVP_MD *md = EVP_sha256();

        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, password.c_str(), password.length());
        EVP_DigestFinal_ex(mdctx, hash, NULL);
        EVP_MD_CTX_free(mdctx);
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        std::string hashed_password = ss.str();


        select << "SELECT id, first_name, last_name, email, title, login, password  FROM users WHERE id=$1",
            into(update_user._id),
            into(update_user._first_name),
            into(update_user._last_name),
            into(update_user._email),
            into(update_user._title),
            into(update_user._login),
            into(update_user._password),
            use(id),
            range(0, 1);
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
            {
                Poco::Data::Statement update(session);
                update << "UPDATE users SET first_name=$1, last_name=$2, login=$3, password=$4, email=$5, title=$6 WHERE id=$7",
                use(first_name),
                use(last_name),
                use(login),
                useRef(hashed_password),
                use(email),
                use(title),
                use(id);
                range(0, 1);
                update.execute();
                
                if (update.execute() > 0)
                {
                Poco::Data::Statement select(session);
                select << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE id=$1",
                into(update_user._id),
                into(update_user._first_name),
                into(update_user._last_name),
                into(update_user._email),
                into(update_user._title),
                into(update_user._login),
                into(update_user._password),
                use(id),
                range(0, 1);

                select.execute();
                if (rs.moveFirst()) return update_user;
            }
            
            }
            
            
        
            

            // Poco::Data::Session session = database::Database::get().create_session();
            // Poco::Data::Statement update(session);
            // User update_user;
            // // Выборка данных пользователя перед удалением
            // update << "UPDATE users SET first_name=$1, last_name=$2, login=$3, password=$4, email=$5, title=$6 WHERE id=$7",
            // use(first_name),
            // use(last_name),
            // use(login),
            // use(password),
            // use(email),
            // use(title),
            // use(id);
            // range(0, 1);
            // update.execute();
            
            // if (update.execute() > 0)
            // {
            //     Poco::Data::Statement select(session);
            // select << "SELECT id, first_name, last_name, email, title, login, password FROM users WHERE id=$1",
            // into(update_user._id),
            // into(update_user._first_name),
            // into(update_user._last_name),
            // into(update_user._email),
            // into(update_user._title),
            // into(update_user._login),
            // into(update_user._password),
            // use(id),
            // range(0, 1);
            // // Выполнение запроса SELECT
            // select.execute();
            // }
            // return update_user;
            
        }  



        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        return {};
    }

//          static std::optional<User> update_to_mysql(long id,std::string first_name,std::string last_name, 
//      std::string login,std::string password,std::string email,std::string title)
//     {

//         try
//         {
//         Poco::Data::Session session = database::Database::get().create_session();
//         Poco::Data::Statement update(session);
//         User update_user;
//  update << "UPDATE users SET first_name=$1, last_name=$2, title=$3, password=$4 WHERE id=$5",
//     use(first_name),
//     use(last_name),
//     use(login),
//     use(password),
//     use(email),
//     use(title),
   
//     use(id);
//     range(0, 1);
    

//     update.execute();
//     std::cout << " user with ID updated: " << id << std::endl;
//     return update_user;
//         }
//         catch (Poco::Data::PostgreSQL::ConnectionException &e)
//         {
//             std::cout << "connection:" << e.what() << std::endl;
//             throw;
//         }
//         catch (Poco::Data::PostgreSQL::StatementException &e)
//         {

//             std::cout << "statement:" << e.what() << std::endl;
//             throw;
//         }
//     }

     






    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_title() const
    {
        return _title;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::title()
    {
        return _title;
    }
}