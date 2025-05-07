import streamlit as st
import pandas as pd
import plotly.express as px
from api import (
    api_get_all_help_requests,
    api_update_help_request_status,
    api_get_alert_config,
    api_update_alert_config,
    api_get_security_logs,
    api_update_security_settings,
    api_verify_account,
    api_get_pending_verifications,
    api_get_security_status,
    api_get_verification_history,
    api_get_verification_settings,
    api_update_verification_settings,
    api_get_all_users,
    api_update_user_status,
    api_get_all_alerts,
    api_create_alert
)

# Admin Dashboard
def admin_dashboard():
    """Display main admin dashboard."""
    st.header("Admin Dashboard")
    
    # Get all help requests
    help_requests = api_get_all_help_requests()
    
    # Display key metrics
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Total Requests", len(help_requests))
    with col2:
        st.metric("Active Requests", len([r for r in help_requests if r.get("status") == "Pending"]))
    with col3:
        st.metric("Resolved Requests", len([r for r in help_requests if r.get("status") == "Resolved"]))
    
    st.markdown("---")
    
    # Display recent requests
    st.subheader("Recent Requests")
    if not help_requests:
        st.info("No requests found.")
    else:
        # Sort by timestamp (newest first)
        sorted_requests = sorted(help_requests, key=lambda x: x.get("timestamp", ""), reverse=True)
        
        for request in sorted_requests[:5]:  # Show only the 5 most recent
            status_color = {
                "Pending": "orange",
                "In Progress": "blue",
                "Resolved": "green",
                "Cancelled": "red"
            }.get(request.get("status", "Pending"), "gray")
            
            st.markdown(f"""
            <div class="request-card">
                <small>{request.get("timestamp", "Unknown time")}</small>
                <h4>{request.get("type", "Unknown type")}</h4>
                <p>{request.get("description", "No description")}</p>
                <p>Location: {request.get("location", "Unknown location")}</p>
                <p>Status: <span style="color: {status_color};">{request.get("status", "Pending")}</span></p>
            </div>
            """, unsafe_allow_html=True)
            st.markdown("---")

def manage_users():
    """Display user management interface."""
    st.header("User Management")
    
    # Get all users
    users = api_get_all_users()
    
    if not users:
        st.info("No users found.")
        return
    
    # Display user table
    st.subheader("All Users")
    df = pd.DataFrame(users)
    st.dataframe(df)
    
    st.markdown("---")
    
    # User actions
    st.subheader("User Actions")
    user_id = st.selectbox("Select User", [u.get("id") for u in users])
    
    if user_id:
        user = next((u for u in users if u.get("id") == user_id), None)
        if user:
            st.write(f"Selected User: {user.get('name', 'Unknown')}")
            st.write(f"Type: {user.get('type', 'Unknown')}")
            st.write(f"Status: {user.get('status', 'Unknown')}")
            
            # User status update
            new_status = st.selectbox("Update Status", 
                                    ["Active", "Suspended", "Blocked"],
                                    index=["Active", "Suspended", "Blocked"].index(user.get("status", "Active")))
            
            if st.button("Update Status"):
                success = api_update_user_status(user_id, new_status)
                if success:
                    st.success(f"User status updated to {new_status}")
                    st.experimental_rerun()
                else:
                    st.error("Failed to update user status")

def alert_system():
    """Display alert system interface."""
    st.header("Alert System")
    
    # Get all alerts
    alerts = api_get_all_alerts()
    
    # Display active alerts
    st.subheader("Active Alerts")
    if not alerts:
        st.info("No active alerts.")
    else:
        for alert in alerts:
            severity_color = {
                "High": "red",
                "Medium": "orange",
                "Low": "yellow"
            }.get(alert.get("severity", "Low"), "gray")
            
            st.markdown(f"""
            <div class="alert-card">
                <h4 style="color: {severity_color};">{alert.get("title", "Untitled Alert")}</h4>
                <p>{alert.get("message", "No message")}</p>
                <small>Severity: {alert.get("severity", "Low")}</small>
                <br>
                <small>Time: {alert.get("timestamp", "Unknown time")}</small>
            </div>
            """, unsafe_allow_html=True)
            st.markdown("---")
    
    # Create new alert
    st.subheader("Create New Alert")
    with st.form("create_alert"):
        title = st.text_input("Alert Title")
        message = st.text_area("Alert Message")
        severity = st.selectbox("Severity", ["High", "Medium", "Low"])
        
        if st.form_submit_button("Create Alert"):
            if not title or not message:
                st.error("Please fill in all fields")
            else:
                success = api_create_alert({
                    "title": title,
                    "message": message,
                    "severity": severity
                })
                if success:
                    st.success("Alert created successfully!")
                    st.experimental_rerun()
                else:
                    st.error("Failed to create alert")

def system_security():
    """Display system security interface."""
    st.header("System Security")
    
    # Get security logs
    security_logs = api_get_security_logs()
    
    # Display security metrics
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Total Logins", len([log for log in security_logs if isinstance(log, dict) and log.get("type") == "login"]))
    with col2:
        st.metric("Failed Attempts", len([log for log in security_logs if isinstance(log, dict) and log.get("type") == "login_failed"]))
    with col3:
        st.metric("Security Alerts", len([log for log in security_logs if isinstance(log, dict) and log.get("type") == "security_alert"]))
    
    st.markdown("---")
    
    # Display recent security logs
    st.subheader("Recent Security Logs")
    if not security_logs:
        st.info("No security logs found.")
    else:
        # Sort by timestamp (newest first)
        sorted_logs = sorted(
            [log for log in security_logs if isinstance(log, dict)],
            key=lambda x: x.get("timestamp", ""),
            reverse=True
        )
        
        for log in sorted_logs[:10]:  # Show only the 10 most recent
            log_type = log.get("type", "unknown")
            log_color = {
                "login": "green",
                "login_failed": "red",
                "security_alert": "orange",
                "system_change": "blue"
            }.get(log_type, "gray")
            
            st.markdown(f"""
            <div class="log-card">
                <small style="color: {log_color};">{log_type.upper()}</small>
                <p>{log.get("message", "No message")}</p>
                <small>Time: {log.get("timestamp", "Unknown time")}</small>
                <br>
                <small>IP: {log.get("ip_address", "Unknown")}</small>
            </div>
            """, unsafe_allow_html=True)
            st.markdown("---")

def account_verification():
    """Display account verification interface."""
    st.header("Account Verification")
    
    # Get pending verifications
    verifications = api_get_pending_verifications()
    
    # Display pending verifications
    st.subheader("Pending Verifications")
    if not verifications:
        st.info("No pending verifications.")
    else:
        for verification in verifications:
            # Handle both string and dictionary responses
            if isinstance(verification, str):
                st.markdown(f"""
                <div class="verification-card">
                    <h4>Verification Request</h4>
                    <p>ID: {verification}</p>
                    <p>Status: Pending</p>
                </div>
                """, unsafe_allow_html=True)
            else:
                st.markdown(f"""
                <div class="verification-card">
                    <h4>{verification.get("user_name", "Unknown User")}</h4>
                    <p>Type: {verification.get("user_type", "Unknown")}</p>
                    <p>Document: {verification.get("document_type", "Unknown")}</p>
                    <p>Submitted: {verification.get("submitted_at", "Unknown time")}</p>
                </div>
                """, unsafe_allow_html=True)
            
            col1, col2 = st.columns(2)
            with col1:
                if st.button("Approve", key=f"approve_{verification if isinstance(verification, str) else verification.get('id')}"):
                    success = api_verify_account(
                        verification if isinstance(verification, str) else verification.get("id"),
                        "approved"
                    )
                    if success:
                        st.success("Account approved successfully!")
                        st.experimental_rerun()
                    else:
                        st.error("Failed to approve account")
            with col2:
                if st.button("Reject", key=f"reject_{verification if isinstance(verification, str) else verification.get('id')}"):
                    success = api_verify_account(
                        verification if isinstance(verification, str) else verification.get("id"),
                        "rejected"
                    )
                    if success:
                        st.success("Account rejected successfully!")
                        st.experimental_rerun()
                    else:
                        st.error("Failed to reject account")
            
            st.markdown("---")

def show_dashboard():
    """Main dashboard function for Admin."""
    if not st.session_state.logged_in or st.session_state.user_type != "admin":
        st.warning("Please log in as an admin to access this dashboard.")
        return
    
    st.title("👨‍💼 Admin Dashboard")
    
    # Get selected menu item from session state
    selected = st.session_state.selected_menu
    
    if selected == "Dashboard":
        admin_dashboard()
    elif selected == "Manage Users":
        manage_users()
    elif selected == "Alert System":
        alert_system()
    elif selected == "System Security":
        system_security()
    elif selected == "Account Verification":
        account_verification()
    elif selected == "Logout":
        st.session_state.logged_in = False
        st.session_state.user_type = None
        st.session_state.user_id = None
        st.success("You have been logged out")
        st.experimental_rerun()
    else:
        # Default to admin dashboard if no menu item is selected
        admin_dashboard()