#!/usr/bin/env python

import cgi
import os

print("Content-Type: text/html")  # HTML is following
print()                           # blank line, end of headers

form = cgi.FieldStorage()  # Parse les données du formulaire

print("<html><head><title>CGI Python Test</title></head><body>")
print("<h1>CGI Python Test</h1>")
print("<p>Request Method: {}</p>".format(os.environ["REQUEST_METHOD"]))

if os.environ["REQUEST_METHOD"] == "POST":
    print("<h2>POST Data:</h2>")
    for key in form.keys():
        print("<p>{}: {}</p>".format(key, form.getvalue(key)))
else:
    print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))

print("</body></html>")
