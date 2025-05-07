import streamlit as st
import requests
from config import API_URL

from api import (
    api_register_volunteer,
    api_login_volunteer,
    api_donate,
    api_help_on_site,
    api_get_volunteer_history,
    api_get_available_tasks,
    api_update_availability,
    api_get_task_status
)

def show_dashboard():
    """Main dashboard function for Volunteer."""
    if not st.session_state.logged_in or st.session_state.user_type != "volunteer":
        st.warning("Please log in as a volunteer to access this dashboard.")
        return

    st.title("🚑 Volunteer Dashboard")
    volunteer_id = st.session_state.user_id

    # Create two columns for better layout
    col1, col2 = st.columns(2)

    with col1:
    # Section 1: Update Availability
    st.subheader("Update Availability")
    availability = st.radio("Are you currently available to help?", ("yes", "no"))
    if st.button("Update Availability"):
            is_available = availability == "yes"
            result = api_update_availability(volunteer_id, is_available)
            if result:
            st.success("Availability updated successfully.")
        else:
                st.error("Failed to update availability. Please try again.")

    # Section 2: Donate
    st.subheader("Make a Donation")
        amount = st.number_input("Enter amount to donate", min_value=1, value=1, step=1)
    if st.button("Donate"):
            result = api_donate(volunteer_id, float(amount))
            if result:
                st.success("Donation successful. Thank you for your contribution!")
        else:
                st.error("Donation failed. Please try again.")

    with col2:
    # Section 3: Help on Site
    st.subheader("Help on Site")
    tasks = api_get_available_tasks()
    if tasks and isinstance(tasks, list):
        task = st.selectbox("Select a task to help with", tasks)
        if st.button("Volunteer for Task"):
            result = api_help_on_site(volunteer_id, task)
                if result:
                st.success("Successfully volunteered for the task.")
            else:
                    st.error("Failed to volunteer for the task. Please try again.")
    else:
        st.info("No tasks currently available.")

    # Section 4: View Task Status
    st.subheader("Your Task Status")
    task_status_result = api_get_task_status(volunteer_id)
    if isinstance(task_status_result, list) and task_status_result:
        for task in task_status_result:
            st.markdown(f"""
            <div style="background-color: #f0f2f6; padding: 1rem; border-radius: 0.5rem; margin-bottom: 1rem;">
                <strong>Task:</strong> {task.get("task", "N/A")}<br>
                <strong>Status:</strong> {task.get("status", "N/A")}<br>
                <strong>Date:</strong> {task.get("date", "N/A")}
            </div>
            """, unsafe_allow_html=True)
    elif isinstance(task_status_result, dict) and task_status_result.get("message"):
        st.info(task_status_result["message"])
    else:
        st.info("No tasks assigned yet.")

    # Section 5: Contribution History
    st.subheader("Contribution History")
    history = api_get_volunteer_history(volunteer_id)
    if isinstance(history, list) and history:
        for entry in history:
            st.markdown(f"""
            <div style="background-color: #f0f2f6; padding: 1rem; border-radius: 0.5rem; margin-bottom: 1rem;">
                <strong>Type:</strong> {entry.get("type", "N/A")}<br>
                <strong>Detail:</strong> {entry.get("detail", "N/A")}<br>
                <strong>Date:</strong> {entry.get("date", "N/A")}
            </div>
            """, unsafe_allow_html=True)
    else:
        st.info("No contributions found yet.")

    # Add some spacing at the bottom
    st.markdown("<br><br>", unsafe_allow_html=True)
