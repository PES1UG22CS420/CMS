import streamlit as st
from components.requests import request_help, track_request
from api import api_update_profile, api_get_profile

def update_profile():
    """Display profile management interface."""
    st.header("Update Profile")
    
    # Get current profile data
    profile = api_get_profile(st.session_state.user_id)
    
    if not profile:
        st.error("Failed to load profile data")
        return
    
    with st.form("update_profile"):
        name = st.text_input("Full Name", value=profile.get("name", ""))
        location = st.text_input("Location", value=profile.get("location", ""))
        phone = st.text_input("Phone Number", value=profile.get("phoneNo", ""))
        
        if st.form_submit_button("Update Profile"):
            success = api_update_profile(st.session_state.user_id, {
                "name": name,
                "location": location,
                "phone": phone
            })
            
            if success:
                st.success("Profile updated successfully!")
            else:
                st.error("Failed to update profile")

def show_dashboard():
    """Main dashboard function for People in Crisis."""
    if not st.session_state.logged_in or st.session_state.user_type != "people_in_crisis":
        st.warning("Please log in as a person in crisis to access this dashboard.")
        return
    
    # Get the selected menu item from session state
    selected = st.session_state.get("selected_menu", "Request Help")
    
    if selected == "Request Help":
        request_help()
    elif selected == "Track Request":
        track_request()
    elif selected == "Update Profile":
        update_profile() 