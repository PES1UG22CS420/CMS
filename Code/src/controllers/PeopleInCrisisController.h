#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/PeopleInCrisis.h"
#include "../database/DatabaseManager.h"

// Controller for PeopleInCrisis
class PeopleInCrisisController {
private:
    DatabaseManager* dbManager;

public:
    // Constructor
    PeopleInCrisisController() {
        dbManager = DatabaseManager::getInstance();
    }

    // Methods from class diagram
    bool signUp(const Poco::JSON::Object::Ptr& data) {
        try {
            std::string name = data->getValue<std::string>("name");
            std::string location = data->getValue<std::string>("location");
            std::string phoneNo = data->getValue<std::string>("phoneNo");
            std::string username = data->getValue<std::string>("username");
            std::string password = data->getValue<std::string>("password");
            
            // Check if username already exists
            Session& session = dbManager->getSession();
            Poco::Int64 count = 0;
            std::string usernameCopy = username;  // Create non-const copy
            session << "SELECT COUNT(*) FROM people_in_crisis WHERE username = ?", 
                into(count), use(usernameCopy), now;
            
            if (count > 0) {
                return false; // Username already exists
            }
            
            // Create new person
            PeopleInCrisis person;
            person.setName(name);
            person.setLocation(location);
            person.setPhoneNo(phoneNo);
            person.setUsername(username);
            person.setPassword(password);
            person.setStatus("Pending");
            person.setHasActiveRequest(false);
            
            return person.save();
        } catch (const std::exception& e) {
            std::cerr << "Error signing up: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool enterHelpRequest(const std::string& description, const std::string& location, int userId) {
        try {
            PeopleInCrisis person = PeopleInCrisis::findById(userId);
            if (person.getId() == 0) {
                return false;
            }
            
            person.setDescription(description);
            person.setLocation(location);
            person.setHasActiveRequest(true);
            person.setStatus("Pending");
            
            // Save the updated person
            return person.save();
        } catch (const std::exception& e) {
            std::cerr << "Error entering help request: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool trackRequest(int userId) {
        PeopleInCrisis person = PeopleInCrisis::findById(userId);
        if (person.getId() == 0) {
            return false;
        }
        
        return person.getHasActiveRequest();
    }
    
    // Authentication
    bool authenticate(const std::string& username, const std::string& password) {
        PeopleInCrisis person = PeopleInCrisis::findByUsername(username);
        if (person.getId() == 0) {
            return false;
        }
        
        return person.verifyPassword(password);
    }
    
    // CRUD operations
    PeopleInCrisis getById(int id) {
        return PeopleInCrisis::findById(id);
    }
    
    PeopleInCrisis getByUsername(const std::string& username) {
        return PeopleInCrisis::findByUsername(username);
    }
    
    std::vector<PeopleInCrisis> getAll() {
        return PeopleInCrisis::findAll();
    }
    
    bool updateStatus(int id, const std::string& status) {
        PeopleInCrisis person = PeopleInCrisis::findById(id);
        if (person.getId() == 0) {
            return false;
        }
        
        person.updateStatus(status);
        return true;
    }
    
    bool remove(int id) {
        return PeopleInCrisis::remove(id);
    }

    PeopleInCrisis getProfile(int id) {
        auto session = dbManager->getSession();
        PeopleInCrisis person;
        
        try {
            Poco::Data::Statement select(session);
            select << "SELECT * FROM people_in_crisis WHERE id = ?",
                into(person),
                use(id),
                now;
            
            if (select.done()) {
                return person;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting profile: " << e.what() << std::endl;
        }
        
        return person;
    }

    bool updateProfile(int id, Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            std::string name = json->getValue<std::string>("name");
            std::string location = json->getValue<std::string>("location");
            std::string phone = json->getValue<std::string>("phone");
            
            Poco::Data::Statement update(session);
            update << "UPDATE people_in_crisis SET name = ?, location = ?, phone_no = ? WHERE id = ?",
                use(name),
                use(location),
                use(phone),
                use(id),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating profile: " << e.what() << std::endl;
            return false;
        }
    }
};