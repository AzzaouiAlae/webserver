#!/usr/bin/env python3
# =============================================================================
#  login.py — CGI script for Login, Logout & Theme handling
#  Tests: CGI execution, Cookie parsing, Session management (SESSIONID)
#
#  Place in a directory configured with:
#      cgi_pass .py /usr/bin/python3
# =============================================================================

import os
import sys
import urllib.parse
import html
import json

# ─── Helpers ─────────────────────────────────────────────────────────────────

def parse_cookies():
    """Parse HTTP_COOKIE env var into a dict (mirrors ARequest::parseCookies)."""
    cookies = {}
    raw = os.environ.get("HTTP_COOKIE", "")
    for part in raw.split(";"):
        part = part.strip()
        if "=" in part:
            k, v = part.split("=", 1)
            cookies[k.strip()] = v.strip()
    return cookies

def parse_body():
    """Read POST body from stdin (mirrors CGI body-passing via pipe)."""
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    if method != "POST":
        return {}
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
        return dict(urllib.parse.parse_qsl(body))
    except Exception:
        return {}

def parse_query():
    """Parse QUERY_STRING (set by webserver from URL ?key=value)."""
    qs = os.environ.get("QUERY_STRING", "")
    return dict(urllib.parse.parse_qsl(qs))

def get_session_id(cookies):
    """Get SESSIONID from cookies (matches server-side cookie name)."""
    return cookies.get("SESSIONID", "")

# ─── Fake in-process session store (for CGI demo / testing) ──────────────────
# In production your C++ SessionManager handles this server-side.
# This simulates reading session data passed via environment variables.

def load_session_from_env():
    """
    The webserver passes session data via CGI env vars:
      SESSIONID        → the current session id
      HTTP_SESSIONID   → same (redundant alias)
    Any extra session data can be passed as custom env vars by your CGI setup.
    """
    session = {
        "id":       os.environ.get("SESSIONID", ""),
        "username": os.environ.get("SESSION_USERNAME", ""),
        "theme":    os.environ.get("SESSION_THEME", "light"),
        "logged_in": os.environ.get("SESSION_LOGGED_IN", "0"),
    }
    return session

# ─── "Database" of valid users (hardcoded for testing) ───────────────────────
USERS = {
    "admin":  "admin123",
    "user42": "pass42",
    "tester": "cgi2026",
}

# ─── Response builders ────────────────────────────────────────────────────────

def set_cookie(name, value, path="/", max_age=3600, http_only=True):
    """Build a Set-Cookie header line."""
    cookie = f"{name}={value}; Path={path}; Max-Age={max_age}"
    if http_only:
        cookie += "; HttpOnly"
    return cookie

def redirect(location):
    """Send a 302 redirect response."""
    print("Status: 302 Found")
    print(f"Location: {location}")
    print("Content-Type: text/html\r\n\r")
    print(f'<html><body>Redirecting to <a href="{html.escape(location)}">{html.escape(location)}</a></body></html>')

def render_page(title, body_html, session, cookies, extra_headers=None):
    """Render a full HTML page with debug info panel."""
    theme = session.get("theme", "light")
    themes = {
        "light": {"bg": "#f5f5f5", "fg": "#222", "card": "#fff",    "accent": "#0066cc"},
        "dark":  {"bg": "#1a1a2e", "fg": "#eee", "card": "#16213e", "accent": "#e94560"},
        "ocean": {"bg": "#0a3d62", "fg": "#f5f6fa", "card": "#1e3799", "accent": "#fbc531"},
    }
    t = themes.get(theme, themes["light"])

    session_id   = session.get("id", "none")
    username     = session.get("username", "guest")
    logged_in    = session.get("logged_in", "0") == "1"
    request_uri  = html.escape(os.environ.get("REQUEST_URI", "/"))
    method       = html.escape(os.environ.get("REQUEST_METHOD", "GET"))
    server_sw    = html.escape(os.environ.get("SERVER_SOFTWARE", "unknown"))
    gateway      = html.escape(os.environ.get("GATEWAY_INTERFACE", "unknown"))
    cookie_str   = html.escape(os.environ.get("HTTP_COOKIE", "(none)"))
    all_env      = {k: v for k, v in os.environ.items() if not k.startswith("_")}

    if extra_headers:
        for h in extra_headers:
            print(h)

    print("Content-Type: text/html; charset=utf-8")
    print()  # end of CGI headers

    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{html.escape(title)}</title>
  <style>
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body {{ background: {t['bg']}; color: {t['fg']}; font-family: 'Segoe UI', Arial, sans-serif; min-height: 100vh; }}
    .navbar {{ background: {t['accent']}; color: #fff; padding: 12px 24px; display: flex; align-items: center; justify-content: space-between; }}
    .navbar a {{ color: #fff; text-decoration: none; margin-left: 12px; font-weight: bold; }}
    .card {{ background: {t['card']}; border-radius: 10px; padding: 28px; margin: 32px auto; max-width: 480px; box-shadow: 0 4px 24px rgba(0,0,0,0.15); }}
    h1 {{ color: {t['accent']}; margin-bottom: 16px; }}
    input, select {{ width: 100%; padding: 10px; margin: 8px 0 16px; border-radius: 6px; border: 1px solid #ccc; font-size: 1em; background: {t['bg']}; color: {t['fg']}; }}
    button, .btn {{ background: {t['accent']}; color: #fff; border: none; padding: 10px 24px; border-radius: 6px; cursor: pointer; font-size: 1em; text-decoration: none; display: inline-block; margin-top: 4px; }}
    button:hover {{ opacity: 0.85; }}
    .debug {{ background: {t['card']}; border-left: 4px solid {t['accent']}; padding: 16px; margin: 24px auto; max-width: 700px; border-radius: 6px; font-size: 0.82em; font-family: monospace; overflow-x: auto; }}
    .debug summary {{ cursor: pointer; font-weight: bold; font-size: 1em; }}
    .tag {{ display: inline-block; background: {t['accent']}; color: #fff; border-radius: 4px; padding: 2px 8px; font-size: 0.75em; margin-left: 8px; }}
    .success {{ color: #27ae60; font-weight: bold; }}
    .error   {{ color: #e74c3c; font-weight: bold; }}
    table {{ width: 100%; border-collapse: collapse; }}
    td, th {{ padding: 4px 8px; border-bottom: 1px solid rgba(128,128,128,0.2); text-align: left; word-break: break-all; }}
    th {{ color: {t['accent']}; }}
  </style>
</head>
<body>
<nav class="navbar">
  <span>🌐 42 WebServer CGI Test <span class="tag">{server_sw}</span></span>
  <div>
    {'<a href="login.py?action=logout">Logout</a>' if logged_in else ''}
    <a href="login.py">Home</a>
    <a href="login.py?action=debug">Debug</a>
  </div>
</nav>
<div class="card">
  <h1>{html.escape(title)}</h1>
  {body_html}
</div>

<!-- ═══ Debug Panel ═══ -->
<div class="debug">
  <details>
    <summary>🔍 CGI / Cookie / Session Debug Panel</summary>
    <br>
    <table>
      <tr><th>Property</th><th>Value</th></tr>
      <tr><td>Session ID</td><td><code>{html.escape(session_id)}</code></td></tr>
      <tr><td>Logged In</td><td>{'<span class="success">YES</span>' if logged_in else '<span class="error">NO</span>'}</td></tr>
      <tr><td>Username</td><td>{html.escape(username)}</td></tr>
      <tr><td>Theme</td><td>{html.escape(theme)}</td></tr>
      <tr><td>HTTP_COOKIE</td><td><code>{cookie_str}</code></td></tr>
      <tr><td>REQUEST_URI</td><td><code>{request_uri}</code></td></tr>
      <tr><td>REQUEST_METHOD</td><td><code>{method}</code></td></tr>
      <tr><td>GATEWAY_INTERFACE</td><td><code>{gateway}</code></td></tr>
    </table>
    <br>
    <b>All CGI Environment Variables:</b>
    <table>
      <tr><th>Variable</th><th>Value</th></tr>
      {''.join(f"<tr><td><code>{html.escape(k)}</code></td><td><code>{html.escape(v)}</code></td></tr>" for k, v in sorted(all_env.items()))}
    </table>
    <br>
    <b>Parsed Cookies:</b>
    <table>
      <tr><th>Name</th><th>Value</th></tr>
      {''.join(f"<tr><td><code>{html.escape(k)}</code></td><td><code>{html.escape(v)}</code></td></tr>" for k, v in cookies.items()) or '<tr><td colspan=2>No cookies</td></tr>'}
    </table>
  </details>
</div>
</body>
</html>""")

# ─── Page renderers ───────────────────────────────────────────────────────────

def page_login(session, cookies, error=""):
    """Render the login form."""
    error_html = f'<p class="error">⚠ {html.escape(error)}</p>' if error else ""
    body = f"""
    {error_html}
    <form method="POST" action="login.py">
      <input type="hidden" name="action" value="login">
      <label>Username</label>
      <input type="text" name="username" placeholder="admin / user42 / tester" required autofocus>
      <label>Password</label>
      <input type="password" name="password" placeholder="Password" required>
      <label>Theme</label>
      <select name="theme">
        <option value="light">☀ Light</option>
        <option value="dark">🌙 Dark</option>
        <option value="ocean">🌊 Ocean</option>
      </select>
      <button type="submit">Login</button>
    </form>
    <br>
    <p style="font-size:0.85em; opacity:0.7;">
      Test users: <code>admin/admin123</code>, <code>user42/pass42</code>, <code>tester/cgi2026</code>
    </p>
    """
    render_page("Login", body, session, cookies)

def page_dashboard(session, cookies):
    """Render the logged-in dashboard."""
    username = html.escape(session.get("username", "?"))
    theme    = session.get("theme", "light")
    body = f"""
    <p class="success">✅ Welcome back, <strong>{username}</strong>!</p>
    <p style="margin-top:12px;">Your session is active. You can change your theme below.</p>

    <form method="POST" action="login.py" style="margin-top:20px;">
      <input type="hidden" name="action" value="theme">
      <label>Change Theme</label>
      <select name="theme">
        <option value="light"  {'selected' if theme=='light'  else ''}>☀ Light</option>
        <option value="dark"   {'selected' if theme=='dark'   else ''}>🌙 Dark</option>
        <option value="ocean"  {'selected' if theme=='ocean'  else ''}>🌊 Ocean</option>
      </select>
      <button type="submit">Apply Theme</button>
    </form>

    <form method="POST" action="login.py" style="margin-top:12px;">
      <input type="hidden" name="action" value="logout">
      <button type="submit" style="background:#e74c3c;">🚪 Logout</button>
    </form>

    <form method="POST" action="login.py" style="margin-top:12px;">
      <input type="hidden" name="action" value="set_cookie_test">
      <button type="submit" style="background:#8e44ad;">🍪 Set Test Cookie</button>
    </form>
    """
    render_page(f"Dashboard — {username}", body, session, cookies)

def page_logged_out(session, cookies):
    """Render the logged-out confirmation page."""
    body = """
    <p class="success">✅ You have been logged out.</p>
    <a class="btn" href="login.py">Back to Login</a>
    """
    render_page("Logged Out", body, session, cookies)

def page_debug(session, cookies):
    """Render a raw environment dump."""
    rows = "".join(
        f"<tr><td><code>{html.escape(k)}</code></td><td><code>{html.escape(v)}</code></td></tr>"
        for k, v in sorted(os.environ.items())
    )
    body = f"""
    <p>Full CGI environment dump:</p>
    <table style="font-size:0.8em;">{rows}</table>
    <br><a class="btn" href="login.py">Home</a>
    """
    render_page("CGI Debug Dump", body, session, cookies)

# ─── Action handlers ──────────────────────────────────────────────────────────

def handle_login(post, session, cookies):
    """Validate credentials and set session cookies."""
    username = post.get("username", "").strip()
    password = post.get("password", "").strip()
    theme    = post.get("theme", "light").strip()

    if theme not in ("light", "dark", "ocean"):
        theme = "light"

    if username in USERS and USERS[username] == password:
        # ── Set cookies that the webserver will see on next request ──
        # SESSIONID is the key cookie your C++ SessionManager looks for.
        # We also store username, theme, logged_in as separate cookies
        # so the CGI script can read them back (since CGI is stateless).
        session_id = os.environ.get("SESSIONID", "new-" + username)

        print("Status: 200 OK\r\n")
        print(set_cookie("SESSIONID",    session_id) + "\r\n")
        print(set_cookie("SESSION_USERNAME", username) + "\r\n")
        print(set_cookie("SESSION_THEME",    theme) + "\r\n")
        print(set_cookie("SESSION_LOGGED_IN", "1") + "\r\n")

        updated_session = {
            "id":        session_id,
            "username":  username,
            "theme":     theme,
            "logged_in": "1",
        }
        # Re-parse cookies to include what we just set
        cookies["SESSION_USERNAME"]  = username
        cookies["SESSION_THEME"]     = theme
        cookies["SESSION_LOGGED_IN"] = "1"
        page_dashboard(updated_session, cookies)
    else:
        print("Status: 401 Unauthorized")
        page_login(session, cookies, error="Invalid username or password.")

def handle_logout(session, cookies):
    """Clear session cookies."""
    print("Status: 200 OK")
    # Expire all session cookies
    print(set_cookie("SESSIONID",          "", max_age=0))
    print(set_cookie("SESSION_USERNAME",   "", max_age=0))
    print(set_cookie("SESSION_THEME",      "", max_age=0))
    print(set_cookie("SESSION_LOGGED_IN",  "", max_age=0))
    print(set_cookie("TEST_COOKIE",        "", max_age=0))

    cleared_session = {"id": "", "username": "", "theme": "light", "logged_in": "0"}
    cookies = {}
    page_logged_out(cleared_session, cookies)

def handle_theme(post, session, cookies):
    """Change the theme cookie."""
    theme = post.get("theme", "light").strip()
    if theme not in ("light", "dark", "ocean"):
        theme = "light"

    print("Status: 200 OK")
    print(set_cookie("SESSION_THEME", theme))

    session["theme"] = theme
    cookies["SESSION_THEME"] = theme
    page_dashboard(session, cookies)

def handle_set_cookie_test(session, cookies):
    """Set an arbitrary test cookie to verify cookie round-tripping."""
    import time
    test_val = f"webserv-test-{int(time.time())}"
    print("Status: 200 OK")
    print(set_cookie("TEST_COOKIE", test_val, max_age=60))
    cookies["TEST_COOKIE"] = test_val

    body = f"""
    <p class="success">✅ Test cookie set!</p>
    <p style="margin-top:8px;"><code>TEST_COOKIE={html.escape(test_val)}</code></p>
    <p style="margin-top:8px;opacity:0.7;">Refresh the page and check the debug panel — the cookie should appear in HTTP_COOKIE.</p>
    <br>
    <a class="btn" href="login.py">Back</a>
    """
    render_page("Cookie Test", body, session, cookies)

# ─── Main dispatcher ──────────────────────────────────────────────────────────

def main():
    cookies = parse_cookies()
    post    = parse_body()
    query   = parse_query()
    session = load_session_from_env()

    # Override session fields from cookies (CGI is stateless; data lives in cookies)
    if not session["username"] and "SESSION_USERNAME" in cookies:
        session["username"]  = cookies["SESSION_USERNAME"]
    if not session["theme"] and "SESSION_THEME" in cookies:
        session["theme"]     = cookies["SESSION_THEME"]
    if not session["logged_in"] and "SESSION_LOGGED_IN" in cookies:
        session["logged_in"] = cookies["SESSION_LOGGED_IN"]
    if not session["id"] and "SESSIONID" in cookies:
        session["id"]        = cookies["SESSIONID"]

    # Determine action from POST body or GET query string
    action = post.get("action") or query.get("action", "")
    logged_in = session.get("logged_in", "0") == "1"

    if action == "login":
        handle_login(post, session, cookies)

    elif action == "logout":
        handle_logout(session, cookies)

    elif action == "theme" and logged_in:
        handle_theme(post, session, cookies)

    elif action == "set_cookie_test" and logged_in:
        handle_set_cookie_test(session, cookies)

    elif action == "debug":
        print("Status: 200 OK")
        page_debug(session, cookies)

    else:
        # Default: show dashboard if logged in, else show login
        print("Status: 200 OK")
        if logged_in:
            page_dashboard(session, cookies)
        else:
            page_login(session, cookies)

if __name__ == "__main__":
    main()