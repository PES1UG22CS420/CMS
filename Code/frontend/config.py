import streamlit as st

API_URL = "http://localhost:8080/api"

def set_page_config():
    st.set_page_config(
        page_title="Crisis Management System",
        page_icon="🚨",
        layout="wide",
        initial_sidebar_state="expanded"
    )
    inject_custom_css()

def inject_custom_css():
    st.markdown("""
    <style>
        .main {
            padding: 2rem;
            background-color: #f8f9fa;
        }
        .stButton button {
            width: 100%;
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 10px;
            text-align: center;
            font-size: 16px;
            margin: 4px 0;
            cursor: pointer;
        }
        .crisis-card {
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            margin-bottom: 20px;
        }
        .alert {
            background-color: #f8d7da;
            color: #721c24;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 10px;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 10px;
        }
    </style>
    """, unsafe_allow_html=True)

def init_session_state():
    st.session_state.setdefault("logged_in", False)
    st.session_state.setdefault("user_type", None)
    st.session_state.setdefault("user_id", None)
