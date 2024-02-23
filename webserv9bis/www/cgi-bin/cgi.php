#!/usr/bin/php-cgi

<?php
header("Content-Type: text/html");

echo "<html><head><title>CGI PHP Test</title></head><body>";
echo "<h1>CGI PHP Test</h1>";
echo "<p>Request Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Query String: " . $_SERVER['QUERY_STRING'] . "</p>";
echo "</body></html>";
?>
