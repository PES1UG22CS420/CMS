#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Model class for volunteers
class Volunteer {
private:
    int id;
    std::string name;
    std::string location;
    bool available;
    std::string username;
    std::string password;
    std::vector<int> assignedTasks;
    std::string orgType;

public:
    // Constructor
    Volunteer() : id(0), available(true) {}
    
    // Getters
    int getUserID() const { return id; }
    std::string getName() const { return name; }
    std::string getLocation() const { return location; }
    bool isAvailable() const { return available; }
    std::string getUsername() const { return username; }
    std::vector<int> getAssignedTasks() const { return assignedTasks; }
    std::string getOrgType() const { return orgType; }

    // Setters
    void setUserID(int id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setLocation(const std::string& location) { this->location = location; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPassword(const std::string& password) { this->password = password; }
    void setOrgType(const std::string& orgType) { this->orgType = orgType; }

    // Method from class diagram
    void setAvailability(bool status) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "UPDATE volunteers SET available = ? WHERE id = ?",
                use(status), use(id), now;
            available = status;  // Only update the local state if the database update succeeds
        } catch (const std::exception& e) {
            std::cerr << "Error updating availability: " << e.what() << std::endl;
            throw;  // Re-throw to let the controller handle the error
        }
    }

    // Database operations
    bool save() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string nameCopy = name;
            std::string locationCopy = location;
            std::string usernameCopy = username;
            std::string passwordCopy = password;
            std::string orgTypeCopy = orgType;

            if (id == 0) {
                // Insert new record
                session << "INSERT INTO volunteers (name, location, available, username, password, org_type) VALUES (?, ?, ?, ?, ?, ?)",
                    use(nameCopy), use(locationCopy), use(available), use(usernameCopy), use(passwordCopy), use(orgTypeCopy), now;
                
                // Get the inserted ID
                session << "SELECT last_insert_rowid()", into(id), now;
            } else {
                // Update existing record
                session << "UPDATE volunteers SET name = ?, location = ?, available = ?, username = ?, password = ?, org_type = ? WHERE id = ?",
                    use(nameCopy), use(locationCopy), use(available), use(usernameCopy), use(passwordCopy), use(orgTypeCopy), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving volunteer: " << e.what() << std::endl;
            return false;
        }
    }

    static Volunteer findById(int id) {
        Volunteer volunteer;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "SELECT id, name, location, available, username, password, org_type FROM volunteers WHERE id = ?",
                into(volunteer.id), into(volunteer.name), into(volunteer.location),
                into(volunteer.available), into(volunteer.username), into(volunteer.password),
                into(volunteer.orgType), use(id), now;
            
            if (volunteer.id != 0) {
                volunteer.loadAssignedTasks();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding volunteer: " << e.what() << std::endl;
        }
        return volunteer;
    }

    static Volunteer findByUsername(const std::string& username) {
        Volunteer volunteer;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string usernameCopy = username;  // Create non-const copy
            session << "SELECT id, name, location, available, username, password, org_type FROM volunteers WHERE username = ?",
                into(volunteer.id), into(volunteer.name), into(volunteer.location),
                into(volunteer.available), into(volunteer.username), into(volunteer.password),
                into(volunteer.orgType), use(usernameCopy), now;
            
            if (volunteer.id != 0) {
                volunteer.loadAssignedTasks();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding volunteer: " << e.what() << std::endl;
        }
        return volunteer;
    }

    static std::vector<Volunteer> findAll() {
        std::vector<Volunteer> volunteers;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int id;
            std::string name, location, username, password, orgType;
            bool available;

            select << "SELECT id, name, location, available, username, password, org_type FROM volunteers",
                into(id), into(name), into(location), into(available), into(username), into(password), into(orgType),
                range(0, 1);

            while (!select.done()) {
                select.execute();
                Volunteer volunteer;
                volunteer.id = id;
                volunteer.name = name;
                volunteer.location = location;
                volunteer.available = available;
                volunteer.username = username;
                volunteer.password = password;
                volunteer.orgType = orgType;
                volunteer.loadAssignedTasks();
                volunteers.push_back(volunteer);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding all volunteers: " << e.what() << std::endl;
        }
        return volunteers;
    }

    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM volunteers WHERE id = ?", use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error removing volunteer: " << e.what() << std::endl;
            return false;
        }
    }

    // Task management
    void loadAssignedTasks() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int taskId;

            select << "SELECT id FROM tasks WHERE assigned_volunteer_id = ?",
                into(taskId), use(id), range(0, 1);

            assignedTasks.clear();
            while (!select.done()) {
                select.execute();
                assignedTasks.push_back(taskId);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading assigned tasks: " << e.what() << std::endl;
        }
    }

    bool assignTask(int taskId) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "UPDATE tasks SET assigned_volunteer_id = ?, status = 'Assigned' WHERE id = ?",
                use(id), use(taskId), now;
            assignedTasks.push_back(taskId);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error assigning task: " << e.what() << std::endl;
            return false;
        }
    }

    bool completeTask(int taskId) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "UPDATE tasks SET status = 'Completed' WHERE id = ? AND assigned_volunteer_id = ?",
                use(taskId), use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error completing task: " << e.what() << std::endl;
            return false;
        }
    }

    // Authentication
    bool verifyPassword(const std::string& password) const {
        return this->password == password;
    }

    // Convert to JSON for API responses
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("name", name);
        json->set("location", location);
        json->set("available", available);
        json->set("username", username);
        
        Poco::JSON::Array tasksArray;
        for (const auto& taskId : assignedTasks) {
            tasksArray.add(taskId);
        }
        json->set("assignedTasks", tasksArray);
        
        return json;
    }

    // Create from JSON for API requests
    static Volunteer fromJSON(const Poco::JSON::Object::Ptr& json) {
        Volunteer volunteer;
        volunteer.setName(json->getValue<std::string>("name"));
        volunteer.setLocation(json->getValue<std::string>("location"));
        volunteer.setUsername(json->getValue<std::string>("username"));
        volunteer.setPassword(json->getValue<std::string>("password"));
        volunteer.setOrgType(json->getValue<std::string>("orgType"));
        return volunteer;
    }
};