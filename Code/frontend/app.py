import streamlit as st
import requests
import json
from datetime import datetime
from components.sidebar import sidebar_menu
from dashboards.peopleincrisis import show_dashboard as show_peopleincrisis_dashboard
from dashboards.volunteer import show_dashboard as show_volunteer_dashboard
from dashboards.admin import show_dashboard as show_admin_dashboard
from dashboards.agency import show_dashboard as show_agency_dashboard
from dashboards.relief_provider import show_dashboard as show_relief_provider_dashboard
from api import (
    api_login,
    api_register_person_in_crisis,
    api_register_volunteer,
    api_register_relief_provider,
    api_register_government_agency
)

# Login page
def login_page():
    st.header("Login")
    
    col1, col2 = st.columns([1, 1])
    
    with col1:
        st.subheader("User Login")
        user_type = st.selectbox("Select User Type", [
            "People in Crisis", 
            "Volunteer", 
            "Relief Provider", 
            "Government Agency", 
            "Admin"
        ])
        
        username = st.text_input("Username")
        password = st.text_input("Password", type="password")
        
        if st.button("Login"):
            if username and password:
                # Map user type to API value
                api_user_type = ""
                if user_type == "People in Crisis":
                    api_user_type = "people_in_crisis"
                elif user_type == "Volunteer":
                    api_user_type = "volunteer"
                elif user_type == "Relief Provider":
                    api_user_type = "relief_provider"
                elif user_type == "Government Agency":
                    api_user_type = "government_agency"
                elif user_type == "Admin":
                    api_user_type = "admin"
                
                # Call API to authenticate
                success, user_id = api_login(username, password, api_user_type)
                
                if success:
                    st.session_state.logged_in = True
                    st.session_state.user_type = api_user_type
                    st.session_state.user_id = user_id
                    st.success(f"Logged in as {user_type}")
                    st.experimental_rerun()
                else:
                    st.error("Invalid username or password")
            else:
                st.error("Please enter username and password")
    
    with col2:
        st.image("https://www.coe.int/documents/21202288/62129062/crisis+management+banner.jpg/db9ea329-bf5b-93b3-9e0f-0781fda21abf?t=1585142132000", use_column_width=True)
        st.markdown("""
        ## Welcome to Crisis Management System
        
        This system connects people in crisis with volunteers and relief providers, 
        coordinated by government agencies and managed by administrators.
        """)

# Register page
def register_page():
    st.header("Register")
    
    user_type = st.selectbox("Register as", [
        "People in Crisis", 
        "Volunteer", 
        "Relief Provider", 
        "Government Agency"
    ])
    
    if user_type == "People in Crisis":
        with st.form("register_crisis"):
            name = st.text_input("Full Name")
            location = st.text_input("Location")
            phone = st.text_input("Phone Number")
            username = st.text_input("Username")
            password = st.text_input("Password", type="password")
            
            submitted = st.form_submit_button("Register")
            if submitted:
                if name and location and phone and username and password:
                    # Call API to register
                    success = api_register_person_in_crisis(name, location, phone, username, password)
                    
                    if success:
                        st.success("Registration successful! You can now login.")
                    else:
                        st.error("Registration failed. Username may already exist.")
                else:
                    st.error("Please fill in all fields")
    
    elif user_type == "Volunteer":
        with st.form("register_volunteer"):
            name = st.text_input("Full Name")
            location = st.text_input("Location")
            org_type = st.selectbox("Organization Type", ["Individual", "NGO", "Private Company", "International Organization", "Local Charity"])
            username = st.text_input("Username")
            password = st.text_input("Password", type="password")
            
            submitted = st.form_submit_button("Register")
            if submitted:
                if name and location and org_type and username and password:
                    # Call API to register volunteer
                    success = api_register_volunteer(name, location, org_type, username, password)
                    if success:
                        st.success("Registration successful! You can now login.")
                    else:
                        st.error("Registration failed. Username may already exist.")
                else:
                    st.error("Please fill in all fields")
    
    elif user_type == "Relief Provider":
        with st.form("register_provider"):
            name = st.text_input("Organization Name")
            org_type = st.selectbox("Organization Type", ["NGO", "Private Company", "International Organization", "Local Charity"])
            location = st.text_input("Headquarters Location")
            resources = st.multiselect("Available Resources", ["Food", "Medicine", "Shelter", "Clothing", "Financial Aid"])
            username = st.text_input("Username")
            password = st.text_input("Password", type="password")
            
            submitted = st.form_submit_button("Register")
            if submitted:
                if name and org_type and location and resources and username and password:
                    # Call API to register relief provider
                    success = api_register_relief_provider(name, org_type, location, resources, username, password)
                    if success:
                        st.success("Registration successful! You can now login.")
                    else:
                        st.error("Registration failed. Username may already exist.")
                else:
                    st.error("Please fill in all fields")
    
    elif user_type == "Government Agency":
        with st.form("register_agency"):
            name = st.text_input("Agency Name")
            jurisdiction = st.selectbox("Jurisdiction", ["Local", "State/Provincial", "National", "International"])
            location = st.text_input("Location")
            username = st.text_input("Username")
            password = st.text_input("Password", type="password")
            
            submitted = st.form_submit_button("Register")
            if submitted:
                if name and jurisdiction and location and username and password:
                    # Call API to register government agency
                    success = api_register_government_agency(name, jurisdiction, location, username, password)
                    if success:
                        st.success("Registration successful! You can now login.")
                    else:
                        st.error("Registration failed. Username may already exist.")
                else:
                    st.error("Please fill in all fields")

# Logout function
def logout():
    st.session_state.logged_in = False
    st.session_state.user_type = None
    st.session_state.user_id = None
    st.success("You have been logged out")
    st.experimental_rerun()

# Main app
def main():
    # Initialize session state variables if they don't exist
    if 'logged_in' not in st.session_state:
        st.session_state.logged_in = False
    if 'user_type' not in st.session_state:
        st.session_state.user_type = None
    if 'user_id' not in st.session_state:
        st.session_state.user_id = None
    if 'selected_menu' not in st.session_state:
        st.session_state.selected_menu = "Login"
    
    # Get menu selection
    selected = sidebar_menu()
    
    # Update selected menu in session state
    st.session_state.selected_menu = selected
    
    if not st.session_state.logged_in:
        if selected == "Login":
            login_page()
        elif selected == "Register":
            register_page()
    else:
        # Admin views
        if st.session_state.user_type == "admin":
            show_admin_dashboard()
            if selected == "Logout":
                logout()
        
        # People in Crisis views
        elif st.session_state.user_type == "people_in_crisis":
            show_peopleincrisis_dashboard()
            if selected == "Logout":
                logout()
        
        # Volunteer views
        elif st.session_state.user_type == "volunteer":
            show_volunteer_dashboard()
            if selected == "Logout":
                logout()
        
        # Relief Provider views
        elif st.session_state.user_type == "relief_provider":
            show_relief_provider_dashboard()
            if selected == "Logout":
                logout()
        
        # Government Agency views
        elif st.session_state.user_type == "government_agency":
            show_agency_dashboard()
            if selected == "Logout":
                logout()
        
        # Other user types would have their own views
        # For now, just logout if selected
        elif selected == "Logout":
            logout()

if __name__ == "__main__":
    main()