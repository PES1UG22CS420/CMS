#!/bin/bash

# Build the backend
echo "Building backend..."
mkdir -p build
cd build
cmake ..
make

# Install frontend dependencies and run
echo "Setting up frontend..."
cd /home/pradhaan/Desktop/Projects/OOAD/Major/frontend
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
