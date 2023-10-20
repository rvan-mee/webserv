#!/usr/bin/env python

import os
import sys

# Define the directory where you want to store uploaded files
upload_dir = '/path/to/upload/directory'

def save_uploaded_file(file_name, file_data):
    # Create an absolute path for the uploaded file
    file_path = os.path.join(upload_dir, file_name)

    # Open the file in binary write mode
    with open(file_path, 'wb') as file:
        while True:
            chunk = file_data.read(8192)  # Read data in chunks of 8KB
            if not chunk:
                break
            file.write(chunk)

    return file_path


def main():
    # Initialize an empty string to store the request data
    request_data = b''

    # Read the request data in chunks
    while True:
        chunk = sys.stdin.buffer.read(8192)  # Read data in chunks of 8KB
        if not chunk:
            break
        request_data += chunk

    # Extract the file name and file data
    if "filename=" in request_data and "filedata=" in request_data:
        file_name = request_data.split(b"filename=")[1].split(b"&")[0].decode('utf-8')
        file_data = request_data.split(b"filedata=")[1]

        try:
            # Save the uploaded file
            file_path = save_uploaded_file(file_name, file_data)
            print("HTTP/1.1 200 OK")
            print("Content-type: text/html\n")
            print(f"File '{file_name}' successfully uploaded and saved at: {file_path}")
        except Exception as e:
            print("HTTP/1.1 500 Internal Server Error")
            print("Content-type: text/html\n")
            print("Error: Failed to save the uploaded file.")
    else:
        print("HTTP/1.1 400 Bad Request")
        print("Content-type: text/html\n")
        print("Error: Invalid request data format")

if __name__ == "__main__":
    main()


def save_uploaded_file(file_name, input_stream):
    # Create an absolute path for the uploaded file
    file_path = os.path.join(upload_dir, file_name)

    # Open the file in binary write mode
    with open(file_path, 'wb') as file:
        while True:
            chunk = input_stream.read(8192)  # Read data in chunks of 8KB
            if not chunk:
                break
            file.write(chunk)

    return file_path

def main():
    # Retrieve the name of the uploaded file from the query string or form data
    query_string = os.environ.get("QUERY_STRING", "")
    file_name = query_string.split("=")[1] if query_string else "unknown"

    try:
        # Save the uploaded file
        file_path = save_uploaded_file(file_name, sys.stdin.buffer)
        print("Content-type: text/html\n")
        print(f"File '{file_name}' successfully uploaded and saved at: {file_path}")
    except Exception as e:
        print("Content-type: text/html\n")
        print("Error: Failed to save the uploaded file.")

if __name__ == "__main__":
    main()
