import streamlit as st
from api import (
    api_create_help_request,
    api_get_help_requests,
    api_update_help_request_status
)

def request_help():
    """Display the help request form and handle submission."""
    st.header("Request Help")
    
    with st.form("help_request"):
        st.subheader("Submit Help Request")
        
        request_type = st.selectbox("Type of Help Needed", ["Food", "Shelter", "Medical", "Evacuation", "Other"])
        description = st.text_area("Describe your situation")
        location = st.text_input("Current Location")
        urgency = st.slider("Urgency Level", 1, 5, 3)
        
        if request_type == "Other":
            other_type = st.text_input("Please specify")
            if other_type:
                request_type = other_type
        
        submitted = st.form_submit_button("Submit Request")
        if submitted:
            if description and location:
                # Call API to create help request
                success = api_create_help_request(
                    st.session_state.user_id,
                    request_type,
                    description,
                    location,
                    urgency
                )
                
                if success:
                    st.success("Your help request has been submitted. Help is on the way!")
                else:
                    st.error("Failed to submit help request. Please try again.")
            else:
                st.error("Please fill in all required fields")
    
    with st.expander("How it works"):
        st.write("""
        1. Submit your request with accurate information
        2. Our system matches your needs with available volunteers and resources
        3. Relief providers and volunteers are dispatched to your location
        4. You'll receive updates on the status of your request
        """)

def track_request():
    """Display and manage help request status."""
    st.header("Track Your Request")
    
    # Get user's help requests from API
    help_requests = api_get_help_requests(st.session_state.user_id)
    
    if not help_requests:
        st.info("You don't have any active help requests. You can create one from the 'Request Help' page.")
        return
    
    # Sort by timestamp (newest first)
    sorted_requests = sorted(help_requests, key=lambda x: x.get("timestamp", ""), reverse=True)
    
    # Get the most recent request
    latest_request = sorted_requests[0] if sorted_requests else None
    
    if latest_request:
        request_status = latest_request.get("status", "Pending")
        
        st.subheader("Request Status")
        
        status_color = {
            "Pending": "orange",
            "In Progress": "blue",
            "Resolved": "green",
            "Cancelled": "red"
        }.get(request_status, "gray")
        
        st.markdown(f"""
        <div style="background-color: {status_color}15; padding: 20px; border-radius: 5px; margin-bottom: 20px; border-left: 5px solid {status_color};">
            <h3>Status: {request_status}</h3>
            <p>Last updated: {latest_request.get("timestamp", "Unknown")}</p>
        </div>
        """, unsafe_allow_html=True)
        
        col1, col2 = st.columns(2)
        
        with col1:
            st.subheader("Request Details")
            st.write(f"**Type**: {latest_request.get('type', 'Unknown')}")
            st.write(f"**Location**: {latest_request.get('location', 'Unknown')}")
            st.write(f"**Urgency**: {latest_request.get('urgency', 3)}/5")
            st.write(f"**Submitted**: {latest_request.get('timestamp', 'Unknown')}")
        
        with col2:
            st.subheader("Description")
            st.write(latest_request.get("description", "No description provided"))
        
        st.subheader("All Your Requests")
        
        for request in sorted_requests:
            status_color = {
                "Pending": "orange",
                "In Progress": "blue",
                "Resolved": "green",
                "Cancelled": "red"
            }.get(request.get("status", "Pending"), "gray")
            
            st.markdown(f"""
            <div style="padding: 10px; border-left: 2px solid {status_color}; margin-bottom: 10px;">
                <small>{request.get('timestamp', 'Unknown')}</small>
                <p><strong>{request.get('type', 'Unknown')}</strong> - {request.get('status', 'Pending')}</p>
                <p>{request.get('description', 'No description')[:100]}...</p>
            </div>
            """, unsafe_allow_html=True)
        
        if request_status == "Pending" or request_status == "In Progress":
            if st.button("Cancel Request"):
                success = api_update_help_request_status(latest_request.get("id"), "Cancelled")
                if success:
                    st.success("Your request has been cancelled.")
                    st.experimental_rerun()
                else:
                    st.error("Failed to cancel request. Please try again.")
    else:
        st.info("No help requests found.") 