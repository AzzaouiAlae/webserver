#!/usr/bin/env python3
# =============================================================================
#  login_simple.py — Minimal CGI Login + Cookie/Session Management
#  Place in your cgi scripts directory, configured with:
#      cgi_pass .py /usr/bin/python3
# =============================================================================

import os
import sys
import urllib.parse
import html
import hashlib
import time

# ─── In-memory session store (lives as long as the process — for testing) ────
# In production, your C++ server's SessionManager handles this.
SESSIONS = {}

# ─── Hardcoded users (username: password) ────────────────────────────────────
USERS = {
    "admin": "admin123",
    "user1": "pass1234",
}

# ─── Helpers ──────────────────────────────────────────────────────────────────

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
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        return {}
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

def parse_query():
    return dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))

def make_session_id(username):
    """Generate a simple session ID."""
    raw = f"{username}-{time.time()}-secret"
    return hashlib.sha256(raw.encode()).hexdigest()[:32]

def set_cookie(name, value, max_age=3600):
    return f"Set-Cookie: {name}={value}; Path=/; Max-Age={max_age}; HttpOnly"

def send_headers(status="200 OK", extra_headers=None):
    print(f"Status: {status}")
    if extra_headers:
        for h in extra_headers:
            print(h)
    print("Content-Type: text/html; charset=utf-8")
    print()  # blank line = end of headers

# ─── Pages ────────────────────────────────────────────────────────────────────

def page_login(error=""):
    error_html = f'<p style="color:red">{html.escape(error)}</p>' if error else ""
    send_headers("200 OK")
    print(f"""<!DOCTYPE html>
<html>
<head><title>Login</title>
<style>
  body {{ font-family: Arial, sans-serif; display: flex; justify-content: center; padding-top: 80px; background: #f0f2f5; }}
  .box {{ background: white; padding: 32px; border-radius: 10px; box-shadow: 0 2px 12px rgba(0,0,0,0.1); width: 320px; }}
  h2 {{ margin-bottom: 16px; color: #333; }}
  input {{ width: 100%; padding: 10px; margin: 6px 0 14px; border: 1px solid #ccc; border-radius: 6px; box-sizing: border-box; }}
  button {{ width: 100%; padding: 10px; background: #0066cc; color: white; border: none; border-radius: 6px; cursor: pointer; font-size: 1em; }}
  button:hover {{ background: #0052a3; }}
</style>
</head>
<body>
<div class="box">
  <h2>🔐 Login</h2>
  {error_html}
  <form method="POST" action="">
    <input type="hidden" name="action" value="login">
    <label>Username</label>
    <input type="text" name="username" placeholder="admin" required autofocus>
    <label>Password</label>
    <input type="password" name="password" placeholder="password" required>
    <button type="submit">Login</button>
  </form>
  <p style="font-size:0.8em; color:#888; margin-top:12px;">
    Hint: <code>admin / admin123</code>
  </p>
</div>
</body></html>""")

def page_dashboard(username, session_id):
    send_headers("200 OK")
    print(f"""<!DOCTYPE html>
<html>
<head><title>Dashboard</title>
<style>
  body {{ font-family: Arial, sans-serif; display: flex; justify-content: center; padding-top: 80px; background: #f0f2f5; }}
  .box {{ background: white; padding: 32px; border-radius: 10px; box-shadow: 0 2px 12px rgba(0,0,0,0.1); width: 360px; }}
  h2 {{ color: #27ae60; }}
  .info {{ background: #f8f9fa; padding: 12px; border-radius: 6px; margin: 16px 0; font-size: 0.85em; font-family: monospace; }}
  button {{ padding: 10px 20px; background: #e74c3c; color: white; border: none; border-radius: 6px; cursor: pointer; }}
</style>
</head>
<body>
<div class="box">
  <h2>✅ Welcome, {html.escape(username)}!</h2>
  <p>You are logged in.</p>
  <div class="info">
    <b>Session ID:</b> {html.escape(session_id)}<br>
    <b>Username:</b>   {html.escape(username)}
  </div>
  <form method="POST" action="">
    <input type="hidden" name="action" value="logout">
    <button type="submit">🚪 Logout</button>
  </form>
</div>
</body></html>""")

def page_logged_out():
    send_headers("200 OK")
    print("""<!DOCTYPE html>
<html>
<head><title>Logged Out</title>
<style>
  body { font-family: Arial, sans-serif; display: flex; justify-content: center; padding-top: 80px; background: #f0f2f5; }
  .box { background: white; padding: 32px; border-radius: 10px; box-shadow: 0 2px 12px rgba(0,0,0,0.1); width: 320px; text-align: center; }
  a { color: #0066cc; }
</style>
</head>
<body>
<div class="box">
  <h2>👋 Logged Out</h2>
  <p>Your session has been cleared.</p>
  <br>
  <a href="">Back to Login</a>
</div>
</body></html>""")

# ─── Main ─────────────────────────────────────────────────────────────────────

def main():
    cookies = parse_cookies()
    post    = parse_post()
    query   = parse_query()
    action  = post.get("action") or query.get("action", "")

    # ── Check existing session from cookie ──
    session_id = cookies.get("SESSIONID", "")
    session    = SESSIONS.get(session_id)  # None if not found / expired

    # ── Handle actions ──

    if action == "login":
        username = post.get("username", "").strip()
        password = post.get("password", "").strip()

        if username in USERS and USERS[username] == password:
            # Create session
            new_sid = make_session_id(username)
            SESSIONS[new_sid] = {"username": username, "created": time.time()}

            send_headers("200 OK", extra_headers=[
                set_cookie("SESSIONID", new_sid, max_age=3600)
            ])
            # Re-render dashboard directly (redirect would lose the Set-Cookie in some CGI setups)
            page_dashboard(username, new_sid)
            return
        else:
            page_login(error="Invalid username or password.")
            return

    elif action == "logout":
        # Expire the session cookie
        send_headers("200 OK", extra_headers=[
            set_cookie("SESSIONID", "", max_age=0)
        ])
        if session_id in SESSIONS:
            del SESSIONS[session_id]
        page_logged_out()
        return

    # ── Default: show dashboard if session valid, else login ──
    if session:
        page_dashboard(session["username"], session_id)
    else:
        page_login()

if __name__ == "__main__":
    main()