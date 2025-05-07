#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/Admin.h"
#include "../models/AlertSystem.h"
#include "../database/DatabaseManager.h"
#include <Poco/JSON/Array.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

// Controller for Admin
class AdminController {
private:
    AlertSystem* alertSystem;
    DatabaseManager* dbManager;

public:
    AdminController(AlertSystem* alertSystem) : alertSystem(alertSystem) {
        dbManager = DatabaseManager::getInstance();
        // Create default admin if none exists
        Session& session = DatabaseManager::getInstance()->getSession();
        Poco::Int64 count = 0;
        session << "SELECT COUNT(*) FROM admins", into(count), now;
        
        if (count == 0) {
            Admin defaultAdmin(alertSystem);
            defaultAdmin.setName("Administrator");
            defaultAdmin.setUsername("admin");
            defaultAdmin.setPassword("admin123");
            defaultAdmin.save();
        }
    }

    // Register a new admin
    bool registerAdmin(const std::string& name, const std::string& username, const std::string& password) {
        try {
            // Check if username already exists
            Session& session = DatabaseManager::getInstance()->getSession();
            Poco::Int64 count = 0;
            std::string usernameCopy = username;  // Create non-const copy
            session << "SELECT COUNT(*) FROM admins WHERE username = ?", 
                into(count), use(usernameCopy), now;
            
            if (count > 0) {
                return false; // Username already exists
            }
            
            // Create new admin
            Admin admin(alertSystem);
            admin.setName(name);
            admin.setUsername(username);
            admin.setPassword(password);
            
            return admin.save();
        } catch (const std::exception& e) {
            std::cerr << "Error registering admin: " << e.what() << std::endl;
            return false;
        }
    }

    // Login admin
    Admin login(const std::string& username, const std::string& password) {
        Admin admin = Admin::findByUsername(username);
        if (admin.getId() != 0 && admin.verifyPassword(password)) {
            Admin newAdmin(alertSystem);
            newAdmin.setId(admin.getId());
            newAdmin.setName(admin.getName());
            newAdmin.setUsername(admin.getUsername());
            newAdmin.setPassword(admin.getPassword());
            return newAdmin;
        }
        return Admin(alertSystem); // return empty admin if login fails
    }

    // Get admin by ID
    Admin getById(int id) {
        Admin admin = Admin::findById(id);
        if (admin.getId() != 0) {
            admin = Admin(alertSystem);  // Recreate with alert system
            admin.setId(admin.getId());
            admin.setName(admin.getName());
            admin.setUsername(admin.getUsername());
            admin.setPassword(admin.getPassword());
        }
        return admin;
    }

    // Get admin by username
    Admin getByUsername(const std::string& username) {
        Admin admin = Admin::findByUsername(username);
        if (admin.getId() != 0) {
            admin = Admin(alertSystem);  // Recreate with alert system
            admin.setId(admin.getId());
            admin.setName(admin.getName());
            admin.setUsername(admin.getUsername());
            admin.setPassword(admin.getPassword());
        }
        return admin;
    }

    // Get all admins
    std::vector<Admin> getAll() {
        return Admin::findAll();
    }

    // Delete admin by ID
    bool deleteAdmin(int id) {
        return Admin::remove(id);
    }

    // Alert system management
    void configureAlertSystem(const std::string& type, bool enabled) {
        Admin admin(alertSystem);
        admin.configureAlertSystem(type, enabled);
    }

    void setAlertThreshold(const std::string& type, int threshold) {
        Admin admin(alertSystem);
        admin.setAlertThreshold(type, threshold);
    }

    Poco::JSON::Object getSecurityLogs() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            std::vector<std::string> logs;
            select << "SELECT * FROM security_logs ORDER BY timestamp DESC LIMIT 100",
                into(logs),
                now;
            
            Poco::JSON::Array logsArray;
            for (const auto& log : logs) {
                Poco::JSON::Object logObj;
                logObj.set("id", log);
                logsArray.add(logObj);
            }
            result.set("logs", logsArray);
        } catch (const std::exception& e) {
            std::cerr << "Error getting security logs: " << e.what() << std::endl;
        }
        
        return result;
    }

    Poco::JSON::Object getSecurityStatus() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            // Get active sessions
            Poco::Data::Statement sessions(session);
            int activeSessions;
            sessions << "SELECT COUNT(*) FROM active_sessions",
                into(activeSessions),
                now;
            result.set("active_sessions", activeSessions);
            
            // Get failed login attempts
            Poco::Data::Statement failedLogins(session);
            int failedLoginCount;
            failedLogins << "SELECT COUNT(*) FROM security_logs WHERE event_type = 'failed_login' AND timestamp > DATE_SUB(NOW(), INTERVAL 1 HOUR)",
                into(failedLoginCount),
                now;
            result.set("failed_logins_last_hour", failedLoginCount);
            
            // Get security alerts
            Poco::Data::Statement alerts(session);
            int activeAlerts;
            alerts << "SELECT COUNT(*) FROM security_alerts WHERE status = 'active'",
                into(activeAlerts),
                now;
            result.set("active_alerts", activeAlerts);
        } catch (const std::exception& e) {
            std::cerr << "Error getting security status: " << e.what() << std::endl;
        }
        
        return result;
    }

    bool updateSecuritySettings(Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            bool twoFactorEnabled = json->getValue<bool>("two_factor_enabled");
            int maxLoginAttempts = json->getValue<int>("max_login_attempts");
            int sessionTimeout = json->getValue<int>("session_timeout");
            bool ipRestriction = json->getValue<bool>("ip_restriction");
            
            Poco::Data::Statement update(session);
            update << "UPDATE security_settings SET "
                   "two_factor_enabled = ?, "
                   "max_login_attempts = ?, "
                   "session_timeout = ?, "
                   "ip_restriction = ?, "
                   "updated_at = NOW()",
                use(twoFactorEnabled),
                use(maxLoginAttempts),
                use(sessionTimeout),
                use(ipRestriction),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating security settings: " << e.what() << std::endl;
            return false;
        }
    }

    Poco::JSON::Object getPendingVerifications() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            std::vector<std::string> verifications;
            select << "SELECT * FROM account_verifications WHERE status = 'pending'",
                into(verifications),
                now;
            
            Poco::JSON::Array verificationsArray;
            for (const auto& ver : verifications) {
                Poco::JSON::Object verification;
                verification.set("id", ver);
                verificationsArray.add(verification);
            }
            result.set("verifications", verificationsArray);
        } catch (const std::exception& e) {
            std::cerr << "Error getting pending verifications: " << e.what() << std::endl;
        }
        
        return result;
    }

    bool verifyAccount(int verificationId, bool approved, const std::string& notes) {
        auto session = dbManager->getSession();
        
        try {
            Poco::Data::Statement update(session);
            std::string status = approved ? "approved" : "rejected";
            std::string notesCopy = notes;
            update << "UPDATE account_verifications SET "
                   "status = ?, "
                   "notes = ?, "
                   "verified_at = NOW() "
                   "WHERE id = ?",
                use(status),
                use(notesCopy),
                use(verificationId),
                now;
            
            if (approved) {
                // Get the user type and ID
                Poco::Data::Statement select(session);
                std::string userType;
                int userId;
                select << "SELECT user_type, user_id FROM account_verifications WHERE id = ?",
                    into(userType),
                    into(userId),
                    use(verificationId),
                    now;
                
                // Update the user's verification status
                std::string tableName = userType + "s";
                Poco::Data::Statement updateUser(session);
                updateUser << "UPDATE " + tableName + " SET verified = true WHERE id = ?",
                    use(userId),
                    now;
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error verifying account: " << e.what() << std::endl;
            return false;
        }
    }
};