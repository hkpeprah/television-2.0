import datetime
import logging
import time

import mongoengine

import db
import telebble.constants
import telebble.fetch


def main():
    now = datetime.datetime.now()
    seconds_in_a_day = telebble.constants.SECONDS_IN_A_DAY
    logging.debug('Worker: Starting up.')
    mongoengine.connect(db.DATABASE_NAME, alias=db.DATABASE_ALIAS)
    # We always scrape ahead 7 days for the television source as it provides us
    # per day incoming show data.
    for _ in range(6):
        telebble.fetch.fetch_from_television(now.year, now.month, now.day)
        now += datetime.timedelta(days=1)

    logging.debug('Worker: Finished initialization.')

    while True:
        logging.debug('Worker: Fetching new data.')
        telebble.fetch.fetch_from_television(now.year, now.month, now.day)
        telebble.fetch.fetch_from_crunchyroll()
        telebble.fetch.fetch_from_funimation()
        logging.debug('Worker: Finished fetching.')
        time.sleep(seconds_in_a_day / 2)
        now += datetime.timedelta(days=1)

if __name__ == '__main__':
    main()
