import streamlit as st
import pandas as pd
import plotly.express as px
from api import (
    api_get_all_help_requests,
    api_trigger_emergency_protocol,
    api_track_relief_effort,
    api_allocate_personnel,
    api_create_emergency_budget,
    api_call_military,
    api_get_personnel_status,
    api_get_budget_status,
    api_get_military_status,
    api_get_emergency_level
)

def severity_report():
    """Display severity analysis and reporting interface."""
    st.header("Severity Report")
    
    # Get all help requests for analysis
    help_requests = api_get_all_help_requests()
    
    if not help_requests:
        st.info("No data available for analysis.")
        return
    
    # Convert to DataFrame for analysis
    df = pd.DataFrame(help_requests)
    
    # Severity by location
    st.subheader("Severity by Location")
    location_severity = df.groupby('location')['urgency'].mean().reset_index()
    location_severity = location_severity.sort_values('urgency', ascending=False)
    
    fig = px.bar(
        location_severity,
        x='location',
        y='urgency',
        title='Average Urgency Level by Location',
        labels={'urgency': 'Average Urgency', 'location': 'Location'}
    )
    st.plotly_chart(fig)
    
    # Severity by request type
    st.subheader("Severity by Request Type")
    type_severity = df.groupby('type')['urgency'].mean().reset_index()
    type_severity = type_severity.sort_values('urgency', ascending=False)
    
    fig = px.pie(
        type_severity,
        values='urgency',
        names='type',
        title='Urgency Distribution by Request Type'
    )
    st.plotly_chart(fig)
    
    # Status distribution
    st.subheader("Request Status Distribution")
    status_counts = df['status'].value_counts().reset_index()
    status_counts.columns = ['status', 'count']
    
    fig = px.pie(
        status_counts,
        values='count',
        names='status',
        title='Distribution of Request Statuses'
    )
    st.plotly_chart(fig)
    
    # Detailed analysis
    st.subheader("Detailed Analysis")
    with st.expander("View Detailed Statistics"):
        st.write("### Request Statistics")
        st.write(f"Total Requests: {len(df)}")
        st.write(f"Average Urgency: {df['urgency'].mean():.2f}")
        st.write(f"Highest Urgency: {df['urgency'].max()}")
        st.write(f"Lowest Urgency: {df['urgency'].min()}")
        
        st.write("### Status Breakdown")
        status_breakdown = df['status'].value_counts()
        st.write(status_breakdown)

def resource_allocation():
    """Display resource allocation interface."""
    st.header("Resource Allocation")
    
    # Get all help requests for resource allocation
    help_requests = api_get_all_help_requests()
    
    if not help_requests:
        st.info("No active requests for resource allocation.")
        return
    
    # Filter for pending and in-progress requests
    active_requests = [req for req in help_requests if req.get("status") in ["Pending", "In Progress"]]
    
    # Group requests by type
    request_types = {}
    for req in active_requests:
        req_type = req.get("type")
        if req_type not in request_types:
            request_types[req_type] = []
        request_types[req_type].append(req)
    
    # Display resource allocation interface
    st.subheader("Resource Allocation by Request Type")
    
    for req_type, requests in request_types.items():
        with st.expander(f"{req_type} Requests ({len(requests)})"):
            # Sort by urgency
            sorted_requests = sorted(requests, key=lambda x: x.get("urgency", 0), reverse=True)
            
            for req in sorted_requests:
                urgency = req.get("urgency", 0)
                urgency_color = {
                    5: "red",
                    4: "orange",
                    3: "yellow",
                    2: "lightgreen",
                    1: "green"
                }.get(urgency, "gray")
                
                st.markdown(f"""
                <div style="background-color: {urgency_color}15; padding: 15px; border-radius: 5px; margin-bottom: 10px; border-left: 5px solid {urgency_color};">
                    <h4>Urgency Level: {urgency}/5</h4>
                    <p><strong>Location:</strong> {req.get('location', 'Unknown')}</p>
                    <p><strong>Status:</strong> {req.get('status', 'Unknown')}</p>
                    <p>{req.get('description', 'No description')}</p>
                </div>
                """, unsafe_allow_html=True)
                
                # Resource allocation form
                with st.form(f"allocate_resources_{req.get('id')}"):
                    st.write("### Allocate Resources")
                    resources = st.multiselect(
                        "Select Resources",
                        ["Medical Team", "Food Supplies", "Shelter", "Transportation", "Search & Rescue"],
                        key=f"resources_{req.get('id')}"
                    )
                    priority = st.slider("Priority Level", 1, 5, 3, key=f"priority_{req.get('id')}")
                    
                    if st.form_submit_button("Allocate Resources"):
                        st.success(f"Resources allocated for request {req.get('id')}")

def emergency_protocol():
    """Display emergency protocol interface."""
    st.header("Emergency Protocol")
    
    # Get current emergency level
    emergency_level = api_get_emergency_level()
    
    # Display current emergency status
    st.subheader("Current Emergency Status")
    col1, col2 = st.columns(2)
    with col1:
        st.metric("Emergency Level", emergency_level.get("level", "Normal").upper())
    with col2:
        st.metric("Last Updated", emergency_level.get("last_updated", "N/A"))
    
    # Emergency protocol form
    with st.form("emergency_protocol"):
        st.subheader("Trigger Emergency Protocol")
        
        protocol_type = st.selectbox("Protocol Type", [
            "Natural Disaster",
            "Civil Unrest",
            "Public Health",
            "Infrastructure Failure",
            "Other"
        ])
        
        severity = st.slider("Severity Level", 1, 5, 3)
        location = st.text_input("Affected Location")
        description = st.text_area("Description")
        
        if st.form_submit_button("Trigger Protocol"):
            data = {
                "type": protocol_type,
                "severity": severity,
                "location": location,
                "description": description
            }
            
            success = api_trigger_emergency_protocol(data)
            if success:
                st.success("Emergency protocol triggered successfully!")
            else:
                st.error("Failed to trigger emergency protocol")

def relief_effort_tracking():
    """Display relief effort tracking interface."""
    st.header("Relief Effort Tracking")
    
    # Get relief effort data
    relief_data = api_track_relief_effort()
    
    if not relief_data:
        st.info("No relief effort data available")
        return
    
    # Display metrics
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Active Relief Operations", relief_data.get("active_operations", 0))
    with col2:
        st.metric("Resources Deployed", relief_data.get("resources_deployed", 0))
    with col3:
        st.metric("Personnel Deployed", relief_data.get("personnel_deployed", 0))
    
    # Display relief operations
    st.subheader("Active Relief Operations")
    operations = relief_data.get("operations", [])
    for op in operations:
        with st.expander(f"Operation: {op.get('name', 'Unnamed')}"):
            st.write(f"**Location:** {op.get('location', 'Unknown')}")
            st.write(f"**Status:** {op.get('status', 'Unknown')}")
            st.write(f"**Resources:** {', '.join(op.get('resources', []))}")
            st.write(f"**Personnel:** {op.get('personnel_count', 0)}")

def personnel_management():
    """Display personnel management interface."""
    st.header("Personnel Management")
    
    # Get current personnel status
    personnel_status = api_get_personnel_status()
    
    # Display current allocation
    st.subheader("Current Personnel Allocation")
    if personnel_status:
        df = pd.DataFrame(personnel_status.get("allocations", []))
        if not df.empty:
            st.dataframe(df)
        else:
            st.info("No personnel currently allocated")
    else:
        st.info("No personnel data available")
    
    # Personnel allocation form
    with st.form("personnel_allocation"):
        st.subheader("Allocate Personnel")
        
        personnel_type = st.selectbox("Personnel Type", [
            "Medical Staff",
            "Security Forces",
            "Relief Workers",
            "Technical Support",
            "Administrative Staff"
        ])
        
        location = st.text_input("Deployment Location")
        count = st.number_input("Number of Personnel", min_value=1, max_value=1000, value=1)
        priority = st.slider("Priority Level", 1, 5, 3)
        
        if st.form_submit_button("Allocate Personnel"):
            data = {
                "type": personnel_type,
                "location": location,
                "count": count,
                "priority": priority
            }
            
            success = api_allocate_personnel(data)
            if success:
                st.success("Personnel allocated successfully!")
            else:
                st.error("Failed to allocate personnel")

def budget_management():
    """Display budget management interface."""
    st.header("Budget Management")
    
    # Get current budget status
    budget_status = api_get_budget_status()
    
    # Display current allocations
    st.subheader("Current Budget Allocations")
    if budget_status:
        df = pd.DataFrame(budget_status.get("allocations", []))
        if not df.empty:
            st.dataframe(df)
            
            # Display total budget
            total_budget = df["amount"].sum()
            st.metric("Total Budget Allocated", f"${total_budget:,.2f}")
        else:
            st.info("No budget currently allocated")
    else:
        st.info("No budget data available")
    
    # Budget allocation form
    with st.form("budget_allocation"):
        st.subheader("Allocate Budget")
        
        category = st.selectbox("Budget Category", [
            "Emergency Response",
            "Infrastructure",
            "Medical Supplies",
            "Food and Water",
            "Shelter",
            "Transportation",
            "Other"
        ])
        
        amount = st.number_input("Amount", min_value=0.0, step=1000.0)
        location = st.text_input("Allocation Location")
        priority = st.slider("Priority Level", 1, 5, 3)
        
        if st.form_submit_button("Allocate Budget"):
            data = {
                "category": category,
                "amount": amount,
                "location": location,
                "priority": priority
            }
            
            success = api_create_emergency_budget(data)
            if success:
                st.success("Budget allocated successfully!")
            else:
                st.error("Failed to allocate budget")

def military_support():
    """Display military support interface."""
    st.header("Military Support")
    
    # Get current military status
    military_status = api_get_military_status()
    
    # Display current deployments
    st.subheader("Current Military Deployments")
    if military_status:
        df = pd.DataFrame(military_status.get("deployments", []))
        if not df.empty:
            st.dataframe(df)
        else:
            st.info("No military units currently deployed")
    else:
        st.info("No military data available")
    
    # Military support request form
    with st.form("military_support"):
        st.subheader("Request Military Support")
        
        support_type = st.selectbox("Support Type", [
            "Search and Rescue",
            "Security",
            "Logistics",
            "Medical Support",
            "Infrastructure",
            "Other"
        ])
        
        location = st.text_input("Deployment Location")
        description = st.text_area("Description")
        priority = st.slider("Priority Level", 1, 5, 3)
        
        if st.form_submit_button("Request Support"):
            data = {
                "type": support_type,
                "location": location,
                "description": description,
                "priority": priority
            }
            
            success = api_call_military(data)
            if success:
                st.success("Military support requested successfully!")
            else:
                st.error("Failed to request military support")

def show_dashboard():
    """Main dashboard function for Government Agency."""
    if not st.session_state.logged_in or st.session_state.user_type != "government_agency":
        st.warning("Please log in as a government agency to access this dashboard.")
        return
    
    st.title("🏛️ Government Agency Dashboard")
    
    # Get selected menu item from session state
    selected = st.session_state.selected_menu
    
    if selected == "Emergency Protocol":
        emergency_protocol()
    elif selected == "Personnel Management":
        personnel_management()
    elif selected == "Budget Management":
        budget_management()
    elif selected == "Military Support":
        military_support()
    elif selected == "Logout":
        st.session_state.logged_in = False
        st.session_state.user_type = None
        st.session_state.user_id = None
        st.success("You have been logged out")
        st.experimental_rerun()
    else:
        # Default to emergency protocol if no menu item is selected
        emergency_protocol()
