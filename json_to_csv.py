#!/usr/bin/env python3
"""
JSON to CSV Converter for BuccLog Stats
Converts stats.json performance metrics to CSV format for time-series analysis.
"""

import json
import csv
from datetime import datetime
from pathlib import Path


def convert_json_to_csv(json_file_path, csv_file_path=None):
    """
    Convert stats.json to CSV format.
    
    Args:
        json_file_path: Path to the input JSON file
        csv_file_path: Path to the output CSV file (optional, defaults to same name with .csv extension)
    """
    # Set default output path if not provided
    if csv_file_path is None:
        json_path = Path(json_file_path)
        csv_file_path = json_path.parent / f"{json_path.stem}.csv"
    
    # Read the JSON file
    print(f"Reading JSON file: {json_file_path}")
    with open(json_file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    print(f"Found {len(data)} records")
    
    # Collect all unique metric names across all records
    all_metrics = set()
    for record in data:
        if 'metrics' in record:
            all_metrics.update(record['metrics'].keys())
    
    # Sort metrics for consistent column order
    metric_names = sorted(all_metrics)
    
    # Create CSV header
    header = ['timestamp', 'timestamp_readable', 'id'] + metric_names
    
    # Prepare rows
    rows = []
    for record in data:
        # Base fields
        timestamp = record.get('timestamp', '')
        record_id = record.get('id', '')
        
        # Convert timestamp to readable format (assuming milliseconds since epoch)
        timestamp_readable = ''
        if timestamp:
            try:
                # Adjust divisor based on timestamp magnitude
                # If timestamp is very large, it might be in microseconds or nanoseconds
                if timestamp > 10**12:  # Likely microseconds or nanoseconds
                    timestamp_seconds = timestamp / 10**6  # Try microseconds first
                else:
                    timestamp_seconds = timestamp / 1000  # Milliseconds
                timestamp_readable = datetime.fromtimestamp(timestamp_seconds).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            except (ValueError, OSError):
                timestamp_readable = 'N/A'
        
        # Create row with base fields
        row = {
            'timestamp': timestamp,
            'timestamp_readable': timestamp_readable,
            'id': record_id
        }
        
        # Add metric values
        metrics = record.get('metrics', {})
        for metric_name in metric_names:
            if metric_name in metrics:
                row[metric_name] = metrics[metric_name].get('value', '')
            else:
                row[metric_name] = ''  # Empty value if metric not present in this record
        
        rows.append(row)
    
    # Write to CSV
    print(f"Writing CSV file: {csv_file_path}")
    with open(csv_file_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=header)
        writer.writeheader()
        writer.writerows(rows)
    
    print(f"Successfully converted {len(rows)} records")
    print(f"CSV file created: {csv_file_path}")
    print(f"Columns: {len(header)} ({len(metric_names)} metrics)")
    
    return csv_file_path


def main():
    """Main entry point for the script."""
    import sys
    
    # Get input file from command line or use default
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    else:
        # Default to stats.json in the same directory as the script
        script_dir = Path(__file__).parent
        input_file = script_dir / "stats.json"
    
    # Get output file from command line (optional)
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    # Check if input file exists
    if not Path(input_file).exists():
        print(f"Error: Input file not found: {input_file}")
        print("\nUsage: python json_to_csv.py [input_json_file] [output_csv_file]")
        sys.exit(1)
    
    # Convert the file
    try:
        convert_json_to_csv(input_file, output_file)
    except Exception as e:
        print(f"Error during conversion: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
