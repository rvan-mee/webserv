#!/usr/bin/python3
import cgi
import os

# Set the path where uploaded files will be stored
upload_dir = "/uploads"

# Create an instance of FieldStorage to handle the form data
form = cgi.FieldStorage()

# Get the uploaded file
file_item = form['file_to_upload']

if file_item.filename:
    # File was uploaded
    filename = os.path.basename(file_item.filename)
    with open(os.path.join(upload_dir, filename), 'wb') as file:
        file.write(file_item.file.read())
    print("Content-Type: text/html")
    print()
    print(f"File '{filename}' was successfully uploaded.")
else:
    print("Content-Type: text/html")
    print()
    print("No file was uploaded.")
