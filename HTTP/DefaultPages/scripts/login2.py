#!/usr/bin/env python3
import os
import sys
import urllib.parse

def main():
    # 1. Process the request and environment variables
    request_method = os.environ.get("REQUEST_METHOD", "GET")
    message = ""

    # Handle POST data
    if request_method == "POST":
        content_length_env = int(os.environ.get("CONTENT_LENGTH", 0))
        # Read the exact number of bytes specified by the server
        post_data = sys.stdin.read(content_length_env)
        parsed_data = urllib.parse.parse_qs(post_data)
        
        username = parsed_data.get("username", [""])[0]
        password = parsed_data.get("password", [""])[0]
        
        if username == "admin" and password == "secret":
            message = "<p style='color: green;'><strong>Login successful!</strong></p>"
        else:
            message = "<p style='color: red;'><strong>Invalid credentials.</strong></p>"

    # 2. Generate the entire HTML body FIRST (so we can measure it)
    html_body = f"""<!DOCTYPE html>
<html>
<head><title>Strict CGI Login</title></head>
<body>
<h2>Strict CGI Login Test</h2>
{message}
<hr>
<form method="POST" action="">
    <label>Username:</label><br>
    <input type="text" name="username"><br><br>
    
    <label>Password:</label><br>
    <input type="password" name="password"><br><br>
    
    <input type="submit" value="Log In">
</form>
</body>
</html>
"""

    # 3. Calculate Content-Length
    # We must measure the length in bytes, not characters, in case of UTF-8 symbols
    body_bytes = html_body.encode('utf-8')
    content_length = len(body_bytes)

    # 4. Output Headers with strict \r\n
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.write(f"Content-Length: {content_length}\r\n")
    
    # The mandatory blank line (\r\n) that separates headers from the body
    sys.stdout.write("\r\n")
    
    # 5. Output the body
    sys.stdout.write(html_body)

if __name__ == "__main__":
    main()