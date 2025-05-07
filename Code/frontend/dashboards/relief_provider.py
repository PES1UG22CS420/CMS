import streamlit as st
import pandas as pd
from api import (
    api_get_all_help_requests,
    api_update_help_request_status,
    api_get_profile,
    api_update_profile
)

def show_dashboard():
    """Main dashboard function for Relief Provider."""
    if not st.session_state.logged_in or st.session_state.user_type != "relief_provider":
        st.warning("Please log in as a relief provider to access this dashboard.")
        return
    
    st.title("🏥 Relief Provider Dashboard")
    
    # Get selected menu item from session state
    selected = st.session_state.selected_menu
    
    if selected == "Dashboard":
        relief_provider_dashboard()
    elif selected == "Resource Management":
        resource_management()
    elif selected == "Update Profile":
        update_profile()
    elif selected == "Logout":
        st.session_state.logged_in = False
        st.session_state.user_type = None
        st.session_state.user_id = None
        st.success("You have been logged out")
        st.experimental_rerun()
    else:
        # Default to main dashboard if no menu item is selected
        relief_provider_dashboard()

def relief_provider_dashboard():
    """Display main relief provider dashboard."""
    st.header("Resource Management Dashboard")
    
    # Get all help requests
    help_requests = api_get_all_help_requests()
    
    # Display key metrics
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Active Requests", len([r for r in help_requests if r.get("status") == "Pending"]))
    with col2:
        st.metric("Total Resources", len(help_requests))
    with col3:
        st.metric("Resources Deployed", len([r for r in help_requests if r.get("status") == "Resolved"]))
    
    # Display recent requests
    st.subheader("Recent Resource Requests")
    if not help_requests:
        st.info("No resource requests found.")
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
            <div class="resource-card">
                <small>{request.get("timestamp", "Unknown time")}</small>
                <h4>{request.get("type", "Unknown type")}</h4>
                <p>{request.get("description", "No description")}</p>
                <p>Location: {request.get("location", "Unknown location")}</p>
                <p>Status: <span style="color: {status_color};">{request.get("status", "Pending")}</span></p>
            </div>
            """, unsafe_allow_html=True)

def resource_management():
    """Display resource management interface."""
    st.header("Resource Management")
    
    # Get all help requests
    help_requests = api_get_all_help_requests()
    
    if not help_requests:
        st.info("No resource requests to manage.")
    else:
        for request in help_requests:
            col1, col2, col3 = st.columns([3, 2, 1])
            
            with col1:
                st.write(f"**{request.get('type', 'Unknown type')}**")
                st.write(f"Description: {request.get('description', 'No description')}")
                st.write(f"Location: {request.get('location', 'Unknown location')}")
            
            with col2:
                st.write(f"Status: {request.get('status', 'Pending')}")
                st.write(f"Urgency: {request.get('urgency', 3)}/5")
                st.write(f"Time: {request.get('timestamp', 'Unknown time')}")
            
            with col3:
                request_id = request.get("id")
                
                # Only show status change options for non-resolved/cancelled requests
                if request.get("status") not in ["Resolved", "Cancelled"]:
                    status = st.selectbox("Change Status", 
                                         ["Pending", "In Progress", "Resolved", "Cancelled"],
                                         index=["Pending", "In Progress", "Resolved", "Cancelled"].index(request.get("status", "Pending")),
                                         key=f"status_{request_id}")
                    
                    if st.button("Update", key=f"update_{request_id}"):
                        updated = api_update_help_request_status(request_id, status)
                        if updated:
                            st.success(f"Status updated to {status}")
                            st.experimental_rerun()
                        else:
                            st.error("Failed to update status")
            
            st.markdown("---")  # Using markdown horizontal rule instead of divider

def update_profile():
    """Display profile update interface."""
    st.header("Update Profile")
    
    # Get current profile
    profile = api_get_profile(st.session_state.user_id)
    
    # Default values if profile fetch fails
    default_profile = {
        "name": "",
        "orgType": "NGO",
        "location": "",
        "resources": []
    }
    
    # Use profile data if available, otherwise use defaults
    profile_data = profile if profile else default_profile
    
    with st.form("update_profile"):
        st.write("Organization Information")
        name = st.text_input("Organization Name", value=profile_data.get("name", ""))
        org_type = st.selectbox("Organization Type", 
                              ["NGO", "Private Company", "International Organization", "Local Charity"],
                              index=["NGO", "Private Company", "International Organization", "Local Charity"].index(profile_data.get("orgType", "NGO")))
        location = st.text_input("Headquarters Location", value=profile_data.get("location", ""))
        resources = st.multiselect("Available Resources", 
                                 ["Food", "Medicine", "Shelter", "Clothing", "Financial Aid"],
                                 default=profile_data.get("resources", []))
        
        if st.form_submit_button("Update Profile"):
            # Prepare profile data
            update_data = {
                "name": name,
                "orgType": org_type,
                "location": location,
                "resources": resources
            }
            
            # Update profile
            success = api_update_profile(st.session_state.user_id, update_data)
            if success:
                st.success("Profile updated successfully!")
            else:
                st.error("Failed to update profile") 