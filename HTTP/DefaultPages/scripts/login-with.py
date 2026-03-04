#!/usr/bin/env python3
import os
import sys
import urllib.parse
import html
import hashlib
import time
import json

SESSION_FILE = "/tmp/login_sessions.json"

USERS = {
    "admin": "admin123",
    "user1": "pass1234",
}

# ── Session helpers ──────────────────────────────────────────────

def load_sessions():
    try:
        if os.path.exists(SESSION_FILE):
            with open(SESSION_FILE, "r") as f:
                return json.load(f)
    except Exception:
        pass
    return {}

def save_sessions(sessions):
    try:
        with open(SESSION_FILE, "w") as f:
            json.dump(sessions, f)
    except Exception:
        pass

def make_session_id(username):
    raw = f"{username}-{time.time()}-secret"
    return hashlib.sha256(raw.encode()).hexdigest()[:32]

# ── HTTP helpers ─────────────────────────────────────────────────

def parse_cookies():
    cookies = {}
    raw = os.environ.get("HTTP_COOKIE", "")
    for part in raw.split(";"):
        part = part.strip()
        if "=" in part:
            k, v = part.split("=", 1)
            cookies[k.strip()] = v.strip()
    return cookies

def parse_post():
    # Only try to read STDIN if it's a POST and we have a length
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        return {}
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        if length <= 0:
            return {}
        # Use .buffer.read to get raw bytes, then decode
        body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

def send_cgi_response(status="200 OK", headers=None, body=""):
    if headers is None:
        headers = []
    
    # CGI standard: Status header first
    sys.stdout.write(f"Status: {status}\r\n")
    for h in headers:
        sys.stdout.write(f"{h}\r\n")
    
    body_encoded = body.encode("utf-8")
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.write(f"Content-Length: {len(body_encoded)}\r\n")
    sys.stdout.write("\r\n") # Mandatory blank line
    
    sys.stdout.flush()
    sys.stdout.buffer.write(body_encoded)
    sys.stdout.flush()

# ── Main Logic ───────────────────────────────────────────────────


# ── Main Logic ───────────────────────────────────────────────────

def main():
    # 1. Initialize variables early to avoid NameErrors
    cookies = parse_cookies()
    post_data = parse_post()
    sessions = load_sessions()
    
    session_id = cookies.get("SESSIONID", "")
    session = sessions.get(session_id)
    
    # Ensure 'action' is always defined
    action = post_data.get("action", "")
    
    # Determine the script's actual URL path dynamically so redirects work 
    # anywhere. Fallback to /scripts/login-with.py just in case your server 
    # doesn't pass SCRIPT_NAME correctly yet.
    script_path = os.environ.get("SCRIPT_NAME", "")
    if not script_path:
        script_path = os.environ.get("REQUEST_URI", "").split('?')[0]
    if not script_path:
        script_path = "/scripts/login-with.py" 
    
    # 2. Handle POST Actions
    if action == "login":
        username = post_data.get("username", "").strip()
        password = post_data.get("password", "").strip()

        if username in USERS and USERS[username] == password:
            new_sid = make_session_id(username)
            sessions[new_sid] = {"username": username, "created": time.time()}
            save_sessions(sessions)

            # Redirect dynamically to the script itself
            cookie_header = f"Set-Cookie: SESSIONID={new_sid}; Path=/; Max-Age=3600; HttpOnly"
            send_cgi_response("302 Found", headers=[f"Location: {script_path}", cookie_header])
            return
        else:
            send_cgi_response("302 Found", headers=[f"Location: {script_path}?error=1"])
            return

    elif action == "logout":
        if session_id in sessions:
            del sessions[session_id]
            save_sessions(sessions)
        
        cookie_header = "Set-Cookie: SESSIONID=; Path=/; Max-Age=0; HttpOnly"
        send_cgi_response("302 Found", headers=[f"Location: {script_path}", cookie_header])
        return

    # 3. Handle GET / Default Display
    query = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))
    error = query.get("error", "")

    if session:
        username = session["username"]
        page_content = f"""
        <html><body>
        <h2>✅ Welcome, {html.escape(username)}!</h2>
        <p>Your session is active (ID: {html.escape(session_id[:8])}...)</p>
        <form method="POST"><input type="hidden" name="action" value="logout"><button type="submit">Logout</button></form>
        </body></html>"""
    else:
        err_msg = '<p style="color:red">❌ Invalid Login</p>' if error else ""
        page_content = f"""
        <html><body>
        <h2>Login</h2>
        {err_msg}
        <form method="POST">
            <input type="hidden" name="action" value="login">
            <input type="text" name="username" placeholder="admin" required><br>
            <input type="password" name="password" placeholder="password" required><br>
            <button type="submit">Login</button>
        </form>
        </body></html>"""
    
    send_cgi_response("200 OK", body=page_content)

if __name__ == "__main__":
    main()