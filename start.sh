#!/bin/bash

# Apply database migrations.
python manage.py migrate        

# Start webpack server.
if [[ $DEBUG == *1* ]]; then
  cd frontend
  npm install
  npm run start &
  cd ..
fi

# Start server.
python manage.py runserver 0.0.0.0:8000
