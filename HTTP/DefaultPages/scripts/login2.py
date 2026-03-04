#!/usr/bin/env python3
# LOGIN WITH COOKIES — Post/Redirect/Get (no refresh warning)

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
        with open(SESSION_FILE, "r") as f:
            return json.load(f)
    except Exception:
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
    if os.environ.get("REQUEST_METHOD", "GET").upper() != "POST":
        return {}
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

def redirect(location, extra_headers=None):
    if extra_headers:
        for h in extra_headers:
            print(h)
    print("Status: 302 Found")
    print(f"Location: {location}")
    print("Content-Type: text/html; charset=utf-8")
    print()

# ── Read state ───────────────────────────────────────────────────

cookies    = parse_cookies()
post       = parse_post()
sessions   = load_sessions()
session_id = cookies.get("SESSIONID", "")
session    = sessions.get(session_id)
action     = post.get("action", "")

# ── POST: login ──────────────────────────────────────────────────

if action == "login":
    username = post.get("username", "").strip()
    password = post.get("password", "").strip()

    if username in USERS and USERS[username] == password:
        new_sid = make_session_id(username)
        sessions[new_sid] = {"username": username, "created": time.time()}
        save_sessions(sessions)

        # Set cookie + redirect → browser does GET → refresh is safe
        redirect("/login-with", extra_headers=[
            f"Set-Cookie: SESSIONID={new_sid}; Path=/; Max-Age=3600; HttpOnly"
        ])

    else:
        redirect("/login-with?error=1")

# ── POST: logout ─────────────────────────────────────────────────

elif action == "logout":
    if session_id in sessions:
        del sessions[session_id]
        save_sessions(sessions)

    redirect("/login-with", extra_headers=[
        "Set-Cookie: SESSIONID=; Path=/; Max-Age=0; HttpOnly"
    ])

# ── GET: show page ───────────────────────────────────────────────

else:
    query = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))
    error = query.get("error", "")

    print("Content-Type: text/html; charset=utf-8")
    print()

    if session:
        username = session["username"]
        print(f"""<!DOCTYPE html>
<html>
<head><title>Dashboard</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#f0fff0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:380px; }}
  h2 {{ color:#27ae60; margin-bottom:12px; }}
  .good {{ background:#d4edda; border-left:4px solid #27ae60; padding:12px; margin:16px 0; border-radius:4px; font-size:0.9em; }}
  .info {{ background:#f8f9fa; padding:14px; border-radius:6px; margin:16px 0; font-family:monospace; font-size:0.85em; line-height:1.8; }}
  .info span {{ color:#0066cc; font-weight:bold; }}
  button {{ padding:10px 24px; background:#e74c3c; color:white; border:none; border-radius:6px; cursor:pointer; font-size:1em; }}
</style>
</head>
<body>
<div class="box">
  <h2>✅ Welcome, {html.escape(username)}!</h2>
  <div class="good">
    ✅ Cookie recognized — session is active.<br>
    Refresh freely, no warning will appear.
  </div>
  <div class="info">
    <span>Cookie:</span>       SESSIONID={html.escape(session_id[:16])}...<br>
    <span>Session file:</span> {SESSION_FILE}<br>
    <span>Username:</span>     {html.escape(username)}
  </div>
  <form method="POST" action="">
    <input type="hidden" name="action" value="logout">
    <button type="submit">🚪 Logout</button>
  </form>
</div>
</body></html>""")

    else:
        error_html = '<p style="color:red;margin-bottom:12px">❌ Wrong username or password.</p>' if error else ""
        print(f"""<!DOCTYPE html>
<html>
<head><title>Login With Cookies</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#f0fff0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:320px; }}
  h2 {{ color:#27ae60; margin-bottom:16px; }}
  .good {{ background:#d4edda; border-left:4px solid #27ae60; padding:12px; margin-bottom:16px; border-radius:4px; font-size:0.85em; }}
  label {{ font-size:0.85em; color:#555; }}
  input {{ width:100%; padding:10px; margin:6px 0 14px; border:1px solid #ccc; border-radius:6px; font-size:1em; }}
  button {{ width:100%; padding:10px; background:#0066cc; color:white; border:none; border-radius:6px; cursor:pointer; font-size:1em; }}
</style>
</head>
<body>
<div class="box">
  <h2>✅ Login With Cookies</h2>
  <div class="good">After login a cookie is set — refresh will not warn you.</div>
  {error_html}
  <form method="POST" action="">
    <input type="hidden" name="action" value="login">
    <label>Username</label>
    <input type="text" name="username" placeholder="admin" required autofocus>
    <label>Password</label>
    <input type="password" name="password" placeholder="password" required>
    <button type="submit">Login</button>
  </form>
  <p style="font-size:0.8em;color:#888;margin-top:12px">admin / admin123</p>
</div>
</body></html>""")
