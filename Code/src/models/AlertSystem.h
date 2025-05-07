#pragma once

#include <string>
#include <list>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Model class for the alert system
class AlertSystem {
private:
    int id;
    std::list<std::string> subscribers;
    int urgencyThreshold = 8;
    bool autoAssign = false;
    std::string lastAlertTime;
    std::string lastAlertType;
    std::string lastAlertMessage;

public:
    // Constructor
    AlertSystem() : id(0) {}

    // Getters
    int getId() const { return id; }
    const std::list<std::string>& getSubscribers() const { return subscribers; }
    int getUrgencyThreshold() const { return urgencyThreshold; }
    bool getAutoAssign() const { return autoAssign; }
    std::string getLastAlertTime() const { return lastAlertTime; }
    std::string getLastAlertType() const { return lastAlertType; }
    std::string getLastAlertMessage() const { return lastAlertMessage; }

    // Setters
    void setId(int id) { this->id = id; }
    void setUrgencyThreshold(int threshold) { urgencyThreshold = threshold; }
    void setAutoAssign(bool value) { autoAssign = value; }
    void setLastAlertTime(const std::string& time) { lastAlertTime = time; }
    void setLastAlertType(const std::string& type) { lastAlertType = type; }
    void setLastAlertMessage(const std::string& message) { lastAlertMessage = message; }

    // Database operations
    bool save() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            int autoAssignInt = autoAssign ? 1 : 0;
            
            if (id == 0) {
                session << "INSERT INTO alert_system (urgency_threshold, auto_assign) VALUES (?, ?)",
                    use(urgencyThreshold), use(autoAssignInt), now;
                
                Poco::Int64 lastId;
                session << "SELECT last_insert_rowid()", into(lastId), now;
                id = static_cast<int>(lastId);
            } else {
                session << "UPDATE alert_system SET urgency_threshold = ?, auto_assign = ? WHERE id = ?",
                    use(urgencyThreshold), use(autoAssignInt), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving alert system: " << e.what() << std::endl;
            return false;
        }
    }

    static AlertSystem findById(int id) {
        AlertSystem system;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            int autoAssignInt;
            session << "SELECT id, urgency_threshold, auto_assign, last_alert_time, last_alert_type, last_alert_message "
                   << "FROM alert_system WHERE id = ?",
                into(system.id), into(system.urgencyThreshold), into(autoAssignInt),
                into(system.lastAlertTime), into(system.lastAlertType), into(system.lastAlertMessage),
                use(id), now;
            system.autoAssign = (autoAssignInt == 1);
        } catch (...) {}
        return system;
    }

    static AlertSystem getInstance() {
        AlertSystem system;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            int autoAssignInt;
            session << "SELECT id, urgency_threshold, auto_assign, last_alert_time, last_alert_type, last_alert_message "
                   << "FROM alert_system LIMIT 1",
                into(system.id), into(system.urgencyThreshold), into(autoAssignInt),
                into(system.lastAlertTime), into(system.lastAlertType), into(system.lastAlertMessage), now;
            system.autoAssign = (autoAssignInt == 1);
        } catch (...) {}
        return system;
    }

    // Methods from class diagram
    void broadcastAlertMessage(const std::string& message, const std::string& type = "General") {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            
            // Create non-const copies
            std::string messageCopy = message;
            std::string typeCopy = type;
            
            // Record the alert
            session << "INSERT INTO alert_history (type, message, timestamp) VALUES (?, ?, datetime('now'))",
                use(typeCopy), use(messageCopy), now;
            
            // Update last alert info
            session << "UPDATE alert_system SET last_alert_time = datetime('now'), last_alert_type = ?, last_alert_message = ? WHERE id = ?",
                use(typeCopy), use(messageCopy), use(id), now;
            
            // Notify subscribers
            for (const auto& subscriber : subscribers) {
                std::string subscriberCopy = subscriber;
                session << "INSERT INTO alert_notifications (subscriber, alert_type, message, timestamp) VALUES (?, ?, ?, datetime('now'))",
                    use(subscriberCopy), use(typeCopy), use(messageCopy), now;
            }
            
            // Log the broadcast
            std::cout << "Broadcasting alert: " << message << " to " << subscribers.size() << " subscribers" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error broadcasting alert: " << e.what() << std::endl;
        }
    }
    
    // Add a subscriber
    void addSubscriber(const std::string& subscriber) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string subscriberCopy = subscriber;
            session << "INSERT INTO alert_subscribers (subscriber) VALUES (?)",
                use(subscriberCopy), now;
            subscribers.push_back(subscriber);
        } catch (const std::exception& e) {
            std::cerr << "Error adding subscriber: " << e.what() << std::endl;
        }
    }
    
    // Remove a subscriber
    void removeSubscriber(const std::string& subscriber) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string subscriberCopy = subscriber;
            session << "DELETE FROM alert_subscribers WHERE subscriber = ?",
                use(subscriberCopy), now;
            subscribers.remove(subscriber);
        } catch (const std::exception& e) {
            std::cerr << "Error removing subscriber: " << e.what() << std::endl;
        }
    }

    // Load subscribers from database
    void loadSubscribers() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            std::string subscriber;

            select << "SELECT subscriber FROM alert_subscribers",
                into(subscriber), range(0, 1);

            subscribers.clear();
            while (!select.done()) {
                select.execute();
                subscribers.push_back(subscriber);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading subscribers: " << e.what() << std::endl;
        }
    }

    // Get alert history
    std::vector<std::map<std::string, std::string>> getAlertHistory(int limit = 100) {
        std::vector<std::map<std::string, std::string>> history;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            std::string type, message, timestamp;

            select << "SELECT type, message, timestamp FROM alert_history ORDER BY timestamp DESC LIMIT ?",
                into(type), into(message), into(timestamp), use(limit), range(0, 1);

            while (!select.done()) {
                select.execute();
                std::map<std::string, std::string> alert;
                alert["type"] = type;
                alert["message"] = message;
                alert["timestamp"] = timestamp;
                history.push_back(alert);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting alert history: " << e.what() << std::endl;
        }
        return history;
    }

    // Get pending notifications for a subscriber
    std::vector<std::map<std::string, std::string>> getPendingNotifications(const std::string& subscriber) {
        std::vector<std::map<std::string, std::string>> notifications;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            std::string type, message, timestamp;
            std::string subscriberCopy = subscriber;

            select << "SELECT alert_type, message, timestamp FROM alert_notifications "
                   << "WHERE subscriber = ? AND delivered = 0 ORDER BY timestamp ASC",
                into(type), into(message), into(timestamp), use(subscriberCopy), range(0, 1);

            while (!select.done()) {
                select.execute();
                std::map<std::string, std::string> notification;
                notification["type"] = type;
                notification["message"] = message;
                notification["timestamp"] = timestamp;
                notifications.push_back(notification);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting pending notifications: " << e.what() << std::endl;
        }
        return notifications;
    }

    // Mark notifications as delivered
    bool markNotificationsAsDelivered(const std::string& subscriber) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string subscriberCopy = subscriber;
            session << "UPDATE alert_notifications SET delivered = 1 WHERE subscriber = ? AND delivered = 0",
                use(subscriberCopy), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error marking notifications as delivered: " << e.what() << std::endl;
            return false;
        }
    }

    // Convert to JSON for API responses
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("urgency_threshold", urgencyThreshold);
        json->set("auto_assign", autoAssign);
        json->set("last_alert_time", lastAlertTime);
        json->set("last_alert_type", lastAlertType);
        json->set("last_alert_message", lastAlertMessage);
        
        Poco::JSON::Array subscribersArray;
        for (const auto& subscriber : subscribers) {
            subscribersArray.add(subscriber);
        }
        json->set("subscribers", subscribersArray);
        
        return json;
    }

    // Create from JSON for API requests
    static AlertSystem fromJSON(const Poco::JSON::Object::Ptr& json) {
        AlertSystem system;
        
        if (json->has("id")) {
            system.setId(json->getValue<int>("id"));
        }
        
        if (json->has("urgency_threshold")) {
            system.setUrgencyThreshold(json->getValue<int>("urgency_threshold"));
        }
        
        if (json->has("auto_assign")) {
            system.setAutoAssign(json->getValue<bool>("auto_assign"));
        }
        
        if (json->has("subscribers")) {
            Poco::JSON::Array::Ptr subscribersArray = json->getArray("subscribers");
            for (size_t i = 0; i < subscribersArray->size(); i++) {
                system.addSubscriber(subscribersArray->getElement<std::string>(i));
            }
        }
        
        return system;
    }

    void setAlertEnabled(const std::string& type, bool enabled) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string typeCopy = type;
            session << "UPDATE alert_config SET enabled = ? WHERE alert_type = ?",
                use(enabled), use(typeCopy), now;
        } catch (const std::exception& e) {
            std::cerr << "Error setting alert enabled: " << e.what() << std::endl;
        }
    }
};