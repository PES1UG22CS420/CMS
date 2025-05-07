import streamlit as st
import requests
import json
from config import API_URL
from typing import Dict, List, Optional, Any

# Base URL for the API
API_BASE_URL = "http://localhost:8080/api"

# Base URL for API
BASE_URL = "http://localhost:8080"

def make_request(method: str, endpoint: str, data: Optional[Dict] = None) -> Dict:
    """Make an API request and return the response."""
    url = f"{BASE_URL}{endpoint}"
    headers = {"Content-Type": "application/json"}
    
    try:
        if method == "GET":
            response = requests.get(url, headers=headers)
        elif method == "POST":
            response = requests.post(url, json=data, headers=headers)
        elif method == "PUT":
            response = requests.put(url, json=data, headers=headers)
        elif method == "DELETE":
            response = requests.delete(url, headers=headers)
        else:
            raise ValueError(f"Unsupported HTTP method: {method}")
        
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"API request failed: {e}")
        return {"status": "error", "message": str(e)}

# API functions
def api_login(username: str, password: str, user_type: str) -> tuple[bool, int]:
    """Authenticate user and return success status and user ID."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/auth/login",
            json={
                "username": username,
                "password": password,
                "userType": user_type
            }
        )
        
        if response.status_code == 200:
            data = response.json()
            if data.get("status") == "success":
                return True, data.get("userId", 0)
        return False, 0
    except Exception as e:
        st.error(f"Login error: {str(e)}")
        return False, 0

def api_register_person_in_crisis(name: str, location: str, phone: str, username: str, password: str) -> bool:
    """Register a new person in crisis."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/people-in-crisis/signup",
            json={
                "name": name,
                "location": location,
                "phoneNo": phone,
                "username": username,
                "password": password
            }
        )
        return response.status_code == 200 and response.json().get("status") == "success"
    except Exception as e:
        st.error(f"Registration error: {str(e)}")
        return False

def api_create_help_request(user_id: int, request_type: str, description: str, location: str, urgency: int) -> bool:
    """Create a new help request."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/help-requests",
            json={
                "requesterId": user_id,
                "type": request_type,
                "description": description,
                "location": location,
                "urgency": urgency
            }
        )
        return response.status_code == 200 and response.json().get("status") == "success"
    except Exception as e:
        st.error(f"Create help request error: {str(e)}")
        return False

def api_get_help_requests(user_id: int) -> list:
    """Get all help requests for a user."""
    try:
        response = requests.get(f"{API_BASE_URL}/help-requests/user/{user_id}")
        if response.status_code == 200:
            return response.json()
        return []
    except Exception as e:
        st.error(f"Get help requests error: {str(e)}")
        return []

def api_update_help_request_status(request_id: int, status: str) -> bool:
    """Update the status of a help request."""
    try:
        response = requests.put(
            f"{API_BASE_URL}/help-requests/{request_id}",
            json={"status": status}
        )
        return response.status_code == 200 and response.json().get("status") == "success"
    except Exception as e:
        st.error(f"Update help request status error: {str(e)}")
        return False

def api_register_volunteer(name: str, location: str, org_type: str, username: str, password: str) -> bool:
    """Register a new volunteer."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/volunteers/signup",
            json={
                "name": name,
                "location": location,
                "orgType": org_type,
                "username": username,
                "password": password
            }
        )
        if response.status_code == 200:
            data = response.json()
            return data.get("status") == "success"
        return False
    except Exception as e:
        print(f"Volunteer registration error: {str(e)}")
        return False

def api_register_relief_provider(name: str, org_type: str, location: str, resources: list, username: str, password: str) -> bool:
    """Register a new relief provider."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/relief-providers/signup",
            json={
                "name": name,
                "orgType": org_type,
                "location": location,
                "resources": resources,
                "username": username,
                "password": password
            }
        )
        return response.status_code == 200 and response.json().get("status") == "success"
    except Exception as e:
        st.error(f"Relief provider registration error: {str(e)}")
        return False

def api_register_government_agency(name: str, jurisdiction: str, location: str, username: str, password: str) -> bool:
    """Register a new government agency."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/government-agencies/signup",
            json={
                "name": name,
                "jurisdiction": jurisdiction,
                "location": location,
                "username": username,
                "password": password
            }
        )
        return response.status_code == 200 and response.json().get("status") == "success"
    except Exception as e:
        st.error(f"Government agency registration error: {str(e)}")
        return False

def api_get_all_help_requests() -> List[Dict]:
    """Get all help requests."""
    return make_request("GET", "/api/help-requests")

def api_get_person_in_crisis(user_id):
    """Get person in crisis by ID."""
    try:
        response = requests.get(f"{API_BASE_URL}/people-in-crisis/{user_id}")
        if response.status_code == 200:
            return response.json()
        return None
    except Exception as e:
        st.error(f"Error getting person in crisis: {str(e)}")
        return None
    
def api_login_volunteer(user_id):
    """
    Authenticates a volunteer by user ID.
    """
    response = requests.post(f"{API_URL}/volunteer/login", json={"userID": user_id})
    return response.json()

def api_donate(volunteer_id: int, amount: float) -> bool:
    """Process a donation."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/volunteers/donate/{volunteer_id}",
            json={"amount": float(amount)}
        )
        if response.status_code == 200:
            data = response.json()
            return data.get("status") == "success"
        print(f"Failed to process donation: {response.text}")
        return False
    except Exception as e:
        print(f"Error processing donation: {str(e)}")
        return False

def api_help_on_site(volunteer_id: int, task: str) -> bool:
    """Volunteer helps on site with a specified task."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/volunteers/help/{volunteer_id}",
            json={"task": task}
        )
        if response.status_code == 200:
            data = response.json()
            return data.get("status") == "success"
        print(f"Failed to register help request: {response.text}")
        return False
    except Exception as e:
        print(f"Error registering help request: {str(e)}")
        return False

def api_get_volunteer_history(volunteer_id: int) -> list:
    """Get volunteer's help/donation history."""
    try:
        response = requests.get(f"{API_BASE_URL}/volunteers/history/{volunteer_id}")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get volunteer history: {response.text}")
        return []
    except Exception as e:
        print(f"Error getting volunteer history: {str(e)}")
        return []

def api_get_available_tasks() -> list:
    """Get list of available tasks."""
    try:
        response = requests.get(f"{API_BASE_URL}/volunteers/tasks")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get available tasks: {response.text}")
        return []
    except Exception as e:
        print(f"Error getting available tasks: {str(e)}")
        return []

def api_update_availability(volunteer_id: int, is_available: bool) -> bool:
    """Update volunteer availability."""
    try:
        response = requests.put(
            f"{API_BASE_URL}/volunteers/{volunteer_id}",
            json={"available": is_available}
        )
        if response.status_code == 200:
            data = response.json()
            return data.get("status") == "success"
        print(f"Failed to update availability: {response.text}")
        return False
    except Exception as e:
        print(f"Error updating availability: {str(e)}")
        return False

def api_get_task_status(volunteer_id: int) -> list:
    """Get status of tasks assigned to volunteer."""
    try:
        response = requests.get(f"{API_BASE_URL}/volunteers/tasks/{volunteer_id}")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get task status: {response.text}")
        return []
    except Exception as e:
        print(f"Error getting task status: {str(e)}")
        return []

def api_get_alert_config() -> Dict:
    """Get alert system configuration."""
    return make_request("GET", "/api/alerts/config")

def api_update_alert_config(config: Dict) -> bool:
    """Update alert system configuration."""
    response = make_request("PUT", "/api/alerts/config", config)
    return response.get("status") == "success"

# Profile Management
def api_get_profile(user_id: int) -> Dict:
    """Get user profile by ID."""
    return make_request("GET", f"/api/profiles/{user_id}")

def api_update_profile(user_id: int, profile_data: Dict) -> bool:
    """Update user profile."""
    response = make_request("PUT", f"/api/profiles/{user_id}", profile_data)
    return response.get("status") == "success"

# Emergency Management
def api_get_emergency_level() -> dict:
    """Get current emergency level."""
    try:
        response = requests.get(f"{API_BASE_URL}/emergency/level")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get emergency level: {response.text}")
        return {"level": "normal", "description": "Error retrieving emergency level"}
    except Exception as e:
        print(f"Error getting emergency level: {str(e)}")
        return {"level": "normal", "description": "Error retrieving emergency level"}

def api_trigger_emergency_protocol(data: dict) -> bool:
    """Trigger emergency protocol."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/emergency/protocol",
            json=data
        )
        if response.status_code == 200:
            return response.json().get("status") == "success"
        return False
    except Exception as e:
        print(f"Error triggering emergency protocol: {str(e)}")
        return False

def api_track_relief_effort() -> dict:
    """Track relief effort status."""
    try:
        response = requests.get(f"{API_BASE_URL}/emergency/relief-effort")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to track relief effort: {response.text}")
        return {}
    except Exception as e:
        print(f"Error tracking relief effort: {str(e)}")
        return {}

def api_allocate_personnel(data: dict) -> bool:
    """Allocate personnel for emergency response."""
    try:
        # Ensure required fields are present
        if not all(key in data for key in ["type", "location", "count"]):
            print("Missing required fields for personnel allocation")
            return False
            
        response = requests.post(
            f"{API_BASE_URL}/emergency/personnel",
            json={
                "type": data["type"],
                "location": data["location"],
                "count": int(data["count"]),
                "priority": data.get("priority", 1)
            }
        )
        if response.status_code == 200:
            result = response.json()
            return result.get("status") == "success"
        print(f"Failed to allocate personnel: {response.text}")
        return False
    except Exception as e:
        print(f"Error allocating personnel: {str(e)}")
        return False

def api_get_personnel_status() -> dict:
    """Get current personnel allocation status."""
    try:
        response = requests.get(f"{API_BASE_URL}/emergency/personnel")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get personnel status: {response.text}")
        return {}
    except Exception as e:
        print(f"Error getting personnel status: {str(e)}")
        return {}

def api_create_emergency_budget(data: dict) -> bool:
    """Create emergency budget allocation."""
    try:
        # Ensure required fields are present
        if not all(key in data for key in ["category", "amount", "location"]):
            print("Missing required fields for budget allocation")
            return False
            
        response = requests.post(
            f"{API_BASE_URL}/emergency/budget",
            json={
                "category": data["category"],
                "amount": float(data["amount"]),
                "priority": data.get("priority", 1),
                "location": data["location"]
            }
        )
        if response.status_code == 200:
            result = response.json()
            return result.get("status") == "success"
        print(f"Failed to allocate budget: {response.text}")
        return False
    except Exception as e:
        print(f"Error allocating budget: {str(e)}")
        return False

def api_get_budget_status() -> dict:
    """Get current budget allocation status."""
    try:
        response = requests.get(f"{API_BASE_URL}/emergency/budget")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get budget status: {response.text}")
        return {}
    except Exception as e:
        print(f"Error getting budget status: {str(e)}")
        return {}

def api_call_military(data: dict) -> bool:
    """Request military support."""
    try:
        # Ensure required fields are present
        if not all(key in data for key in ["type", "location", "description"]):
            print("Missing required fields for military support request")
            return False
            
        response = requests.post(
            f"{API_BASE_URL}/emergency/military",
            json={
                "type": data["type"],
                "location": data["location"],
                "description": data["description"],
                "priority": data.get("priority", 1)
            }
        )
        if response.status_code == 200:
            result = response.json()
            return result.get("status") == "success"
        print(f"Failed to request military support: {response.text}")
        return False
    except Exception as e:
        print(f"Error requesting military support: {str(e)}")
        return False

def api_get_military_status() -> dict:
    """Get current military support status."""
    try:
        response = requests.get(f"{API_BASE_URL}/emergency/military")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get military status: {response.text}")
        return {}
    except Exception as e:
        print(f"Error getting military status: {str(e)}")
        return {}

def api_get_security_logs() -> list:
    """Get security logs."""
    try:
        response = requests.get(f"{API_BASE_URL}/security/logs")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get security logs: {response.text}")
        return []
    except Exception as e:
        print(f"Error getting security logs: {str(e)}")
        return []

def api_update_security_settings(settings: dict) -> bool:
    """Update security settings."""
    try:
        response = requests.put(
            f"{API_BASE_URL}/security/settings",
            json=settings
        )
        if response.status_code == 200:
            result = response.json()
            return result.get("status") == "success"
        print(f"Failed to update security settings: {response.text}")
        return False
    except Exception as e:
        print(f"Error updating security settings: {str(e)}")
        return False

def api_get_security_status() -> dict:
    """Get current security status."""
    try:
        response = requests.get(f"{API_BASE_URL}/security/status")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get security status: {response.text}")
        return {
            "status": "error",
            "message": "Failed to get security status",
            "last_updated": None,
            "alerts": []
        }
    except Exception as e:
        print(f"Error getting security status: {str(e)}")
        return {
            "status": "error",
            "message": str(e),
            "last_updated": None,
            "alerts": []
        }

def api_get_pending_verifications() -> list:
    """Get pending account verifications."""
    try:
        response = requests.get(f"{API_BASE_URL}/security/verifications")
        if response.status_code == 200:
            return response.json()
        print(f"Failed to get pending verifications: {response.text}")
        return []
    except Exception as e:
        print(f"Error getting pending verifications: {str(e)}")
        return []

def api_verify_account(account_id: int, status: str) -> bool:
    """Verify an account."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/security/verify/{account_id}",
            json={"status": status}
        )
        if response.status_code == 200:
            result = response.json()
            return result.get("status") == "success"
        print(f"Failed to verify account: {response.text}")
        return False
    except Exception as e:
        print(f"Error verifying account: {str(e)}")
        return False

def api_get_verification_history(filters=None):
    """Get verification history with optional filters."""
    try:
        response = requests.get(f"{API_BASE_URL}/verification/history", params=filters)
        if response.status_code == 200:
            return response.json()
        return []
    except Exception as e:
        print(f"Error getting verification history: {e}")
        return []

def api_update_verification_settings(settings):
    """Update verification settings."""
    try:
        response = requests.post(f"{API_BASE_URL}/verification/settings", json=settings)
        return response.status_code == 200
    except Exception as e:
        print(f"Error updating verification settings: {e}")
        return False

def api_request_verification_info(verification_id, message):
    """Request additional information for a verification."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/verification/{verification_id}/request-info",
            json={"message": message}
        )
        return response.status_code == 200
    except Exception as e:
        print(f"Error requesting verification info: {e}")
        return False

def api_get_verification_settings():
    """Get current verification settings."""
    try:
        response = requests.get(f"{API_BASE_URL}/verification/settings")
        if response.status_code == 200:
            return response.json()
        return {
            "require_id": True,
            "require_address": True,
            "require_org": True,
            "auto_verify": False,
            "auto_verify_types": ["Volunteer"],
            "notify_admin": True,
            "notify_user": True
        }
    except Exception as e:
        print(f"Error getting verification settings: {e}")
        return {
            "require_id": True,
            "require_address": True,
            "require_org": True,
            "auto_verify": False,
            "auto_verify_types": ["Volunteer"],
            "notify_admin": True,
            "notify_user": True
        }

def api_get_all_users():
    """Get all users from the system."""
    try:
        response = requests.get(f"{API_BASE_URL}/users")
        if response.status_code == 200:
            return response.json()
        return []
    except Exception as e:
        print(f"Error getting users: {e}")
        return []

def api_update_user_status(user_id, status):
    """Update user status."""
    try:
        response = requests.put(
            f"{API_BASE_URL}/users/{user_id}/status",
            json={"status": status}
        )
        return response.status_code == 200
    except Exception as e:
        print(f"Error updating user status: {e}")
        return False

def api_get_all_alerts():
    """Get all system alerts."""
    try:
        response = requests.get(f"{API_BASE_URL}/alerts")
        if response.status_code == 200:
            return response.json()
        return []
    except Exception as e:
        print(f"Error getting alerts: {e}")
        return []

def api_create_alert(alert_data):
    """Create a new system alert."""
    try:
        response = requests.post(
            f"{API_BASE_URL}/alerts",
            json=alert_data
        )
        return response.status_code == 201
    except Exception as e:
        print(f"Error creating alert: {e}")
        return False 