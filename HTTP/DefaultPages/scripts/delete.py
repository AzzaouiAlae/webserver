#!/usr/bin/env python3
# test_delete.py — Demonstrates handling an HTTP DELETE request in CGI

import os
import sys
import urllib.parse
import json

def send_response(body_str, content_type="text/html; charset=utf-8", status="200 OK"):
    """Helper function to calculate content-length and send strict CGI headers."""
    body_bytes = body_str.encode('utf-8')
    content_length = len(body_bytes)
    
    sys.stdout.write(f"Status: {status}\r\n")
    sys.stdout.write(f"Content-Type: {content_type}\r\n")
    sys.stdout.write(f"Content-Length: {content_length}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(body_str)

def main():
    # 1. Get the HTTP Method and environment variables
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    query = dict(urllib.parse.parse_qsl(os.environ.get("QUERY_STRING", "")))
    
    # Dynamically get the script's path so it always targets itself
    script_name = os.environ.get("SCRIPT_NAME", "/test_delete.py")

    # ── DELETE: Handle the deletion logic ─────────────────────────
    if method == "DELETE":
        # In RESTful APIs, the target is usually in the URL query string (e.g., ?id=123)
        item_id = query.get("id", "").strip()
        
        if not item_id:
            # Send a 400 Bad Request if they forgot to tell us what to delete
            error_resp = json.dumps({"status": "error", "message": "Missing item 'id' in query string."})
            send_response(error_resp, content_type="application/json", status="400 Bad Request")
            return

        # (In a real app, you would delete the item from your database here)
        
        # Send a 200 OK success response back as JSON
        success_resp = json.dumps({
            "status": "success", 
            "message": f"Item {item_id} was successfully deleted."
        })
        send_response(success_resp, content_type="application/json", status="200 OK")
        return

    # ── GET: Show the UI to test the DELETE method ────────────────
    else:
        # We use f-strings to inject the script_name into the JavaScript
        html_body = f"""<!DOCTYPE html>
<html>
<head><title>Test HTTP DELETE</title>
<style>
  * {{ box-sizing: border-box; margin: 0; padding: 0; }}
  body {{ font-family: Arial, sans-serif; padding: 40px; background: #f0f2f5; display: flex; justify-content: center; }}
  .container {{ width: 100%; max-width: 500px; background: white; padding: 24px; border-radius: 8px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }}
  h2 {{ color: #e74c3c; margin-bottom: 12px; }}
  p {{ color: #555; font-size: 0.9em; margin-bottom: 20px; line-height: 1.5; }}
  ul {{ list-style: none; }}
  li {{ display: flex; justify-content: space-between; padding: 12px; border-bottom: 1px solid #eee; align-items: center; }}
  button {{ background: #e74c3c; color: white; border: none; padding: 8px 16px; border-radius: 6px; cursor: pointer; font-size: 0.9em; transition: background 0.2s; }}
  button:hover {{ background: #c0392b; }}
  .log-box {{ margin-top: 24px; }}
  h3 {{ font-size: 1em; color: #333; margin-bottom: 8px; }}
  #log {{ padding: 12px; background: #1e1e1e; color: #4af626; font-family: monospace; font-size: 0.85em; border-radius: 6px; white-space: pre-wrap; min-height: 80px; }}
</style>
</head>
<body>
<div class="container">
  <h2>🗑️ Test DELETE Request</h2>
  <p>Standard HTML forms only support GET and POST. To send a true <b>DELETE</b> request, we use JavaScript (Fetch API).</p>
  
  <ul id="itemList">
    <li id="item-1">📄 Document A <button onclick="deleteItem('1')">Delete</button></li>
    <li id="item-2">🖼️ Image B <button onclick="deleteItem('2')">Delete</button></li>
    <li id="item-3">📊 Report C <button onclick="deleteItem('3')">Delete</button></li>
  </ul>

  <div class="log-box">
    <h3>Server Response Log:</h3>
    <div id="log">Waiting for action...</div>
  </div>
</div>

<script>
  function deleteItem(id) {{
    const logEl = document.getElementById('log');
    logEl.textContent = "Sending DELETE request for item " + id + "...\\n";

    // Call this CGI script dynamically using the DELETE method
    fetch(`{script_name}?id=${{id}}`, {{
      method: 'DELETE'
    }})
    .then(response => {{
      logEl.textContent += "HTTP Status: " + response.status + "\\n";
      return response.json(); // We expect the server to return JSON
    }})
    .then(data => {{
      logEl.textContent += "Response Body: " + JSON.stringify(data, null, 2);
      
      // If the server confirmed the deletion, remove the item from the UI
      if (data.status === 'success') {{
         const li = document.getElementById('item-' + id);
         if (li) {{
             li.style.background = '#ffe5e5'; // Highlight red briefly
             setTimeout(() => li.remove(), 400); // Remove from list
         }}
      }}
    }})
    .catch(err => {{
      logEl.textContent += "\\nError: " + err;
    }});
  }}
</script>
</body>
</html>"""
        
        send_response(html_body)

if __name__ == "__main__":
    main()