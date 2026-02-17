/**
 * @file templates.h
 * @brief HTML templates for directory listing and other responses
 */

#ifndef TEMPLATES_H
#define TEMPLATES_H

// Directory listing HTML template - Header
static const char *DIR_LISTING_HTML_HEAD = 
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<meta charset=\"utf-8\">\n"
"<title>Index of %s</title>\n"
"<style>\n"
"body { font-family: monospace; margin: 40px; background: #f5f5f5; }\n"
"h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
"table { border-collapse: collapse; width: 100%%; background: white; }\n"
"th { background: #e0e0e0; text-align: left; padding: 8px; border-bottom: 2px solid #ccc; }\n"
"td { padding: 6px 8px; border-bottom: 1px solid #eee; }\n"
"tr:hover { background: #f9f9f9; }\n"
"a { color: #0066cc; text-decoration: none; }\n"
"a:hover { text-decoration: underline; }\n"
".dir { font-weight: bold; }\n"
".size { text-align: right; }\n"
".date { color: #666; }\n"
"hr { border: none; border-top: 1px solid #ccc; margin: 20px 0; }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h1>Index of %s</h1>\n";

// Directory listing HTML - Table header
static const char *DIR_LISTING_TABLE_HEAD = 
"<table>\n"
"<tr><th>Name</th><th>Last Modified</th><th class=\"size\">Size</th></tr>\n";

// Directory listing HTML - Parent directory row
static const char *DIR_LISTING_PARENT_ROW = 
"<tr><td colspan=\"3\"><a href=\"../\" class=\"dir\">[Parent Directory]</a></td></tr>\n";

// Directory listing HTML - Footer
static const char *DIR_LISTING_HTML_FOOTER = 
"</table>\n"
"<hr>\n"
"<i>Pico HTTP Server</i>\n"
"</body>\n"
"</html>\n";

#endif // TEMPLATES_H
