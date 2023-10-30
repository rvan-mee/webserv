#!/usr/bin/env python

import os
import time

while True:
    time.sleep(1)
import sys

# Define the directory where you want to store uploaded files
upload_dir = '/home/dkramer/Documents/WEBSERVfolder/webserv/uploads/'

import re

# def extract_content_from_http_request(http_request):
#     # Define a regular expression pattern to extract the boundary from the Content-Type header
#     content_type_pattern = re.compile(r'Content-Type: multipart/form-data; boundary=(\S+)')
#     content_type_match = content_type_pattern.search(http_request)

#     if content_type_match:
#         boundary = content_type_match.group(1)
#         # Define a regular expression pattern to extract content between the empty line and the boundary
#         pattern = re.compile(rb'\r\n\r\n(.*?)(?=\r\n--' + re.escape(boundary.encode('utf-8')) + rb')', re.DOTALL)
#         match = pattern.search(http_request.encode('utf-8'))
#         if match:
#             return match.group(1).decode('utf-8')

#     return None

# def extract_content_from_http_request(http_request):
#     # Define a regular expression pattern to extract the boundary from the Content-Type header
#     content_type_pattern = re.compile(r'Content-Type: multipart/form-data; boundary=(\S+)')
#     content_type_match = content_type_pattern.search(http_request)

#     if content_type_match:
#         boundary = content_type_match.group(1)
#         # Define a regular expression pattern to extract the content
#         pattern = re.compile(rb'--' + re.escape(boundary.encode('utf-8')) + rb'\r\nContent-Type: text/plain\r\n\r\n(.*?)\r\n--' + re.escape(boundary.encode('utf-8')) + rb'--', re.DOTALL)
#         match = pattern.search(http_request.encode('utf-8'))

#         if match:
#             return match.group(1).decode('utf-8')

#     return None
# def extract_content_from_http_request(http_request):
#     # Define a regular expression pattern to extract the content from the Content-Disposition field
#     pattern = re.compile(r'Content-Disposition: form-data; name="myFile"; filename=".*"\r\nContent-Type: text/plain\r\n\r\n(.*?)\r\n--', re.DOTALL)

#     # Search for the pattern in the HTTP request
#     match = pattern.search(http_request)

#     if match:
#         return match.group(1)
#     return None


def extract_content_from_http_request(http_request):
    # Define a regular expression pattern to extract the content between the boundaries
    # http_request = http_request.encode('utf-8')

    pattern = re.compile(rb'--\S+\r\n.*?\r\n\r\n(.*?)\r\n--\S+--', re.DOTALL)
    
    # Search for the pattern in the HTTP request
    match = pattern.search(http_request)

    if match:
        content = match.group(1)
        # Split the content by newlines and return the last part (the actual content)
        return content.split(b'\r\n')[-1]

    return None

    
# def extract_content_from_http_request(http_request):
#     # Define a regular expression pattern to extract the boundary from the Content-Type header
#     content_type_pattern = re.compile(r'Content-Type: multipart/form-data; boundary=(\S+)')
#     content_type_match = content_type_pattern.search(http_request)

#     if content_type_match:
#         boundary = content_type_match.group(1)
#         # Define a regular expression pattern to find the start of the content
#         start_pattern = rb'\r\n\r\n'
#         start_match = re.search(start_pattern, http_request.encode('utf-8'))

#         if start_match:
#             start_index = start_match.end()
#             content_start = http_request.encode('utf-8')[start_index:]
#             # Define a regular expression pattern to extract content after the boundary
#             end_pattern = rb'--' + re.escape(boundary.encode('utf-8')) + rb'--'
#             end_match = re.search(end_pattern, content_start)

#             if end_match:
#                 end_index = end_match.start()
#                 content = content_start[:end_index]
#                 return content.decode('utf-8')

#     return None

def save_uploaded_file(file_name, file_data):
    # Create an absolute path for the uploaded file
    file_path = os.path.join(upload_dir, file_name)

    # Open the file in binary write mode and write the data
    with open(file_path, 'wb') as file:
        file.write(file_data)

    return file_path


# def upload_file(file_name, file_data, upload_folder):
#     # Create an absolute path for the uploaded file
#     file_path = os.path.join(upload_folder, file_name)

#     # Open the file in binary write mode and write the data
#     with open(file_path, 'wb') as file:
#         file.write(file_data)

#     return file_path
def extract_filename_from_http_request(http_request):
    # Define a regular expression pattern to find the filename attribute within the Content-Disposition header
    pattern = re.compile(rb'Content-Disposition:.*?filename="([^"]+)"', re.DOTALL)

    # Search for the pattern in the HTTP request
    match = pattern.search(http_request)

    if match:
        return match.group(1).decode('utf-8')

    return None
def main():
    # Initialize an empty string to store the request data
    request_data = b''

    # Read the request data in chunks
    request_data = """
    POST /upload.py HTTP/1.1
    Host: localhost:8070
    Content-Disposition: form-data; name="myFile"; filename="hoi.txt"
    Content-Type: text/plain

    hoi
    """
    # while True:
    #     chunk = sys.stdin.buffer.read(8192)  # Read data in chunks of 8KB
    #     if not chunk:
    #         break
    #     request_data += chunk
    # print(request_data.decode('utf-8'))=
    # Extract the file name and file data
    # if "filename=" in request_data.decode('utf-8'):
        # file_name = request_data.split(b"filename=")[1].split(b"&")[0].decode('utf-8')
    # file_name = extract_filename_from_http_request(request_data)
    # print(file_name)
    # file_data = extract_content_from_http_request(request_data)
    # print(file_data)
    file_name = "hoi.txt"
    file_data = b"hoi"
#  request_data.split(b"\n")[1]

    try:
        # Save the uploaded file
        file_path = save_uploaded_file(file_name, file_data)
        character_count = len(f"File '{file_name}' successfully uploaded and saved at: {file_path}")
        print("HTTP/1.1 200 OK")
        # Set the content type to HTML
        print("Content-type: text/html")
        # Get environment variables
        print(f"Content-Length: {character_count + 1}\n\n")
        print(f"File '{file_name}' successfully uploaded and saved at: {file_path}")
    except Exception as e:
        print(e)
        print("HTTP/1.1 500 Internal Server Error")
        print("Content-type: text/html\n")
        print("Error: Failed to save the uploaded file.")
    # else:
        # print("HTTP/1.1 400 Bad Request")
        # print("Content-type: text/html\n")
        # print("Error: Invalid request data format")

if __name__ == "__main__":
    main()
