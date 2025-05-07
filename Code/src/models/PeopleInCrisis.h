#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include "../database/DatabaseManager.h"
#include <Poco/Data/TypeHandler.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

// Model class for people in crisis
class PeopleInCrisis {
private:
    int id;
    std::string name;
    int userID;
    std::string location;
    std::string phoneNo;
    std::string description;
    std::string status;
    bool hasActiveRequest;
    std::string username;
    std::string password;

public:
    // Constructor
    PeopleInCrisis() : id(0), userID(0), hasActiveRequest(false) {}
    
    PeopleInCrisis(const std::string& name, int userID, const std::string& location, 
                  const std::string& phoneNo, const std::string& description) 
        : id(0), name(name), userID(userID), location(location), 
          phoneNo(phoneNo), description(description), 
          status("Pending"), hasActiveRequest(true) {}

    // Getters
    int getId() const { return id; }
    std::string getName() const { return name; }
    int getUserID() const { return userID; }
    std::string getLocation() const { return location; }
    std::string getPhoneNo() const { return phoneNo; }
    std::string getDescription() const { return description; }
    std::string getStatus() const { return status; }
    bool getHasActiveRequest() const { return hasActiveRequest; }
    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }

    // Setters
    void setId(int id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setUserID(int userID) { this->userID = userID; }
    void setLocation(const std::string& location) { this->location = location; }
    void setPhoneNo(const std::string& phoneNo) { this->phoneNo = phoneNo; }
    void setDescription(const std::string& description) { this->description = description; }
    void setStatus(const std::string& status) { this->status = status; }
    void setHasActiveRequest(bool hasActiveRequest) { this->hasActiveRequest = hasActiveRequest; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPassword(const std::string& password) { this->password = password; }

    // Method to update status as mentioned in the class diagram
    void updateStatus(const std::string& newStatus) {
        status = newStatus;
        if (status == "Resolved" || status == "Cancelled") {
            hasActiveRequest = false;
        }
    
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            int activeFlag = hasActiveRequest ? 1 : 0;
    
            session << "UPDATE people_in_crisis SET status = ?, has_active_request = ? WHERE id = ?",
                use(status), use(activeFlag), use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error updating status: " << e.what() << std::endl;
        }
    }
    
    
    // Verify password
    bool verifyPassword(const std::string& inputPassword) {
        return password == inputPassword;
    }

    // Convert to JSON for API responses
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("name", name);
        json->set("userID", userID);
        json->set("location", location);
        json->set("phoneNo", phoneNo);
        json->set("description", description);
        json->set("status", status);
        json->set("hasActiveRequest", hasActiveRequest);
        json->set("username", username);
        return json;
    }

    // Create from JSON for API requests
    static PeopleInCrisis fromJSON(const Poco::JSON::Object::Ptr& json) {
        PeopleInCrisis person;
        
        if (json->has("id")) {
            person.setId(json->getValue<int>("id"));
        }
        
        person.setName(json->getValue<std::string>("name"));
        
        if (json->has("userID")) {
            person.setUserID(json->getValue<int>("userID"));
        }
        
        if (json->has("location")) {
            person.setLocation(json->getValue<std::string>("location"));
        }
        
        if (json->has("phoneNo")) {
            person.setPhoneNo(json->getValue<std::string>("phoneNo"));
        }
        
        if (json->has("description")) {
            person.setDescription(json->getValue<std::string>("description"));
        }
        
        if (json->has("status")) {
            person.setStatus(json->getValue<std::string>("status"));
        } else {
            person.setStatus("Pending");
        }
        
        if (json->has("hasActiveRequest")) {
            person.setHasActiveRequest(json->getValue<bool>("hasActiveRequest"));
        } else {
            person.setHasActiveRequest(true);
        }
        
        if (json->has("username")) {
            person.setUsername(json->getValue<std::string>("username"));
        }
        
        if (json->has("password")) {
            person.setPassword(json->getValue<std::string>("password"));
        }
        
        return person;
    }
    
    // Database operations
    bool save() {
        try {
            auto session = DatabaseManager::getInstance()->getSession();
            
            if (id == 0) {
                // Insert new record
                bool verified = false;  // Initialize verified status
                Poco::Data::Statement insert(session);
                insert << "INSERT INTO people_in_crisis (name, user_id, location, phone_no, description, status, has_active_request, username, password, verified) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    use(name), use(userID), use(location), use(phoneNo), use(description), use(status), use(hasActiveRequest), use(username), use(password), use(verified), now;
                
                // Get the last inserted ID
                Poco::Data::Statement select(session);
                select << "SELECT last_insert_rowid()", into(id), now;
                
                // Create verification record
                std::string userType = "people_in_crisis";
                int userIdCopy = id;
                Poco::Data::Statement verifyInsert(session);
                verifyInsert << "INSERT INTO account_verifications (user_type, user_id, status, created_at) VALUES (?, ?, 'pending', datetime('now'))",
                    use(userType), use(userIdCopy), now;
            } else {
                // Update existing record
                Poco::Data::Statement update(session);
                update << "UPDATE people_in_crisis SET name = ?, user_id = ?, location = ?, phone_no = ?, description = ?, status = ?, has_active_request = ?, username = ?, password = ? WHERE id = ?",
                    use(name), use(userID), use(location), use(phoneNo), use(description), use(status), use(hasActiveRequest), use(username), use(password), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving person in crisis: " << e.what() << std::endl;
            return false;
        }
    }
    
    static PeopleInCrisis findById(int id) {
        PeopleInCrisis person;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Poco::Int64 dbId = 0;
            std::string dbName;
            Poco::Int64 dbUserId = 0;
            std::string dbLocation;
            std::string dbPhoneNo;
            std::string dbDescription;
            std::string dbStatus;
            Poco::Int64 dbHasActiveRequest = 0;
            std::string dbUsername;
            std::string dbPassword;
            
            session << "SELECT id, name, user_id, location, phone_no, description, status, has_active_request, username, password "
                   << "FROM people_in_crisis WHERE id = ?",
                into(dbId), into(dbName), into(dbUserId), into(dbLocation), into(dbPhoneNo),
                into(dbDescription), into(dbStatus), into(dbHasActiveRequest), into(dbUsername), into(dbPassword),
                use(id), now;
            
            person.setId(static_cast<int>(dbId));
            person.setName(dbName);
            person.setUserID(static_cast<int>(dbUserId));
            person.setLocation(dbLocation);
            person.setPhoneNo(dbPhoneNo);
            person.setDescription(dbDescription);
            person.setStatus(dbStatus);
            person.setHasActiveRequest(dbHasActiveRequest != 0);
            person.setUsername(dbUsername);
            person.setPassword(dbPassword);
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding PeopleInCrisis by ID: " << e.what() << std::endl;
        }
        
        return person;
    }
    
    static PeopleInCrisis findByUsername(const std::string& username) {
        PeopleInCrisis person;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Poco::Int64 dbId = 0;
            std::string dbName;
            Poco::Int64 dbUserId = 0;
            std::string dbLocation;
            std::string dbPhoneNo;
            std::string dbDescription;
            std::string dbStatus;
            Poco::Int64 dbHasActiveRequest = 0;
            std::string dbUsername;
            std::string dbPassword;
            std::string uname = username;
            session << "SELECT id, name, user_id, location, phone_no, description, status, has_active_request, username, password "
                   << "FROM people_in_crisis WHERE username = ?",
                into(dbId), into(dbName), into(dbUserId), into(dbLocation), into(dbPhoneNo),
                into(dbDescription), into(dbStatus), into(dbHasActiveRequest), into(dbUsername), into(dbPassword),
                use(uname), now;
            
            person.setId(static_cast<int>(dbId));
            person.setName(dbName);
            person.setUserID(static_cast<int>(dbUserId));
            person.setLocation(dbLocation);
            person.setPhoneNo(dbPhoneNo);
            person.setDescription(dbDescription);
            person.setStatus(dbStatus);
            person.setHasActiveRequest(dbHasActiveRequest != 0);
            person.setUsername(dbUsername);
            person.setPassword(dbPassword);
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding PeopleInCrisis by username: " << e.what() << std::endl;
        }
        
        return person;
    }
    
    static std::vector<PeopleInCrisis> findAll() {
        std::vector<PeopleInCrisis> people;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Statement select(session);
            select << "SELECT id, name, user_id, location, phone_no, description, status, has_active_request, username "
                  << "FROM people_in_crisis";
            
            select.execute();
            
            Poco::Data::RecordSet rs(select);
            
            bool more = rs.moveFirst();
            while (more) {
                PeopleInCrisis person;
                person.setId(rs[0].convert<int>());
                person.setName(rs[1].convert<std::string>());
                person.setUserID(rs[2].convert<int>());
                person.setLocation(rs[3].convert<std::string>());
                person.setPhoneNo(rs[4].convert<std::string>());
                person.setDescription(rs[5].convert<std::string>());
                person.setStatus(rs[6].convert<std::string>());
                person.setHasActiveRequest(rs[7].convert<int>() != 0);
                person.setUsername(rs[8].convert<std::string>());
                
                people.push_back(person);
                more = rs.moveNext();
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding all PeopleInCrisis: " << e.what() << std::endl;
        }
        
        return people;
    }
    
    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM people_in_crisis WHERE id = ?", use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error removing PeopleInCrisis: " << e.what() << std::endl;
            return false;
        }
    }
};

namespace Poco {
namespace Data {

template <>
class TypeHandler<PeopleInCrisis> {
public:
    static std::size_t size() {
        return 10; // Number of fields in PeopleInCrisis
    }

    static void bind(std::size_t pos, const PeopleInCrisis& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir) {
        poco_assert_dbg(!pBinder.isNull());
        TypeHandler<int>::bind(pos++, obj.getId(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getName(), pBinder, dir);
        TypeHandler<int>::bind(pos++, obj.getUserID(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getLocation(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getPhoneNo(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getDescription(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getStatus(), pBinder, dir);
        TypeHandler<bool>::bind(pos++, obj.getHasActiveRequest(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getUsername(), pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.getPassword(), pBinder, dir);
    }

    static void extract(std::size_t pos, PeopleInCrisis& obj, const PeopleInCrisis& defVal, AbstractExtractor::Ptr pExt) {
        poco_assert_dbg(!pExt.isNull());
        int id = 0;
        std::string name;
        int userID = 0;
        std::string location;
        std::string phoneNo;
        std::string description;
        std::string status;
        bool hasActiveRequest = false;
        std::string username;
        std::string password;

        TypeHandler<int>::extract(pos++, id, defVal.getId(), pExt);
        TypeHandler<std::string>::extract(pos++, name, defVal.getName(), pExt);
        TypeHandler<int>::extract(pos++, userID, defVal.getUserID(), pExt);
        TypeHandler<std::string>::extract(pos++, location, defVal.getLocation(), pExt);
        TypeHandler<std::string>::extract(pos++, phoneNo, defVal.getPhoneNo(), pExt);
        TypeHandler<std::string>::extract(pos++, description, defVal.getDescription(), pExt);
        TypeHandler<std::string>::extract(pos++, status, defVal.getStatus(), pExt);
        TypeHandler<bool>::extract(pos++, hasActiveRequest, defVal.getHasActiveRequest(), pExt);
        TypeHandler<std::string>::extract(pos++, username, defVal.getUsername(), pExt);
        TypeHandler<std::string>::extract(pos++, password, defVal.getPassword(), pExt);

        obj.setId(id);
        obj.setName(name);
        obj.setUserID(userID);
        obj.setLocation(location);
        obj.setPhoneNo(phoneNo);
        obj.setDescription(description);
        obj.setStatus(status);
        obj.setHasActiveRequest(hasActiveRequest);
        obj.setUsername(username);
        obj.setPassword(password);
    }

    static void prepare(std::size_t pos, const PeopleInCrisis& obj, AbstractPreparator::Ptr pPrep) {
        poco_assert_dbg(!pPrep.isNull());
        TypeHandler<int>::prepare(pos++, obj.getId(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getName(), pPrep);
        TypeHandler<int>::prepare(pos++, obj.getUserID(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getLocation(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getPhoneNo(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getDescription(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getStatus(), pPrep);
        TypeHandler<bool>::prepare(pos++, obj.getHasActiveRequest(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getUsername(), pPrep);
        TypeHandler<std::string>::prepare(pos++, obj.getPassword(), pPrep);
    }
};

} // namespace Data
} // namespace Poco