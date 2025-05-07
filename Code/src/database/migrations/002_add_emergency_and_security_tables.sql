-- Emergency Protocol Tables
CREATE TABLE IF NOT EXISTS emergency_protocols (
    id INT PRIMARY KEY AUTO_INCREMENT,
    level VARCHAR(50) NOT NULL,
    description TEXT,
    triggered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    triggered_by INT,
    status VARCHAR(20) DEFAULT 'active',
    FOREIGN KEY (triggered_by) REFERENCES government_agencies(id)
);

CREATE TABLE IF NOT EXISTS relief_operations (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    location VARCHAR(100) NOT NULL,
    status VARCHAR(20) DEFAULT 'active',
    resources_deployed INT DEFAULT 0,
    personnel_deployed INT DEFAULT 0,
    started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ended_at TIMESTAMP NULL,
    protocol_id INT,
    FOREIGN KEY (protocol_id) REFERENCES emergency_protocols(id)
);

CREATE TABLE IF NOT EXISTS personnel_allocations (
    id INT PRIMARY KEY AUTO_INCREMENT,
    type VARCHAR(50) NOT NULL,
    location VARCHAR(100) NOT NULL,
    count INT NOT NULL,
    priority INT DEFAULT 1,
    status VARCHAR(20) DEFAULT 'pending',
    allocated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP NULL,
    operation_id INT,
    FOREIGN KEY (operation_id) REFERENCES relief_operations(id)
);

CREATE TABLE IF NOT EXISTS emergency_budgets (
    id INT PRIMARY KEY AUTO_INCREMENT,
    category VARCHAR(50) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    priority INT DEFAULT 1,
    status VARCHAR(20) DEFAULT 'available',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    allocated_at TIMESTAMP NULL,
    operation_id INT,
    FOREIGN KEY (operation_id) REFERENCES relief_operations(id)
);

CREATE TABLE IF NOT EXISTS military_support (
    id INT PRIMARY KEY AUTO_INCREMENT,
    type VARCHAR(50) NOT NULL,
    location VARCHAR(100) NOT NULL,
    priority INT DEFAULT 1,
    description TEXT,
    status VARCHAR(20) DEFAULT 'pending',
    requested_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    responded_at TIMESTAMP NULL,
    operation_id INT,
    FOREIGN KEY (operation_id) REFERENCES relief_operations(id)
);

-- Security Tables
CREATE TABLE IF NOT EXISTS security_logs (
    id INT PRIMARY KEY AUTO_INCREMENT,
    event_type VARCHAR(50) NOT NULL,
    description TEXT,
    ip_address VARCHAR(45),
    user_id INT,
    user_type VARCHAR(20),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS security_alerts (
    id INT PRIMARY KEY AUTO_INCREMENT,
    type VARCHAR(50) NOT NULL,
    description TEXT,
    severity VARCHAR(20) DEFAULT 'medium',
    status VARCHAR(20) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP NULL
);

CREATE TABLE IF NOT EXISTS security_settings (
    id INT PRIMARY KEY AUTO_INCREMENT,
    two_factor_enabled BOOLEAN DEFAULT false,
    max_login_attempts INT DEFAULT 5,
    session_timeout INT DEFAULT 3600,
    ip_restriction BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS active_sessions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    user_type VARCHAR(20) NOT NULL,
    session_token VARCHAR(255) NOT NULL,
    ip_address VARCHAR(45),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL,
    UNIQUE KEY unique_session (session_token)
);

CREATE TABLE IF NOT EXISTS account_verifications (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_type VARCHAR(20) NOT NULL,
    user_id INT NOT NULL,
    document_type VARCHAR(50) NOT NULL,
    document_number VARCHAR(100) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    verified_at TIMESTAMP NULL,
    notes TEXT,
    verified_by INT,
    FOREIGN KEY (verified_by) REFERENCES admins(id)
);

-- Add verification status to user tables
ALTER TABLE people_in_crisis ADD COLUMN verified BOOLEAN DEFAULT false;
ALTER TABLE volunteers ADD COLUMN verified BOOLEAN DEFAULT false;
ALTER TABLE relief_providers ADD COLUMN verified BOOLEAN DEFAULT false;
ALTER TABLE government_agencies ADD COLUMN verified BOOLEAN DEFAULT false;

-- Insert default security settings
INSERT INTO security_settings (two_factor_enabled, max_login_attempts, session_timeout, ip_restriction)
VALUES (false, 5, 3600, false); 