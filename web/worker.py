import datetime
import time
import sys

import mongoengine

import db
import telebble.constants
import telebble.fetch


def main():
    now = datetime.datetime.now()
    seconds_in_a_day = telebble.constants.SECONDS_IN_A_DAY
    sys.stdout.write('Worker: Starting up.\n')
    mongoengine.connect(db.DATABASE_NAME, alias=db.DATABASE_ALIAS)
    # We always scrape ahead 7 days for the television source as it provides us
    # per day incoming show data.
    for _ in range(6):
        telebble.fetch.fetch_from_television(now.year, now.month, now.day)
        now += datetime.timedelta(days=1)

    sys.stdout.write('Worker: Finished initialization.\n')
    while True:
        sys.stdout.write('Worker: Fetching new data.\n')
        telebble.fetch.fetch_from_television(now.year, now.month, now.day)
        telebble.fetch.fetch_from_crunchyroll()
        telebble.fetch.fetch_from_funimation()
        sys.stdout.write('Worker: Finished fetching.\n')
        sys.stdout.flush()
        time.sleep(seconds_in_a_day)
        now += datetime.timedelta(days=1)

if __name__ == '__main__':
    main()
