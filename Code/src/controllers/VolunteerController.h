#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/Volunteer.h"
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Controller for Volunteer
class VolunteerController {
public:
    // Constructor
    VolunteerController() {}

    // Methods from class diagram
    bool signUp(const Poco::JSON::Object::Ptr& json) {
        try {
            // Extract user data
            std::string name = json->getValue<std::string>("name");
            std::string location = json->getValue<std::string>("location");
            std::string username = json->getValue<std::string>("username");
            std::string password = json->getValue<std::string>("password");
            std::string orgType = json->getValue<std::string>("orgType");
            
            // Check if username already exists
            Session& session = DatabaseManager::getInstance()->getSession();
            Poco::Int64 count = 0;
            session << "SELECT COUNT(*) FROM volunteers WHERE username = ?", 
                into(count), use(username), now;
            
            if (count > 0) {
                return false; // Username already exists
            }
            
            // Create new volunteer
            Volunteer volunteer;
            volunteer.setName(name);
            volunteer.setLocation(location);
            volunteer.setUsername(username);
            volunteer.setPassword(password);
            volunteer.setOrgType(orgType);
            volunteer.setAvailability(true);
            
            if (volunteer.save()) {
                // Create verification record
                std::string userType = "volunteer";
                int userId = volunteer.getUserID();
                Poco::Data::Statement verifyInsert(session);
                verifyInsert << "INSERT INTO account_verifications (user_type, user_id, status, created_at) VALUES (?, ?, 'pending', datetime('now'))",
                    use(userType), use(userId), now;
                return true;
            }
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error signing up volunteer: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool donate(double amount, int volunteerId) {
        try {
            // First check if volunteer exists
            Volunteer volunteer = Volunteer::findById(volunteerId);
            if (volunteer.getUserID() == 0) {
                std::cerr << "Volunteer not found with ID: " << volunteerId << std::endl;
                return false;
            }
            
            // Process the donation
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO donations (volunteer_id, amount, timestamp) VALUES (?, ?, datetime('now'))",
                use(volunteerId), use(amount), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error processing donation: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool helpRequest(const std::string& description, int volunteerId) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string descCopy = description;  // Create non-const copy
            session << "INSERT INTO volunteer_help_requests (volunteer_id, description, timestamp) VALUES (?, ?, datetime('now'))",
                use(volunteerId), use(descCopy), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error registering help request: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool acceptRequest(int volunteerId, int requestId) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO volunteer_assignments (volunteer_id, request_id, timestamp) VALUES (?, ?, datetime('now'))",
                use(volunteerId), use(requestId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error accepting request: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::vector<int> getVolunteerHistory(int volunteerId) {
        std::vector<int> history;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int requestId;

            select << "SELECT request_id FROM volunteer_assignments WHERE volunteer_id = ? ORDER BY timestamp DESC",
                into(requestId), use(volunteerId), range(0, 1);

            while (!select.done()) {
                select.execute();
                history.push_back(requestId);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting volunteer history: " << e.what() << std::endl;
        }
        return history;
    }
    
    // CRUD operations
    Volunteer getById(int id) {
        return Volunteer::findById(id);
    }
    
    Volunteer getByUsername(const std::string& username) {
        std::string usernameCopy = username;  // Create non-const copy
        return Volunteer::findByUsername(usernameCopy);
    }
    
    std::vector<Volunteer> getAll() {
        return Volunteer::findAll();
    }
    
    bool updateAvailability(int id, bool available) {
        try {
        Volunteer volunteer = Volunteer::findById(id);
        if (volunteer.getUserID() == 0) {
                std::cerr << "Volunteer not found with ID: " << id << std::endl;
                return false;
            }
            volunteer.setAvailability(available);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating volunteer availability: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool remove(int id) {
        return Volunteer::remove(id);
    }
};