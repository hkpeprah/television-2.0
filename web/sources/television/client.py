from datetime import datetime
import requests
import urlparse

from common import COUNTRY_CODES


class TeleivsionApiError(Exception):
    pass

class TelevisionApi(object):
    API_URL = 'http://api.tvmaze.com'

    # At least for the time being, lets consider these to be the only country
    # codes we care about based on users that would use this application.  Obviously
    # other countries have their own television networks too.
    COUNTRY_CODES = [
        'US',          # United States of America
        'CA',          # Canada
        'GB',          # United Kingdom
        'AU',          # Australia
        'CN',          # China
        'FR',          # France
        'DE',          # Germany
        'JP',          # Japan
        'KP',          # Democratic People's Republic of Korea
        'KR',          # Republic of Korea
        'MX'           # Mexico
    ]

    QUERY_CODES = [
        'country',   # One of the country codes listed above
        'date',      # Note that date is a formatted as YYYY-MM-DD
        'embed',
        'tvrage',
        'thetvdb'
    ]

    def __init__(self, *args, **kwargs):
        super(TelevisionApi, self).__init__()
        self.api_url = self.API_URL
        self.query_codes = self.QUERY_CODES
        self.headers = { 'USER-AGENT': 'Mozilla/5.0' }

    def get_latest(self, country_code='US', date=None):
        if date is None:
            i = datetime.now()
            date = '{}-{}-{}'.format(pad(i.year), pad(i.month), pad(i.day))
        params = {
            'country': country_code,
            'date': date
        }
        return self.get('schedule', params)

    def get_show(self, show_id):
        return self.get('shows/{}'.format(show_id))

    def get_episodes_for_show(self, show_id):
        return self.get('shows/{}/episodes'.format(show_id))

    def search(self, search):
        return self.get('search/shows', { 'q': search })

    def get(self, endpoint, params=None):
        url = urlparse.urljoin(self.api_url, endpoint)
        r = requests.get(url, params=params, headers=self.headers)
        if r.status_code == 200:
            return r.json()
        raise TelevisionApiError('Error in request: %s (status code %s)' % (r.text, r.status_code))


def pad(number, padding=1):
    """
    Consumes a number and returns it as a left zero-padded string.
    """
    if not isinstance(number, str):
        number = str(number)
    while len(number) <= padding:
        number = '0' + number
    return number

def get_country_long_name(country_code):
    """
    Returns the long name from a country code.
    Example: 'US' -> 'United States'
    """
    return COUNTRY_CODES.get(country_code, None)
