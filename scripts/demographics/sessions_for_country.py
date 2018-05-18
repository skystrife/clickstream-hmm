import argparse
import csv
import re
import sys

parser = argparse.ArgumentParser(
    description='Extract session ids for users who were born/reside in a '\
                'specific country')
parser.add_argument('survey_csv', help='survey data CSV file')
parser.add_argument(
    'country', help='regular expression for matching the country')
parser.add_argument(
    '--reside',
    help='find users who reside in the specified country (default: find ' \
         'users who were born there',
    action='store_true')

args = parser.parse_args()

with open(args.survey_csv, 'r', encoding='utf-8-sig') as csvfile:
    reader = csv.DictReader(csvfile)
    # there are two types of survey data CSVs, so determine what format we
    # are dealing with
    if 'V1' in reader.fieldnames:
        print('Old format detected...', file=sys.stderr)

        # Q3 == "In what country were you born?"
        # Q4 == "In what country do you currently live?"
        born_col = 'Q3'
        reside_col = 'Q4'
        session_col = 'V3'

        # skip the first line with the question text
        next(reader)
    else:
        print('New format detected...', file=sys.stderr)

        born_col = 'Country Of Birth'
        reside_col = 'Country Of Current Residence'
        session_col = 'Session User Id'

    col = reside_col if args.reside else born_col
    for row in reader:
        if re.search(args.country, row[col], re.IGNORECASE):
            print(row[session_col])

print('Done!', file=sys.stderr)
