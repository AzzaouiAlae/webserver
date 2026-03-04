#!/usr/bin/env python3
# LOGIN WITHOUT COOKIES — has redirect, but still forgets you

import os
import sys
import urllib.parse
import html

USERS = {
    "admin": "admin123",
    "user1": "pass1234",
}

def parse_post():
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        return {}
    try:
        # Safely handle CONTENT_LENGTH just like we did in the previous script
        length_env = os.environ.get("CONTENT_LENGTH")
        if length_env and length_env.isdigit():
            length = int(length_env)
            body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        else:
            body = sys.stdin.read()
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

def send_response(html_body):
    """Helper function to calculate content-length and send strict CGI headers."""
    body_bytes = html_body.encode('utf-8')
    content_length = len(body_bytes)
    
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.write(f"Content-Length: {content_length}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(html_body)

post   = parse_post()
method = os.environ.get("REQUEST_METHOD", "GET").upper()
query  = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))

# Dynamically get the script's path so it always redirects to itself
script_name = os.environ.get("SCRIPT_NAME", "")
# Get the host for display purposes
http_host = os.environ.get("HTTP_HOST", "localhost")

# ── POST: check credentials then redirect ────────────────────────
if method == "POST":
    action   = post.get("action", "")
    username = post.get("username", "").strip()
    password = post.get("password", "").strip()

    if action == "login":
        if username in USERS and USERS[username] == password:
            # Redirect back to the script dynamically using script_name
            sys.stdout.write("Status: 302 Found\r\n")
            sys.stdout.write(f"Location: {script_name}?logged_in=1\r\n")
            sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
            sys.stdout.write("Content-Length: 0\r\n")
            sys.stdout.write("\r\n")
        else:
            # Redirect back to the script with an error
            sys.stdout.write("Status: 302 Found\r\n")
            sys.stdout.write(f"Location: {script_name}?error=1\r\n")
            sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
            sys.stdout.write("Content-Length: 0\r\n")
            sys.stdout.write("\r\n")

# ── GET: show page ───────────────────────────────────────────────
else:
    logged_in = query.get("logged_in", "")
    error     = query.get("error", "")

    if logged_in:
        html_body = f"""<!DOCTYPE html>
<html>
<head><title>Without Cookies — Dashboard</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#fff0f0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:380px; }}
  h2 {{ color:#27ae60; margin-bottom:12px; }}
  .warn {{ background:#fff3cd; border-left:4px solid #f39c12; padding:12px; margin:16px 0; border-radius:4px; font-size:0.9em; line-height:1.6; }}
  .url {{ background:#f8f9fa; padding:8px; border-radius:4px; font-family:monospace; font-size:0.8em; margin:8px 0; word-break:break-all; }}
  a {{ display:inline-block; margin-top:16px; color:#0066cc; }}
</style>
</head>
<body>
<div class="box">
  <h2>⚠ Logged in... kind of</h2>
  <div class="warn">
    ⚠ <b>No cookie was set.</b><br><br>
    The redirect worked — no refresh warning.<br>
    But look at the URL:
    <div class="url">http://{http_host}{script_name}?logged_in=1</div>
    The server only thinks you're logged in because
    of <b>?logged_in=1</b> in the URL.<br><br>
    Problems:<br>
    • Anyone can type this URL and get in<br>
    • Remove <b>?logged_in=1</b> from URL → logged out<br>
    • This is <b>not real authentication</b>
  </div>
  <a href="{script_name}">← Back to Login</a>
</div>
</body></html>"""
        send_response(html_body)

    else:
        error_html = '<p style="color:red;margin-bottom:12px">❌ Wrong username or password.</p>' if error else ""
        html_body = f"""<!DOCTYPE html>
<html>
<head><title>Without Cookies — Login</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#fff0f0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:320px; }}
  h2 {{ color:#e74c3c; margin-bottom:16px; }}
  .warn {{ background:#fff3cd; border-left:4px solid #f39c12; padding:12px; margin-bottom:16px; border-radius:4px; font-size:0.85em; }}
  label {{ font-size:0.85em; color:#555; }}
  input {{ width:100%; padding:10px; margin:6px 0 14px; border:1px solid #ccc; border-radius:6px; font-size:1em; }}
  button {{ width:100%; padding:10px; background:#e74c3c; color:white; border:none; border-radius:6px; cursor:pointer; font-size:1em; }}
</style>
</head>
<body>
<div class="box">
  <h2>❌ Login Without Cookies</h2>
  <div class="warn">
    ⚠ Has redirect (no refresh warning)<br>
    but no cookies — server still forgets you.
  </div>
  {error_html}
  <form method="POST" action="{script_name}">
    <input type="hidden" name="action" value="login">
    <label>Username</label>
    <input type="text" name="username" placeholder="admin" required autofocus>
    <label>Password</label>
    <input type="password" name="password" placeholder="password" required>
    <button type="submit">Login</button>
  </form>
  <p style="font-size:0.8em;color:#888;margin-top:12px">admin / admin123</p>
</div>
</body></html>"""
        send_response(html_body)