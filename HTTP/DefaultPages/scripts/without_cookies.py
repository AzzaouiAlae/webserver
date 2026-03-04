#!/usr/bin/env python3
# COUNTER WITHOUT COOKIES — has redirect, but counter always resets

import os
import sys
import urllib.parse

method = os.environ.get("REQUEST_METHOD", "GET").upper()
query  = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))

# ── POST: increment then redirect ───────────────────────────────
if method == "POST":
    # Try to get count from query (passed in URL) — not from cookie
    try:
        counter = int(query.get("count", "0"))
    except ValueError:
        counter = 0

    counter += 1

    # Redirect with count in URL — no cookie set
    print("Status: 302 Found")
    print(f"Location: /without-cookies?count={counter}")
    print("Content-Type: text/html; charset=utf-8")
    print()

# ── GET: show current count from URL ────────────────────────────
else:
    try:
        counter = int(query.get("count", "0"))
    except ValueError:
        counter = 0

    print("Content-Type: text/html; charset=utf-8")
    print()
    print(f"""<!DOCTYPE html>
<html>
<head><title>Without Cookies</title>
<style>
  * {{ box-sizing:border-box; margin:0; padding:0; }}
  body {{ font-family:Arial; display:flex; justify-content:center; padding-top:80px; background:#fff0f0; }}
  .box {{ background:white; padding:32px; border-radius:10px; box-shadow:0 2px 12px rgba(0,0,0,0.1); width:340px; text-align:center; }}
  h2 {{ color:#e74c3c; margin-bottom:8px; }}
  .count {{ font-size:5em; font-weight:bold; color:#e74c3c; margin:20px 0; }}
  .warn {{ background:#fff3cd; border-left:4px solid #f39c12; padding:12px; margin:12px 0; border-radius:4px; font-size:0.85em; text-align:left; line-height:1.6; }}
  .url {{ background:#f8f9fa; padding:6px; border-radius:4px; font-family:monospace; font-size:0.75em; margin-top:6px; word-break:break-all; }}
  button {{ padding:10px 32px; background:#e74c3c; color:white; border:none; border-radius:6px; cursor:pointer; font-size:1em; }}
</style>
</head>
<body>
<div class="box">
  <h2>❌ Without Cookies</h2>
  <p>How many times did you click?</p>
  <div class="count">{counter}</div>
  <form method="POST" action="?count={counter}">
    <button type="submit">Click Me</button>
  </form>
  <div class="warn">
    ⚠ No cookie — count lives in the URL:<br>
    <div class="url">?count={counter}</div>
    Close the tab and reopen → resets to 0.<br>
    Anyone can fake it: <code>?count=9999</code>
  </div>
</div>
</body></html>""")