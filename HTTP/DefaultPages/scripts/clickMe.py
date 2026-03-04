#!/usr/bin/env python3
# COUNTER WITH COOKIES — Post/Redirect/Get (no refresh warning)

import os
import sys
import urllib.parse

def parse_cookies():
    cookies = {}
    raw = os.environ.get("HTTP_COOKIE", "")
    for part in raw.split(";"):
        part = part.strip()
        if "=" in part:
            k, v = part.split("=", 1)
            cookies[k.strip()] = v.strip()
    return cookies

def send_response(html_body):
    """Helper function to calculate content-length and send strict CGI headers."""
    body_bytes = html_body.encode('utf-8')
    content_length = len(body_bytes)
    
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.write(f"Content-Length: {content_length}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(html_body)

cookies = parse_cookies()
method  = os.environ.get("REQUEST_METHOD", "GET").upper()

# Dynamically get the script's path so it always redirects and posts to itself
script_name = os.environ.get("SCRIPT_NAME", "")

# ── POST: increment counter then redirect ────────────────────────
if method == "POST":
    try:
        counter = int(cookies.get("VISIT_COUNT", "0"))
    except ValueError:
        counter = 0

    counter += 1

    # Save new count in cookie + redirect to GET dynamically using script_name
    sys.stdout.write("Status: 302 Found\r\n")
    sys.stdout.write(f"Location: {script_name}\r\n")
    sys.stdout.write(f"Set-Cookie: VISIT_COUNT={counter}; Path=/; Max-Age=3600\r\n")
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.write("Content-Length: 0\r\n")
    sys.stdout.write("\r\n")

# ── GET: just show the current count ────────────────────────────
else:
    try:
        counter = int(cookies.get("VISIT_COUNT", "0"))
    except ValueError:
        counter = 0

    html_body = f"""<!DOCTYPE html>
<html>
<head><title>With Cookies</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#f0fff0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:320px; text-align:center; }}
  h2 {{ color:#27ae60; margin-bottom:8px; }}
  .count {{ font-size:5em; font-weight:bold; color:#27ae60; margin:20px 0; }}
  .cookie {{ background:#f8f9fa; padding:8px; border-radius:4px; font-family:monospace; font-size:0.85em; margin:12px 0; }}
  button {{ padding:10px 32px; background:#27ae60; color:white; border:none; border-radius:6px; cursor:pointer; font-size:1em; }}
  .note {{ color:#888; font-size:0.8em; margin-top:12px; }}
</style>
</head>
<body>
<div class="box">
  <h2>✅ With Cookies</h2>
  <p>How many times did you click?</p>
  <div class="count">{counter}</div>
  <form method="POST" action="{script_name}">
    <button type="submit">Click Me</button>
  </form>
  <div class="cookie">Cookie: VISIT_COUNT = <b>{counter}</b></div>
  <p class="note">✅ Count grows and persists. Refresh is safe — no warning!</p>
</div>
</body></html>"""

    send_response(html_body)