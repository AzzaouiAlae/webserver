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
    error_html = f'<div style="background:rgba(239,68,68,0.15);border:1px solid rgba(239,68,68,0.3);border-radius:10px;padding:12px;margin-bottom:18px;color:#f87171;font-size:0.9em;">{html.escape(error)}</div>' if error else ""
    send_headers("200 OK")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>Login — Enginx</title>
<style>
  :root {{ --primary:#6366f1;--accent:#06b6d4;--accent2:#8b5cf6;--bg:#030712;--text:#f1f5f9;--text-muted:#94a3b8;--border:rgba(99,102,241,0.15); }}
  * {{ margin:0;padding:0;box-sizing:border-box; }}
  body {{ font-family:'Segoe UI',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;display:flex;align-items:center;justify-content:center; }}
  .card {{ background:rgba(15,23,42,0.6);border:1px solid var(--border);border-radius:16px;padding:40px;width:380px;backdrop-filter:blur(20px);animation:fadeIn .8s ease-out; }}
  @keyframes fadeIn {{ from{{opacity:0;transform:translateY(30px)}} to{{opacity:1;transform:translateY(0)}} }}
  .logo {{ display:flex;align-items:center;justify-content:center;gap:0.5rem;margin-bottom:2rem;opacity:0.7; }}
  .logo svg {{ width:28px;height:28px; }}
  .logo span {{ font-weight:800;font-size:1rem;background:linear-gradient(135deg,#fff,var(--primary));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }}
  h2 {{ font-size:1.6rem;font-weight:800;margin-bottom:8px;background:linear-gradient(135deg,var(--primary),var(--accent),var(--accent2));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }}
  .subtitle {{ color:var(--text-muted);font-size:0.9em;margin-bottom:24px; }}
  .divider {{ width:60px;height:3px;background:linear-gradient(90deg,var(--primary),var(--accent));border-radius:2px;margin:0 auto 24px; }}
  label {{ font-size:0.85em;color:var(--text-muted);display:block;margin-bottom:6px; }}
  input[type="text"],input[type="password"] {{ width:100%;padding:12px 16px;margin-bottom:16px;border:1px solid var(--border);border-radius:10px;background:rgba(15,23,42,0.8);color:var(--text);font-size:1em;outline:none;transition:border-color .3s; }}
  input[type="text"]:focus,input[type="password"]:focus {{ border-color:var(--primary);box-shadow:0 0 20px rgba(99,102,241,0.15); }}
  button {{ width:100%;padding:12px;border:none;border-radius:10px;font-size:1em;font-weight:600;color:#fff;background:linear-gradient(135deg,var(--primary),var(--accent2));box-shadow:0 4px 24px rgba(99,102,241,0.3);cursor:pointer;transition:all .4s cubic-bezier(.4,0,.2,1);position:relative;overflow:hidden; }}
  button::before {{ content:'';position:absolute;top:0;left:-100%;width:100%;height:100%;background:linear-gradient(90deg,transparent,rgba(255,255,255,0.15),transparent);transition:left .5s; }}
  button:hover::before {{ left:100%; }}
  button:hover {{ transform:translateY(-2px);box-shadow:0 8px 40px rgba(99,102,241,0.5); }}
  .hint {{ font-size:0.8em;color:var(--text-muted);margin-top:16px;text-align:center; }}
  .hint code {{ background:rgba(99,102,241,0.15);padding:2px 8px;border-radius:4px;color:var(--accent); }}
</style>
</head>
<body>
<div class="card">
  <div class="logo">
    <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg"><defs><linearGradient id="lg" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#6366f1"/><stop offset="50%" stop-color="#06b6d4"/><stop offset="100%" stop-color="#8b5cf6"/></linearGradient></defs><polygon points="50,8 92,30 92,70 50,92 8,70 8,30" fill="none" stroke="url(#lg)" stroke-width="3"/><text x="50" y="58" text-anchor="middle" font-family="system-ui" font-weight="900" font-size="28" fill="url(#lg)">E</text></svg>
    <span>Enginx</span>
  </div>
  <h2>🔐 Login</h2>
  <p class="subtitle">Sign in to your account</p>
  <div class="divider"></div>
  {error_html}
  <form method="POST" action="">
    <input type="hidden" name="action" value="login">
    <label>Username</label>
    <input type="text" name="username" placeholder="admin" required autofocus>
    <label>Password</label>
    <input type="password" name="password" placeholder="password" required>
    <button type="submit">Login</button>
  </form>
  <p class="hint">Hint: <code>admin / admin123</code></p>
</div>
</body></html>""")
def page_dashboard(username, session_id):
    send_headers("200 OK")
    print(f"""<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>Dashboard — Enginx</title>
<style>
  :root {{ --primary:#6366f1;--accent:#06b6d4;--accent2:#8b5cf6;--bg:#030712;--text:#f1f5f9;--text-muted:#94a3b8;--border:rgba(99,102,241,0.15); }}
  * {{ margin:0;padding:0;box-sizing:border-box; }}
  body {{ font-family:'Segoe UI',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;display:flex;align-items:center;justify-content:center; }}
  .card {{ background:rgba(15,23,42,0.6);border:1px solid var(--border);border-radius:16px;padding:40px;width:420px;backdrop-filter:blur(20px);animation:fadeIn .8s ease-out; }}
  @keyframes fadeIn {{ from{{opacity:0;transform:translateY(30px)}} to{{opacity:1;transform:translateY(0)}} }}
  .logo {{ display:flex;align-items:center;justify-content:center;gap:0.5rem;margin-bottom:2rem;opacity:0.7; }}
  .logo svg {{ width:28px;height:28px; }}
  .logo span {{ font-weight:800;font-size:1rem;background:linear-gradient(135deg,#fff,var(--primary));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }}
  h2 {{ font-size:1.6rem;font-weight:800;margin-bottom:8px;background:linear-gradient(135deg,var(--accent),var(--primary),var(--accent2));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }}
  .subtitle {{ color:var(--text-muted);font-size:0.9em;margin-bottom:20px; }}
  .divider {{ width:60px;height:3px;background:linear-gradient(90deg,var(--accent),var(--primary));border-radius:2px;margin:0 auto 24px; }}
  .info {{ background:rgba(15,23,42,0.8);border:1px solid var(--border);padding:16px;border-radius:10px;margin:20px 0;font-family:'JetBrains Mono',monospace;font-size:0.85em;line-height:1.8;color:var(--text-muted); }}
  .info b {{ color:var(--accent); }}
  button {{ padding:12px 28px;border:none;border-radius:10px;font-size:1em;font-weight:600;color:#fff;background:linear-gradient(135deg,#ef4444,#dc2626);box-shadow:0 4px 24px rgba(239,68,68,0.3);cursor:pointer;transition:all .4s cubic-bezier(.4,0,.2,1);position:relative;overflow:hidden; }}
  button::before {{ content:'';position:absolute;top:0;left:-100%;width:100%;height:100%;background:linear-gradient(90deg,transparent,rgba(255,255,255,0.15),transparent);transition:left .5s; }}
  button:hover::before {{ left:100%; }}
  button:hover {{ transform:translateY(-2px);box-shadow:0 8px 40px rgba(239,68,68,0.5); }}
</style>
</head>
<body>
<div class="card">
  <div class="logo">
    <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg"><defs><linearGradient id="lg" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#6366f1"/><stop offset="50%" stop-color="#06b6d4"/><stop offset="100%" stop-color="#8b5cf6"/></linearGradient></defs><polygon points="50,8 92,30 92,70 50,92 8,70 8,30" fill="none" stroke="url(#lg)" stroke-width="3"/><text x="50" y="58" text-anchor="middle" font-family="system-ui" font-weight="900" font-size="28" fill="url(#lg)">E</text></svg>
    <span>Enginx</span>
  </div>
  <h2>✅ Welcome, {html.escape(username)}!</h2>
  <p class="subtitle">You are logged in.</p>
  <div class="divider"></div>
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
<html lang="en">
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>Logged Out — Enginx</title>
<style>
  :root { --primary:#6366f1;--accent:#06b6d4;--accent2:#8b5cf6;--bg:#030712;--text:#f1f5f9;--text-muted:#94a3b8;--border:rgba(99,102,241,0.15); }
  * { margin:0;padding:0;box-sizing:border-box; }
  body { font-family:'Segoe UI',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;display:flex;align-items:center;justify-content:center; }
  .card { background:rgba(15,23,42,0.6);border:1px solid var(--border);border-radius:16px;padding:40px;width:380px;text-align:center;backdrop-filter:blur(20px);animation:fadeIn .8s ease-out; }
  @keyframes fadeIn { from{opacity:0;transform:translateY(30px)} to{opacity:1;transform:translateY(0)} }
  .logo { display:flex;align-items:center;justify-content:center;gap:0.5rem;margin-bottom:2rem;opacity:0.7; }
  .logo svg { width:28px;height:28px; }
  .logo span { font-weight:800;font-size:1rem;background:linear-gradient(135deg,#fff,var(--primary));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }
  h2 { font-size:1.6rem;font-weight:800;margin-bottom:8px;background:linear-gradient(135deg,var(--primary),var(--accent),var(--accent2));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text; }
  p { color:var(--text-muted);margin-bottom:20px; }
  .divider { width:60px;height:3px;background:linear-gradient(90deg,var(--primary),var(--accent));border-radius:2px;margin:0 auto 24px; }
  a { display:inline-flex;align-items:center;gap:0.5rem;padding:0.8rem 2rem;border-radius:12px;font-size:0.95rem;font-weight:600;text-decoration:none;color:#fff;background:linear-gradient(135deg,var(--primary),var(--accent2));box-shadow:0 4px 24px rgba(99,102,241,0.3);transition:all .4s cubic-bezier(.4,0,.2,1);position:relative;overflow:hidden; }
  a::before { content:'';position:absolute;top:0;left:-100%;width:100%;height:100%;background:linear-gradient(90deg,transparent,rgba(255,255,255,0.15),transparent);transition:left .5s; }
  a:hover::before { left:100%; }
  a:hover { transform:translateY(-2px);box-shadow:0 8px 40px rgba(99,102,241,0.5); }
</style>
</head>
<body>
<div class="card">
  <div class="logo">
    <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg"><defs><linearGradient id="lg" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#6366f1"/><stop offset="50%" stop-color="#06b6d4"/><stop offset="100%" stop-color="#8b5cf6"/></linearGradient></defs><polygon points="50,8 92,30 92,70 50,92 8,70 8,30" fill="none" stroke="url(#lg)" stroke-width="3"/><text x="50" y="58" text-anchor="middle" font-family="system-ui" font-weight="900" font-size="28" fill="url(#lg)">E</text></svg>
    <span>Enginx</span>
  </div>
  <h2>👋 Logged Out</h2>
  <div class="divider"></div>
  <p>Your session has been cleared.</p>
  <a href="">← Back to Login</a>
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