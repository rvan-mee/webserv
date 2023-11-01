# #!/usr/local/bin/python3
import os

def count_characters_in_html(env_variables):
    # Define the HTML content as a string
    html_content = """
<html>
<head><title>Environment Variables</title></head>
<body>
<h1>Environment Variables:</h1>
<ul>
"""

    # Iterate through the 'env_variables' and add the content to the string
    for key, value in env_variables.items():
        html_content += f"<li><b>{key}:</b> {value}</li>"

    # Complete the HTML content
    html_content += """
</ul>
</body>
</html>
"""

    # Calculate the number of characters in the HTML content
    character_count = len(html_content)
    return character_count

if __name__ == "__main__":
    # Define your environment variables
    # env_variables = {
    #     "Variable1": "Value1",
    #     "Variable2": "Value2",
    #     "Variable3": "Value3"
    # }


    # Call the function to count characters in the HTML content
    env_variables = os.environ
    character_count = count_characters_in_html(env_variables)
    print("HTTP/1.1 200 OK")
    # Set the content type to HTML
    print("Content-type: text/html")
    # Get environment variables
    print(f"Content-Length: {character_count}\r\n")
    print("\r\n")
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