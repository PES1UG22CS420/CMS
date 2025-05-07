#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/ReliefProvider.h"
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Controller for ReliefProvider
class ReliefProviderController {
public:
    // Constructor
    ReliefProviderController() {}

    // Methods from class diagram
    bool signUp(const Poco::JSON::Object::Ptr& data) {
        try {
            std::string name = data->getValue<std::string>("name");
            std::string orgType = data->getValue<std::string>("orgType");
            std::string location = data->getValue<std::string>("location");
            std::string username = data->getValue<std::string>("username");
            std::string password = data->getValue<std::string>("password");
            
            // Check if username already exists
            Session& session = DatabaseManager::getInstance()->getSession();
            Poco::Int64 count = 0;
            session << "SELECT COUNT(*) FROM relief_providers WHERE username = ?", 
                into(count), use(username), now;
            
            if (count > 0) {
                return false; // Username already exists
            }
            
            // Create new relief provider
            ReliefProvider provider;
            provider.setName(name);
            provider.setOrgType(orgType);
            provider.setLocation(location);
            provider.setUsername(username);
            provider.setPassword(password);
            
            return provider.save();
        } catch (const std::exception& e) {
            std::cerr << "Error signing up relief provider: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool requestManpower(int providerId) {
        try {
            ReliefProvider provider = ReliefProvider::findById(providerId);
            if (provider.getId() == 0) return false;
            
            // Create a manpower request
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO manpower_requests (provider_id, status, timestamp) VALUES (?, 'Pending', datetime('now'))",
                use(providerId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting manpower: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool requestGovAid(int providerId) {
        try {
            ReliefProvider provider = ReliefProvider::findById(providerId);
            if (provider.getId() == 0) return false;
            
            // Create a government aid request
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO gov_aid_requests (provider_id, status, timestamp) VALUES (?, 'Pending', datetime('now'))",
                use(providerId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting government aid: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool requestAdditionalAid(int providerId) {
        try {
            ReliefProvider provider = ReliefProvider::findById(providerId);
            if (provider.getId() == 0) return false;
            
            // Create an additional aid request
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO additional_aid_requests (provider_id, status, timestamp) VALUES (?, 'Pending', datetime('now'))",
                use(providerId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting additional aid: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool requestMonetaryService(int providerId) {
        try {
            ReliefProvider provider = ReliefProvider::findById(providerId);
            if (provider.getId() == 0) return false;
            
            // Create a monetary service request
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO monetary_requests (provider_id, status, timestamp) VALUES (?, 'Pending', datetime('now'))",
                use(providerId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting monetary service: " << e.what() << std::endl;
            return false;
        }
    }
    
    // CRUD operations
    ReliefProvider getById(int id) {
        return ReliefProvider::findById(id);
    }
    
    ReliefProvider getByUsername(const std::string& username) {
        return ReliefProvider::findByUsername(username);
    }
    
    std::vector<ReliefProvider> getAll() {
        return ReliefProvider::findAll();
    }
    
    bool remove(int id) {
        return ReliefProvider::remove(id);
    }
};