import streamlit as st
from streamlit_option_menu import option_menu

def sidebar_menu():
    """Display the sidebar menu based on user type and login status."""
    with st.sidebar:
        st.title("Crisis Management")
        
        # Initialize selected variable
        selected = "Login"  # Default value
        
        if not st.session_state.logged_in:
            selected = option_menu(
                "Main Menu", 
                ["Login", "Register"], 
                icons=["key", "person-plus"], 
                menu_icon="list",
                default_index=0
            )
        else:
            if st.session_state.user_type == "admin":
                selected = option_menu(
                    "Admin Menu", 
                    ["Dashboard", "Manage Users", "Alert System", "System Security", "Account Verification", "Logout"], 
                    icons=["speedometer", "people", "bell", "shield-lock", "check-circle", "box-arrow-right"], 
                    menu_icon="list",
                    default_index=0
                )
            elif st.session_state.user_type == "people_in_crisis":
                selected = option_menu(
                    "Crisis Help", 
                    ["Request Help", "Track Request", "Update Profile", "Logout"], 
                    icons=["exclamation-triangle", "search", "person", "box-arrow-right"], 
                    menu_icon="list",
                    default_index=0
                )
            elif st.session_state.user_type == "volunteer":
                selected = option_menu(
                    "Volunteer", 
                    ["Available Tasks", "Donation Form", "History", "Update Profile", "Logout"], 
                    icons=["list-task", "cash", "clock-history", "person", "box-arrow-right"], 
                    menu_icon="list",
                    default_index=0
                )
            elif st.session_state.user_type == "relief_provider":
                selected = option_menu(
                    "Relief Provider", 
                    ["Dashboard", "Resource Management", "Update Profile", "Logout"], 
                    icons=["hospital", "box-seam", "person", "box-arrow-right"], 
                    menu_icon="list",
                    default_index=0
                )
            elif st.session_state.user_type == "government_agency":
                selected = option_menu(
                    "Government Agency", 
                    ["Emergency Protocol", "Personnel Management", "Budget Management", "Military Support", "Logout"], 
                    icons=["exclamation-triangle", "people", "cash", "shield", "box-arrow-right"], 
                    menu_icon="list",
                    default_index=0
                )
        
        return selected 