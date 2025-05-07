#pragma once

#include <string>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Object.h>
#include <memory>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

// Singleton Database Manager
class DatabaseManager {
private:
    static DatabaseManager* instance;
    std::unique_ptr<Session> session;
    
    // Private constructor for singleton
    DatabaseManager(){
        // Register SQLite connector
        Poco::Data::SQLite::Connector::registerConnector();
        
        // Create session
        session = std::make_unique<Session>("SQLite", "crisis_management.db");
        
        // Initialize database
        initDatabase();
    }
    
    // Initialize database tables
    void initDatabase() {
        // Create tables if they don't exist
        
        // PeopleInCrisis table
        *session << "CREATE TABLE IF NOT EXISTS people_in_crisis ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "name TEXT NOT NULL, "
                << "user_id INTEGER UNIQUE, "
                << "location TEXT, "
                << "phone_no TEXT, "
                << "description TEXT, "
                << "status TEXT DEFAULT 'Pending', "
                << "has_active_request INTEGER DEFAULT 0, "
                << "username TEXT UNIQUE, "
                << "password TEXT, "
                << "verified INTEGER DEFAULT 0"
                << ")", now;
        
        // Volunteer table
        *session << "CREATE TABLE IF NOT EXISTS volunteers ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "name TEXT NOT NULL, "
                << "user_id INTEGER UNIQUE, "
                << "location TEXT, "
                << "available INTEGER DEFAULT 1, "
                << "username TEXT UNIQUE, "
                << "password TEXT, "
                << "org_type TEXT, "
                << "verified INTEGER DEFAULT 0"
                << ")", now;
        
        // VolunteerHistory table
        *session << "CREATE TABLE IF NOT EXISTS volunteer_history ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "volunteer_id INTEGER, "
                << "request_id INTEGER, "
                << "date TEXT, "
                << "description TEXT, "
                << "hours INTEGER, "
                << "FOREIGN KEY (volunteer_id) REFERENCES volunteers (id)"
                << ")", now;
        
        // ReliefProvider table
        *session << "CREATE TABLE IF NOT EXISTS relief_providers ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "name TEXT NOT NULL, "
                << "org_type TEXT, "
                << "location TEXT, "
                << "username TEXT UNIQUE, "
                << "password TEXT, "
                << "verified INTEGER DEFAULT 0"
                << ")", now;
        
        // ProviderResources table
        *session << "CREATE TABLE IF NOT EXISTS provider_resources ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "resource_type TEXT, "
                << "quantity INTEGER DEFAULT 0, "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id), "
                << "UNIQUE(provider_id, resource_type)"
                << ")", now;
        
        // IncidentReports table
        *session << "CREATE TABLE IF NOT EXISTS incident_reports ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "description TEXT, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "status TEXT DEFAULT 'active', "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id)"
                << ")", now;
        
        // ManpowerRequests table
        *session << "CREATE TABLE IF NOT EXISTS manpower_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "status TEXT DEFAULT 'Pending', "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id)"
                << ")", now;
        
        // GovAidRequests table
        *session << "CREATE TABLE IF NOT EXISTS gov_aid_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "status TEXT DEFAULT 'Pending', "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id)"
                << ")", now;
        
        // AdditionalAidRequests table
        *session << "CREATE TABLE IF NOT EXISTS additional_aid_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "status TEXT DEFAULT 'Pending', "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id)"
                << ")", now;
        
        // MonetaryRequests table
        *session << "CREATE TABLE IF NOT EXISTS monetary_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "provider_id INTEGER, "
                << "status TEXT DEFAULT 'Pending', "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (provider_id) REFERENCES relief_providers (id)"
                << ")", now;
        
        // GovernmentAgency table
        *session << "CREATE TABLE IF NOT EXISTS government_agencies ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "agency_name TEXT NOT NULL, "
                << "severity_level INTEGER DEFAULT 0, "
                << "username TEXT UNIQUE, "
                << "password TEXT, "
                << "verified INTEGER DEFAULT 0"
                << ")", now;
        
        // Resource table
        *session << "CREATE TABLE IF NOT EXISTS resources ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "agency_id INTEGER, "
                << "resource_name TEXT, "
                << "quantity INTEGER DEFAULT 0, "
                << "FOREIGN KEY (agency_id) REFERENCES government_agencies (id)"
                << ")", now;
        
        // HelpRequest table
        *session << "CREATE TABLE IF NOT EXISTS help_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "requester_id INTEGER, "
                << "type TEXT, "
                << "description TEXT, "
                << "location TEXT, "
                << "urgency INTEGER, "
                << "status TEXT DEFAULT 'Pending', "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (requester_id) REFERENCES people_in_crisis (id)"
                << ")", now;
        
        // Task table
        *session << "CREATE TABLE IF NOT EXISTS tasks ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "type TEXT, "
                << "location TEXT, "
                << "description TEXT, "
                << "urgency TEXT, "
                << "time TEXT, "
                << "assigned_volunteer_id INTEGER, "
                << "status TEXT DEFAULT 'Available', "
                << "FOREIGN KEY (assigned_volunteer_id) REFERENCES volunteers (id)"
                << ")", now;
        
        // Admins table
        *session << "CREATE TABLE IF NOT EXISTS admins ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "name TEXT NOT NULL, "
                << "username TEXT UNIQUE NOT NULL, "
                << "password TEXT NOT NULL, "
                << "is_active INTEGER DEFAULT 1, "
                << "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
                << ")", now;
        
        // AlertSystem table
        *session << "CREATE TABLE IF NOT EXISTS alert_system ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "subscriber TEXT UNIQUE"
                << ")", now;
        
        // Donations table
        *session << "CREATE TABLE IF NOT EXISTS donations ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "volunteer_id INTEGER, "
                << "amount REAL NOT NULL, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "FOREIGN KEY (volunteer_id) REFERENCES volunteers (id)"
                << ")", now;
        
        // VolunteerHelpRequests table
        *session << "CREATE TABLE IF NOT EXISTS volunteer_help_requests ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "volunteer_id INTEGER, "
                << "description TEXT, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "status TEXT DEFAULT 'Pending', "
                << "FOREIGN KEY (volunteer_id) REFERENCES volunteers (id)"
                << ")", now;
        
        // VolunteerAssignments table
        *session << "CREATE TABLE IF NOT EXISTS volunteer_assignments ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "volunteer_id INTEGER, "
                << "request_id INTEGER, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "status TEXT DEFAULT 'Active', "
                << "FOREIGN KEY (volunteer_id) REFERENCES volunteers (id), "
                << "FOREIGN KEY (request_id) REFERENCES help_requests (id)"
                << ")", now;
        
        // Alerts table
        *session << "CREATE TABLE IF NOT EXISTS alerts ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "type TEXT, "
                << "message TEXT, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "sender TEXT"
                << ")", now;
        
        // Emergency Protocol Tables
        *session << "CREATE TABLE IF NOT EXISTS emergency_protocols ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "level TEXT NOT NULL, "
                << "description TEXT, "
                << "triggered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                << "triggered_by INTEGER, "
                << "status TEXT DEFAULT 'active', "
                << "FOREIGN KEY (triggered_by) REFERENCES government_agencies(id)"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS relief_operations ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "name TEXT NOT NULL, "
                << "location TEXT NOT NULL, "
                << "status TEXT DEFAULT 'active', "
                << "resources_deployed INTEGER DEFAULT 0, "
                << "personnel_deployed INTEGER DEFAULT 0, "
                << "started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                << "ended_at TIMESTAMP, "
                << "protocol_id INTEGER, "
                << "FOREIGN KEY (protocol_id) REFERENCES emergency_protocols(id)"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS personnel_allocations ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "type TEXT NOT NULL, "
                << "location TEXT NOT NULL, "
                << "count INTEGER NOT NULL, "
                << "priority INTEGER DEFAULT 1, "
                << "status TEXT DEFAULT 'pending', "
                << "allocated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                << "completed_at TIMESTAMP, "
                << "operation_id INTEGER, "
                << "FOREIGN KEY (operation_id) REFERENCES relief_operations(id)"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS emergency_budgets ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "category TEXT NOT NULL, "
                << "amount REAL NOT NULL, "
                << "priority INTEGER DEFAULT 1, "
                << "status TEXT DEFAULT 'available', "
                << "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                << "allocated_at TIMESTAMP, "
                << "operation_id INTEGER, "
                << "FOREIGN KEY (operation_id) REFERENCES relief_operations(id)"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS military_support ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "type TEXT NOT NULL, "
                << "location TEXT NOT NULL, "
                << "priority INTEGER DEFAULT 1, "
                << "description TEXT, "
                << "status TEXT DEFAULT 'pending', "
                << "requested_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                << "responded_at TIMESTAMP, "
                << "operation_id INTEGER, "
                << "FOREIGN KEY (operation_id) REFERENCES relief_operations(id)"
                << ")", now;
        
        // Security tables
        *session << "CREATE TABLE IF NOT EXISTS security_logs ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "event_type TEXT NOT NULL, "
                << "description TEXT, "
                << "timestamp TEXT DEFAULT CURRENT_TIMESTAMP"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS active_sessions ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "user_id INTEGER NOT NULL, "
                << "user_type TEXT NOT NULL, "
                << "session_token TEXT UNIQUE, "
                << "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "expires_at TEXT"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS security_alerts ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "type TEXT NOT NULL, "
                << "description TEXT, "
                << "severity TEXT, "
                << "status TEXT DEFAULT 'active', "
                << "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS security_settings ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "two_factor_enabled INTEGER DEFAULT 0, "
                << "max_login_attempts INTEGER DEFAULT 3, "
                << "session_timeout INTEGER DEFAULT 3600, "
                << "ip_restriction INTEGER DEFAULT 0, "
                << "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
                << ")", now;
        
        *session << "CREATE TABLE IF NOT EXISTS account_verifications ("
                << "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                << "user_type TEXT NOT NULL, "
                << "user_id INTEGER NOT NULL, "
                << "status TEXT DEFAULT 'pending', "
                << "notes TEXT, "
                << "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
                << "verified_at TEXT"
                << ")", now;
        
        // Insert default admin if not exists
        Poco::Int64 adminCount = 0;
        *session << "SELECT COUNT(*) FROM admins WHERE username = 'admin'", into(adminCount), now;
        
        if (adminCount == 0) {
            *session << "INSERT INTO admins (name, username, password, is_active) VALUES ('Administrator', 'admin', 'admin123', 1)", now;
        }
    }
    
public:
    // Get singleton instance
    static DatabaseManager* getInstance() {
        if (!instance) {
            instance = new DatabaseManager();
        }
        return instance;
    }
    
    // Get database session
    Session& getSession() {
        return *session;
    }
    
    // Destructor
    ~DatabaseManager() {
        Poco::Data::SQLite::Connector::unregisterConnector();
    }
};

// Initialize static member
DatabaseManager* DatabaseManager::instance = nullptr;