#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

// Model class for help requests
class HelpRequest {
private:
    int id;
    int requesterId;
    std::string type;
    std::string description;
    std::string location;
    int urgency;
    std::string status;
    std::string timestamp;

public:
    // Constructor
    HelpRequest() : id(0), requesterId(0), urgency(3) {}
    
    // Getters
    int getId() const { return id; }
    int getRequesterId() const { return requesterId; }
    std::string getType() const { return type; }
    std::string getDescription() const { return description; }
    std::string getLocation() const { return location; }
    int getUrgency() const { return urgency; }
    std::string getStatus() const { return status; }
    std::string getTimestamp() const { return timestamp; }

    // Setters
    void setId(int id) { this->id = id; }
    void setRequesterId(int requesterId) { this->requesterId = requesterId; }
    void setType(const std::string& type) { this->type = type; }
    void setDescription(const std::string& description) { this->description = description; }
    void setLocation(const std::string& location) { this->location = location; }
    void setUrgency(int urgency) { this->urgency = urgency; }
    void setStatus(const std::string& status) { this->status = status; }
    void setTimestamp(const std::string& timestamp) { this->timestamp = timestamp; }

    // Convert to JSON for API responses
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("requesterId", requesterId);
        json->set("type", type);
        json->set("description", description);
        json->set("location", location);
        json->set("urgency", urgency);
        json->set("status", status);
        json->set("timestamp", timestamp);
        return json;
    }

    // Create from JSON for API requests
    static HelpRequest fromJSON(const Poco::JSON::Object::Ptr& json) {
        HelpRequest request;
        
        if (json->has("id")) {
            request.setId(json->getValue<int>("id"));
        }
        
        if (json->has("requesterId")) {
            request.setRequesterId(json->getValue<int>("requesterId"));
        }
        
        if (json->has("type")) {
            request.setType(json->getValue<std::string>("type"));
        }
        
        if (json->has("description")) {
            request.setDescription(json->getValue<std::string>("description"));
        }
        
        if (json->has("location")) {
            request.setLocation(json->getValue<std::string>("location"));
        }
        
        if (json->has("urgency")) {
            request.setUrgency(json->getValue<int>("urgency"));
        }
        
        if (json->has("status")) {
            request.setStatus(json->getValue<std::string>("status"));
        } else {
            request.setStatus("Pending");
        }
        
        if (json->has("timestamp")) {
            request.setTimestamp(json->getValue<std::string>("timestamp"));
        } else {
            // Set current timestamp
            Poco::DateTime now;
            request.setTimestamp(Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
        }
        
        return request;
    }
    
    // Database operations
    bool save() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            // Set timestamp if not set
            if (timestamp.empty()) {
                Poco::DateTime now;
                timestamp = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S");
            }
            
            if (id == 0) {
                // Insert new record
                session << "INSERT INTO help_requests (requester_id, type, description, location, urgency, status, timestamp) "
                       << "VALUES (?, ?, ?, ?, ?, ?, ?)",
                    use(requesterId), use(type), use(description), use(location), 
                    use(urgency), use(status), use(timestamp), now;
                
                // Get the last inserted ID
                Poco::Int64 lastId = 0;
                session << "SELECT last_insert_rowid()", into(lastId), now;
                id = static_cast<int>(lastId);
            } else {
                // Update existing record
                session << "UPDATE help_requests SET requester_id = ?, type = ?, description = ?, location = ?, "
                       << "urgency = ?, status = ?, timestamp = ? WHERE id = ?",
                    use(requesterId), use(type), use(description), use(location), 
                    use(urgency), use(status), use(timestamp), use(id), now;
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving HelpRequest: " << e.what() << std::endl;
            return false;
        }
    }
    
    static HelpRequest findById(int id) {
        HelpRequest request;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Poco::Int64 dbId = 0;
            Poco::Int64 dbRequesterId = 0;
            std::string dbType;
            std::string dbDescription;
            std::string dbLocation;
            Poco::Int64 dbUrgency = 0;
            std::string dbStatus;
            std::string dbTimestamp;
            
            session << "SELECT id, requester_id, type, description, location, urgency, status, timestamp "
                   << "FROM help_requests WHERE id = ?",
                into(dbId), into(dbRequesterId), into(dbType), into(dbDescription),
                into(dbLocation), into(dbUrgency), into(dbStatus), into(dbTimestamp),
                use(id), now;
            
            request.setId(static_cast<int>(dbId));
            request.setRequesterId(static_cast<int>(dbRequesterId));
            request.setType(dbType);
            request.setDescription(dbDescription);
            request.setLocation(dbLocation);
            request.setUrgency(static_cast<int>(dbUrgency));
            request.setStatus(dbStatus);
            request.setTimestamp(dbTimestamp);
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding HelpRequest by ID: " << e.what() << std::endl;
        }
        
        return request;
    }
    
    static std::vector<HelpRequest> findByRequesterId(int requesterId) {
        std::vector<HelpRequest> requests;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Statement select(session);
            select << "SELECT id, requester_id, type, description, location, urgency, status, timestamp "
                  << "FROM help_requests WHERE requester_id = ?",
                use(requesterId);
            
            select.execute();
            
            Poco::Data::RecordSet rs(select);
            
            bool more = rs.moveFirst();
            while (more) {
                HelpRequest request;
                request.setId(rs[0].convert<int>());
                request.setRequesterId(rs[1].convert<int>());
                request.setType(rs[2].convert<std::string>());
                request.setDescription(rs[3].convert<std::string>());
                request.setLocation(rs[4].convert<std::string>());
                request.setUrgency(rs[5].convert<int>());
                request.setStatus(rs[6].convert<std::string>());
                request.setTimestamp(rs[7].convert<std::string>());
                
                requests.push_back(request);
                more = rs.moveNext();
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding HelpRequests by requester ID: " << e.what() << std::endl;
        }
        
        return requests;
    }
    
    static std::vector<HelpRequest> findAll() {
        std::vector<HelpRequest> requests;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            Statement select(session);
            select << "SELECT id, requester_id, type, description, location, urgency, status, timestamp "
                  << "FROM help_requests";
            
            select.execute();
            
            Poco::Data::RecordSet rs(select);
            
            bool more = rs.moveFirst();
            while (more) {
                HelpRequest request;
                request.setId(rs[0].convert<int>());
                request.setRequesterId(rs[1].convert<int>());
                request.setType(rs[2].convert<std::string>());
                request.setDescription(rs[3].convert<std::string>());
                request.setLocation(rs[4].convert<std::string>());
                request.setUrgency(rs[5].convert<int>());
                request.setStatus(rs[6].convert<std::string>());
                request.setTimestamp(rs[7].convert<std::string>());
                
                requests.push_back(request);
                more = rs.moveNext();
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error finding all HelpRequests: " << e.what() << std::endl;
        }
        
        return requests;
    }
    
    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM help_requests WHERE id = ?", use(id), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error removing HelpRequest: " << e.what() << std::endl;
            return false;
        }
    }
};