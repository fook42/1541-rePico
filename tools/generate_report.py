#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import os
import sys

def create_markdown_table(xml_file):
    if not os.path.exists(xml_file):
        print(f"XML report not found: {xml_file}")
        return
    
    tree = ET.parse(xml_file)
    root = tree.getroot()
    errors = root.findall('errors/error')
    
    print("| Datei | Zeile | Spalte | Severity | ID | Nachricht |")
    print("|-------|-------|--------|----------|----|---------  |")
    
    for error in sorted(errors, key=lambda x: (x.get('file0', ''), int(x.find('location').get('line', 0) if x.find('location') is not None else 0))):
        severity = error.get('severity', '?')
        if severity == 'information':
            continue
        file_elem = error.find('location')
        if file_elem is None:
            continue
        file_name = file_elem.get('file', '?').replace('firmware/', '')
        line = file_elem.get('line', '?')
        column = file_elem.get('column', '?')
        error_id = error.get('id', '?')
        message = error.get('msg', '?').replace('|', '\\|').strip()
        print(f"| `{file_name}` | {line} | {column} | {severity} | {error_id} | {message} |")

def create_error_table(xml_file, html_file):
    if not os.path.exists(xml_file):
        print(f"XML report not found: {xml_file}")
        return
    
    tree = ET.parse(xml_file)
    root = tree.getroot()
    errors = root.findall('errors/error')
    
    html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cppcheck Analysis Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .summary { background: #fff; padding: 15px; border-radius: 5px; margin: 20px 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        table { width: 100%; border-collapse: collapse; background: white; margin: 20px 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        th { background-color: #2c3e50; color: white; padding: 12px; text-align: left; }
        td { padding: 10px 12px; border-bottom: 1px solid #ddd; }
        tr:hover { background-color: #f9f9f9; }
        .error { color: #d32f2f; font-weight: bold; }
        .warning { color: #f57c00; font-weight: bold; }
        .style { color: #1976d2; font-weight: bold; }
        .info { color: #388e3c; font-weight: bold; }
    </style>
</head>
<body>
    <h1>Cppcheck Analysis Report</h1>
"""
    
    # Count by severity
    severity_counts = {}
    for error in errors:
        severity = error.get('severity', 'unknown')
        severity_counts[severity] = severity_counts.get(severity, 0) + 1
    
    html += f"<div class='summary'><h2>Summary</h2><p>Total Issues: {len(errors)}</p>"
    for severity, count in sorted(severity_counts.items()):
        html += f"<p><strong>{severity.capitalize()}:</strong> {count}</p>"
    html += "</div>"
    
    html += """<table>
    <thead>
        <tr>
            <th>File</th>
            <th>Line</th>
            <th>Column</th>
            <th>Severity</th>
            <th>ID</th>
            <th>Message</th>
        </tr>
    </thead>
    <tbody>
"""
    
    for error in sorted(errors, key=lambda x: (x.get('file0', ''), int(x.find('location').get('line', 0) if x.find('location') is not None else 0))):
        file_elem = error.find('location')
        if file_elem is None:
            continue
            
        file_name = file_elem.get('file', 'unknown')
        line = file_elem.get('line', '0')
        column = file_elem.get('column', '0')
        severity = error.get('severity', 'unknown')
        error_id = error.get('id', 'unknown')
        message = error.get('msg', 'No message')
        
        severity_class = f"class='{severity}'"
        
        html += f"""    <tr>
        <td>{file_name}</td>
        <td>{line}</td>
        <td>{column}</td>
        <td {severity_class}>{severity}</td>
        <td>{error_id}</td>
        <td>{message}</td>
    </tr>
"""
    
    html += """    </tbody>
</table>
</body>
</html>"""
    
    with open(html_file, 'w') as f:
        f.write(html)
    
    print(f"HTML report generated: {html_file}")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--markdown":
        xml_file = sys.argv[2] if len(sys.argv) > 2 else "cppcheck-report.xml"
        create_markdown_table(xml_file)
    else:
        xml_file = sys.argv[1] if len(sys.argv) > 1 else "cppcheck-report.xml"
        html_file = sys.argv[2] if len(sys.argv) > 2 else "cppcheck-report.html"
        create_error_table(xml_file, html_file)
