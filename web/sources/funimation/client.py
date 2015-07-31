import requests
import simplejson
import urlparse


class FunimationApiError(Exception):
    pass

class FunimationApi(object):
    API_URL = 'https://www.funimation.com/'
    USER_AGENT = 'Sony-PS3'

    def __init__(self, username=None, password=None, cookiefile=None):
        super(FunimationApi, self).__init__()
        self.api_url = self.API_URL
        self.headers = { 'USER-AGENT': self.USER_AGENT }
        self.user_type = 'FunimationUser'

    def get_shows(self, limit=100, offset=0, sort=None, filter=None, first_letter=None):
        query = self.build_query(limit=limit, offset=offset, sort=sort, filter=filter)
        return self.get('feeds/ps/shows', query)

    def get_latest(self, limit=100, offset=0, premium=True):
        sort = None
        if premium:
            sort = 'SortOptionLatestSubscription'
        else:
            sort = 'SortOptionLatestFree'
        return self.get_shows(limit, offset, sort)

    def get_simulcasts(self, limit=100, offset=0):
        return self.get_shows(limit, offset, filter='FilterOptionSimulcast')

    def get_featured(self, limit=100, offset=0):
        query = self.build_query(limit=limit, offset=offset)
        return self.get('feeds/ps/featured', query)

    def get_videos(self, show_id, limit=100, offset=0):
        query = self.build_query(limit=limit, offset=offset, show_id=show_id)
        return self.get('feeds/ps/videos', query)

    def get_episodes_for_series(self, series, offset=0, media_limit=1000):
        data = self.get_videos(series['asset_id'], limit=media_limit, offset=offset)
        return data['videos']

    def get_latest_episodes(self, limit=100, offset=0, media_limit=100):
        newest = self.get_latest(limit=limit)
        for new_series in newest:
            data = self.get_videos(new_series['asset_id'], limit=media_limit)
            new_series['episodes'] = data['videos']
        return newest

    def search(self, search):
        query = self.build_query(**search)
        return self.get('feeds/ps/search', query)

    def get(self, endpoint, params):
        url = urlparse.urljoin(self.api_url, endpoint)
        r = requests.get(url, params=params, headers=self.headers)
        if r.status_code == 200:
            try:
                return r.json()
            except ValueError:
                raise FunimationApiError('Failed to parse request json')
        raise FunimationApiError('Error in request: %s (status code %s)' % (r.text, r.status_code))

    def build_query(self, **params):
        params['first-letter'] = params.pop('first_letter', None)
        params.setdefault('ut', self.user_type)
        return params
