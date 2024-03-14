#!/usr/bin/python3
import uuid

print("Content-Type: text/html; charset=utf-8")
print("Set-Cookie: testCookie=simpleTestValue; Path=/")
print("Set-Cookie: testCookieID=" + str(uuid.uuid4()) + "; Path=/; HttpOnly")
print()

print("""
<html>
<head>
    <title>Test CGI Cookies</title>
</head>
<body>
    <p>Un nouveau cookie avec ID unique a été défini.</p>
</body>
</html>
""")


