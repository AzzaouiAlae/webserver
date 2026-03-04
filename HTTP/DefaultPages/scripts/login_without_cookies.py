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
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

post   = parse_post()
method = os.environ.get("REQUEST_METHOD", "GET").upper()
query  = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))

# ── POST: check credentials then redirect ────────────────────────
if method == "POST":
    action   = post.get("action", "")
    username = post.get("username", "").strip()
    password = post.get("password", "").strip()

    if action == "login":
        if username in USERS and USERS[username] == password:
            # Redirect to GET — no cookie set, so server won't remember
            print("Status: 302 Found")
            print("Location: /login-without?logged_in=1")
            print("Content-Type: text/html; charset=utf-8")
            print()
        else:
            print("Status: 302 Found")
            print("Location: /login-without?error=1")
            print("Content-Type: text/html; charset=utf-8")
            print()

# ── GET: show page ───────────────────────────────────────────────
else:
    logged_in = query.get("logged_in", "")
    error     = query.get("error", "")

    print("Content-Type: text/html; charset=utf-8")
    print()

    if logged_in:
        # Shows dashboard BUT only because of ?logged_in=1 in URL
        # Anyone can type this URL and see the dashboard — not secure!
        # Also: refresh removes ?logged_in=1 from URL → back to login
        print("""<!DOCTYPE html>
<html>
<head><title>Without Cookies — Dashboard</title>
<style>
  * { box-sizing:border-box; margin:0; padding:0; }
  body { font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#fff0f0; }
  .box { background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:380px; }
  h2 { color:#27ae60; margin-bottom:12px; }
  .warn { background:#fff3cd; border-left:4px solid #f39c12; padding:12px; margin:16px 0; border-radius:4px; font-size:0.9em; line-height:1.6; }
  .url { background:#f8f9fa; padding:8px; border-radius:4px; font-family:monospace; font-size:0.8em; margin:8px 0; word-break:break-all; }
  a { display:inline-block; margin-top:16px; color:#0066cc; }
</style>
</head>
<body>
<div class="box">
  <h2>⚠ Logged in... kind of</h2>
  <div class="warn">
    ⚠ <b>No cookie was set.</b><br><br>
    The redirect worked — no refresh warning.<br>
    But look at the URL:
    <div class="url">http://localhost/login-without?logged_in=1</div>
    The server only thinks you're logged in because
    of <b>?logged_in=1</b> in the URL.<br><br>
    Problems:<br>
    • Anyone can type this URL and get in<br>
    • Remove <b>?logged_in=1</b> from URL → logged out<br>
    • This is <b>not real authentication</b>
  </div>
  <a href="/login-without">← Back to Login</a>
</div>
</body></html>""")

    else:
        error_html = '<p style="color:red;margin-bottom:12px">❌ Wrong username or password.</p>' if error else ""
        print(f"""<!DOCTYPE html>
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