#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include "../database/DatabaseManager.h"
#include "AlertSystem.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Model class for admin
class Admin {
private:
    int id;
    std::string name;
    std::string username;
    std::string password;
    AlertSystem* alertSystem;

public:
    // Constructor
    Admin() : id(0), alertSystem(nullptr) {}
    
    Admin(AlertSystem* alertSystem) : id(0), alertSystem(alertSystem) {}

    // Getters
    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }

    // Setters
    void setId(int id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPassword(const std::string& password) { this->password = password; }

    // Database operations
    bool save() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string nameCopy = name;
            std::string usernameCopy = username;
            std::string passwordCopy = password;

            if (id == 0) {
                // Insert new record
                session << "INSERT INTO admins (name, username, password) VALUES (?, ?, ?)",
                    use(nameCopy), use(usernameCopy), use(passwordCopy), now;
                
                // Get the inserted ID
                session << "SELECT last_insert_rowid()", into(id), now;
            } else {
                // Update existing record
                session << "UPDATE admins SET name = ?, username = ?, password = ? WHERE id = ?",
                    use(nameCopy), use(usernameCopy), use(passwordCopy), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving admin: " << e.what() << std::endl;
            return false;
        }
    }

    static Admin findById(int id) {
        Admin admin;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "SELECT id, name, username, password FROM admins WHERE id = ?",
                into(admin.id), into(admin.name), into(admin.username), into(admin.password),
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error finding admin: " << e.what() << std::endl;
        }
        return admin;
    }

    static Admin findByUsername(const std::string& username) {
        Admin admin;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string usernameCopy = username;  // Create non-const copy
            
            // First check if the admin exists
            Poco::Int64 count = 0;
            session << "SELECT COUNT(*) FROM admins WHERE username = ?",
                into(count),
                use(usernameCopy),
                now;
            
            if (count > 0) {
                // If exists, get the admin details
                session << "SELECT id, name, username, password FROM admins WHERE username = ?",
                    into(admin.id),
                    into(admin.name),
                    into(admin.username),
                    into(admin.password),
                    use(usernameCopy),
                    now;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding admin by username: " << e.what() << std::endl;
        }
        return admin;
    }

    static std::vector<Admin> findAll() {
        std::vector<Admin> admins;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int id;
            std::string name, username, password;

            select << "SELECT id, name, username, password FROM admins",
                into(id), into(name), into(username), into(password),
                range(0, 1);

            while (!select.done()) {
                select.execute();
                Admin admin;
                admin.id = id;
                admin.name = name;
                admin.username = username;
                admin.password = password;
                admins.push_back(admin);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding all admins: " << e.what() << std::endl;
        }
        return admins;
    }

    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM admins WHERE id = ?", use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error removing admin: " << e.what() << std::endl;
            return false;
        }
    }

    // Authentication
    bool verifyPassword(const std::string& inputPassword) const {
        try {
            // For now, just do a direct comparison since we're not hashing passwords yet
            return password == inputPassword;
        } catch (const std::exception& e) {
            std::cerr << "Error verifying password: " << e.what() << std::endl;
            return false;
        }
    }

    // Alert system management
    void configureAlertSystem(const std::string& type, bool enabled) {
        if (alertSystem) {
            alertSystem->setAlertEnabled(type, enabled);
        }
    }

    void setAlertThreshold(const std::string& type, int threshold) {
        if (alertSystem) {
            alertSystem->setUrgencyThreshold(threshold);
        }
    }

    // Methods from class diagram
    void verifyHelpRequest(int requestId) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "UPDATE help_requests SET verified = 1, verified_by = ? WHERE id = ?",
                use(id), use(requestId), now;
        } catch (const std::exception& e) {
            std::cerr << "Error verifying help request: " << e.what() << std::endl;
        }
    }

    void manageAccounts() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            // Implementation for account management
            session << "UPDATE user_accounts SET last_managed = datetime('now') WHERE managed_by = ?",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error managing accounts: " << e.what() << std::endl;
        }
    }

    void secureSystem() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            // Implementation for system security
            session << "INSERT INTO security_logs (admin_id, action, timestamp) VALUES (?, 'Security Check', datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error securing system: " << e.what() << std::endl;
        }
    }

    // Convert to JSON for API responses
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("name", name);
        json->set("username", username);
        return json;
    }

    // Create from JSON for API requests
    static Admin fromJSON(const Poco::JSON::Object::Ptr& json) {
        Admin admin;
        admin.setName(json->getValue<std::string>("name"));
        admin.setUsername(json->getValue<std::string>("username"));
        admin.setPassword(json->getValue<std::string>("password"));
        return admin;
    }
};