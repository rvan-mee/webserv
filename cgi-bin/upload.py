#!/usr/bin/env python

import os
import sys

# Define the directory where you want to store uploaded files
upload_dir = '/path/to/upload/directory'

import re

def extract_content_from_http_request(http_request):
    # Define a regular expression pattern to extract the boundary from the Content-Type header
    content_type_pattern = re.compile(r'Content-Type: multipart/form-data; boundary=(\S+)')
    content_type_match = content_type_pattern.search(http_request)

    if content_type_match:
        boundary = content_type_match.group(1)
        # Define a regular expression pattern to extract content between the empty line and the boundary
        pattern = re.compile(rb'\r\n\r\n(.*?)(?=\r\n--' + re.escape(boundary.encode('utf-8')) + rb')', re.DOTALL)
        match = pattern.search(http_request.encode('utf-8'))

        if match:
            return match.group(1).decode('utf-8')

    return None

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
    # print(request_data.decode('utf-8'))
    # Extract the file name and file data
    if "filename=" in request_data.decode('utf-8') and "filedata=" in request_data.decode('utf-8'):
        file_name = request_data.split(b"filename=")[1].split(b"&")[0].decode('utf-8')
       
        file_data = extract_content_from_http_request(request_data.decode('utf-8'))
#  request_data.split(b"\n")[1]

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
