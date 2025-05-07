#pragma once

#include <string>
#include <vector>
#include <map>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include "../database/DatabaseManager.h"
#include "PeopleInCrisis.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Model class for relief providers
class ReliefProvider {
private:
    int id;
    std::string name;
    std::string orgType;
    std::string location;
    std::string username;
    std::string password;
    std::vector<int> incidentReports;
    std::map<std::string, int> resources; // resource type -> quantity

public:
    // Constructor
    ReliefProvider() : id(0) {}
    
    ReliefProvider(const std::string& name, const std::string& orgType) 
        : id(0), name(name), orgType(orgType) {}

    // Getters
    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getOrgType() const { return orgType; }
    std::string getLocation() const { return location; }
    std::string getUsername() const { return username; }
    std::vector<int> getIncidentReports() const { return incidentReports; }
    std::map<std::string, int> getResources() const { return resources; }

    // Setters
    void setId(int id) { this->id = id; }
    void setName(const std::string& name) { this->name = name; }
    void setOrgType(const std::string& orgType) { this->orgType = orgType; }
    void setLocation(const std::string& location) { this->location = location; }
    void setUsername(const std::string& username) { this->username = username; }
    void setPassword(const std::string& password) { this->password = password; }

    // Methods from class diagram
    void accessIncidentReports() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int reportId;
            std::string description;
            std::string timestamp;

            select << "SELECT id, description, timestamp FROM incident_reports WHERE provider_id = ?",
                into(reportId), into(description), into(timestamp), use(id), range(0, 1);

            incidentReports.clear();
            while (!select.done()) {
                select.execute();
                if (reportId > 0) {
                incidentReports.push_back(reportId);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error accessing incident reports: " << e.what() << std::endl;
        }
    }
    
    std::string categorizeRequest(const PeopleInCrisis& request) {
        // Simple categorization based on description
        std::string desc = request.getDescription();
        
        if (desc.find("food") != std::string::npos) {
            return "Food";
        } else if (desc.find("shelter") != std::string::npos) {
            return "Shelter";
        } else if (desc.find("medical") != std::string::npos) {
            return "Medical";
        } else {
            return "Other";
        }
    }

    // New methods from class diagram
    void requestManpower() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO manpower_requests (provider_id, status, request_date) VALUES (?, 'Pending', datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting manpower: " << e.what() << std::endl;
        }
    }

    void requestGovtAid() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO government_aid_requests (provider_id, status, request_date) VALUES (?, 'Pending', datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting government aid: " << e.what() << std::endl;
        }
    }

    void requestAdditionalAid() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO additional_aid_requests (provider_id, status, request_date) VALUES (?, 'Pending', datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting additional aid: " << e.what() << std::endl;
        }
    }

    void requestMonetaryService() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO monetary_service_requests (provider_id, status, request_date) VALUES (?, 'Pending', datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error requesting monetary service: " << e.what() << std::endl;
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

            if (id == 0) {
                // Insert new record
                session << "INSERT INTO relief_providers (name, location, username, password) VALUES (?, ?, ?, ?)",
                    use(nameCopy), use(locationCopy), use(usernameCopy), use(passwordCopy), now;
                
                // Get the inserted ID
                session << "SELECT last_insert_rowid()", into(id), now;
            } else {
                // Update existing record
                session << "UPDATE relief_providers SET name = ?, location = ?, username = ?, password = ? WHERE id = ?",
                    use(nameCopy), use(locationCopy), use(usernameCopy), use(passwordCopy), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving relief provider: " << e.what() << std::endl;
            return false;
        }
    }

    static ReliefProvider findById(int id) {
        ReliefProvider provider;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "SELECT id, name, org_type, location, username, password FROM relief_providers WHERE id = ?",
                into(provider.id), into(provider.name), into(provider.orgType), 
                into(provider.location), into(provider.username), into(provider.password),
                use(id), now;
            
            if (provider.id != 0) {
                provider.loadResources();
                provider.accessIncidentReports();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding relief provider: " << e.what() << std::endl;
        }
        return provider;
    }

    static ReliefProvider findByUsername(const std::string& username) {
        ReliefProvider provider;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string usernameCopy = username;  // Create non-const copy
            session << "SELECT id, name, org_type, location, username, password FROM relief_providers WHERE username = ?",
                into(provider.id), into(provider.name), into(provider.orgType),
                into(provider.location), into(provider.username), into(provider.password),
                use(usernameCopy), now;
            
            if (provider.id != 0) {
                provider.loadResources();
                provider.accessIncidentReports();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding relief provider: " << e.what() << std::endl;
        }
        return provider;
    }

    static std::vector<ReliefProvider> findAll() {
        std::vector<ReliefProvider> providers;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int id;
            std::string name, orgType, location, username, password;

            select << "SELECT id, name, org_type, location, username, password FROM relief_providers",
                into(id), into(name), into(orgType), into(location), into(username), into(password),
                range(0, 1);

            while (!select.done()) {
                select.execute();
                ReliefProvider provider;
                provider.id = id;
                provider.name = name;
                provider.orgType = orgType;
                provider.location = location;
                provider.username = username;
                provider.password = password;
                provider.loadResources();
                provider.accessIncidentReports();
                providers.push_back(provider);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error finding all relief providers: " << e.what() << std::endl;
        }
        return providers;
    }

    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM relief_providers WHERE id = ?", use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error removing relief provider: " << e.what() << std::endl;
            return false;
        }
    }

    // Resource management
    void addResource(const std::string& type, int quantity) {
        resources[type] += quantity;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string typeCopy = type;  // Create non-const copy
            session << "INSERT OR REPLACE INTO provider_resources (provider_id, resource_type, quantity) VALUES (?, ?, ?)",
                use(id), use(typeCopy), use(resources[type]), now;
        } catch (const std::exception& e) {
            std::cerr << "Error adding resource: " << e.what() << std::endl;
        }
    }

    bool useResource(const std::string& type, int quantity) {
        if (resources[type] < quantity) return false;
        resources[type] -= quantity;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string typeCopy = type;  // Create non-const copy
            session << "UPDATE provider_resources SET quantity = ? WHERE provider_id = ? AND resource_type = ?",
                use(resources[type]), use(id), use(typeCopy), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error using resource: " << e.what() << std::endl;
            return false;
        }
    }

    void loadResources() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            std::string type;
            int quantity;

            select << "SELECT resource_type, quantity FROM provider_resources WHERE provider_id = ?",
                into(type), into(quantity), use(id), range(0, 1);

            resources.clear();
            while (!select.done()) {
                select.execute();
                resources[type] = quantity;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading resources: " << e.what() << std::endl;
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
        json->set("orgType", orgType);
        json->set("location", location);
        json->set("username", username);
        
        Poco::JSON::Array reportsArray;
        for (const auto& reportId : incidentReports) {
            reportsArray.add(reportId);
        }
        json->set("incidentReports", reportsArray);
        
        Poco::JSON::Object::Ptr resourcesObj = new Poco::JSON::Object();
        for (const auto& pair : resources) {
            resourcesObj->set(pair.first, pair.second);
        }
        json->set("resources", resourcesObj);
        
        return json;
    }

    // Create from JSON for API requests
    static ReliefProvider fromJSON(const Poco::JSON::Object::Ptr& json) {
        ReliefProvider provider;
        provider.setName(json->getValue<std::string>("name"));
        provider.setOrgType(json->getValue<std::string>("orgType"));
        provider.setLocation(json->getValue<std::string>("location"));
        provider.setUsername(json->getValue<std::string>("username"));
        provider.setPassword(json->getValue<std::string>("password"));
        return provider;
    }
};