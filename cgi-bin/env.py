#!/usr/local/bin/python3
import os
print("HTTP/1.1 200 OK")

# Set the content type to HTML
print("Content-type: text/html\n\n")

# Get environment variables
env_variables = os.environ

# Generate an HTML response
print("<html>")
print("<head><title>Environment Variables</title></head>")
print("<body>")
print("<h1>Environment Variables:</h1>")
print("<ul>")
for key, value in env_variables.items():
    print(f"<li><b>{key}:</b> {value}</li>")
print("</ul>")
print("</body>")
print("</html>")
