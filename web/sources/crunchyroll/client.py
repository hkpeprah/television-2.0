# Inspired by:
# https://github.com/aheadley/python-crunchyroll
import requests


class CrunchyrollApiError(Exception):
    pass

class CrunchyrollApi(object):
    API_ROOT = 'https://crunchyroll.com'
    API_VERSION = '0'
    API_URL = 'https://api.crunchyroll.com/{}.{}.json'
    ACCESS_TOKEN = 'Scwg9PRRZ19iVwD'

    # Defaults to limit the number of returned results
    MAX_SERIES = 100

    # This information is used to spoof the user agent sent when making the request
    # to the Crunchyroll servers.
    DEVICE_MANUFACTURER = 'unknown'
    DEVICE_MODEL = 'google_sdk'
    DEVICE_PRODUCT = 'google_sdk'
    USER_AGENT = 'Dalvik/1.6.0 (Linux; U; Android 4.2; google_sdk Build/JB_MR1)'

    # This information is used to make it seem like this request is coming from
    # the Crunchyroll Android application.
    DEVICE_ID  = '00000000-18c4-ade8-ffff-ffff99d603a9'
    SDK_VERSION = '17'
    RELEASE_VERSION = '4.2'
    APP_CODE = '66'
    APP_VERSION_NAME = '0.7.9'
    APP_PACKAGE = 'com.crunchyroll.crunchyroid'

    PREMIUM_FLAG_ANIME = 'anime'
    PREMIUM_FLAG_DRAMA = 'drama'
    PREMIUM_FLAG_TAISENG = 'taiseng'

    # Types of media (flags)
    MEDIA_TYPE_ANIME = 'anime'
    MEDIA_TYPE_DRAMA = 'drama'

    # Filter flags
    FILTER_POPULAR = 'popular'
    FILTER_SIMULCAST = 'simulcast'
    FILTER_UPDATED = 'updated'
    FILTER_ALPHA = 'alpha'
    FILTER_NEWEST = 'newest'
    FILTER_PREFIX = 'prefix:'
    FILTER_TAG = 'tag'
    # When listing media, must use one of these additional filters
    # to sort the results.
    FILTER_ASC = 'asc'
    FILTER_DESC = 'desc'

    class FIELDS:
        # Database fields that can be added to the query to only return the particular
        # set of fields in the result set that you care about.
        GENERAL_MOST_LIKELY_MEDIA = 'most_likely_media'
        GENERAL_ORDERING = 'ordering'
        MEDIA_ID = 'media.media_id'
        MEDIA_NAME = 'media.name'
        MEDIA_EPISODE_NUMBER = 'media.episode_number'
        MEDIA_SCREENSHOT_IMAGE = 'media.screenshot_image'
        MEDIA_STREAM_DATA = 'media.stream_data'
        MEDIA_FREE_AVAILABLE = 'media.free_available'
        MEDIA_PREMIUM_AVAILABLE = 'media.premium_available'
        MEDIA_AVAILABILITY_NOTES = 'media.availablity_notes'
        MEDIA_PLAYHEAD = 'media.playhead'
        MEDIA_TYPE = 'media.media_type'
        MEDIA_DESCRIPTION = 'media.description'
        SERIES = 'series'
        SERIES_ID = 'series.series_id'
        SERIES_NAME = 'series.name'
        SERIES_PUBLISHER_NAME = 'series.publisher_name'
        SERIES_YEAR = 'series.year'
        SERIES_SCREENSHOT_IMAGE = 'series.screenshot_image'
        SERIES_LANDSCAPE_IMAGE = 'series.landscape_image'
        SERIES_PORTRAIT_IMAGE = 'series.portrait_image'
        SERIES_MEDIA_COUNT = 'series.media_count'
        SERIES_MEDIA_TYPE = 'series.media_type'
        SERIES_IN_QUEUE = 'series.in_queue'
        SERIES_DESCRIPTION = 'series.description'
        IMAGE_FWIDE_URL = 'image.fwide_url'
        IMAGE_WIDESTAR_URL = 'image.widestar_url'
        IMAGE_LARGE_URL = 'image.large_url'
        IMAGE_FULL_URL = 'image.full_url'

    def __init__(self, username=None, password=None):
        pass

    def get_anime_series(self, **kwargs):
        kwargs.setdefault('limit', self.MAX_SERIES)
        kwargs.setdefault('sort', self.FILTER_ALPHA)
        kwargs.setdefault('media_type', self.MEDIA_TYPE_ANIME)
        kwargs.setdefault('offset', 0)
        return self.get('list_series', kwargs)

    def get_newest(self, **kwargs):
        kwargs['sort'] = self.FILTER_NEWEST
        return self.get_anime_series(**kwargs)
    
    def get_simulcasts(self, **kwargs):
        kwargs['sort'] = self.FILTER_SIMULCAST
        return self.get_anime_series(**kwargs)

    def list_media(self, show_data, **kwargs):
        kwargs.setdefault('limit', self.MAX_SERIES)
        kwargs.setdefault('sort', self.FILTER_DESC)
        kwargs.setdefault('offset', 0)
        kwargs.setdefault('media_type', self.MEDIA_TYPE_ANIME)
        kwargs.setdefault('filter', 'contains:' + show_data['name'])
        if 'series_id' in show_data:
            kwargs['series_id'] = show_data['series_id']
        elif 'collection_id' in show_data:
            kwargs['collection_id'] = show_data['collection_id']
        return self.get('list_media', kwargs)

    def get_latest_episodes(self, **kwargs):
        newest = self.get_newest()
        kwargs.setdefault('limit', 10)
        for new_series in newest:
            new_series['episodes'] = self.list_media(new_series, **kwargs)
        return newest

    def get(self, endpoint, params):
        request_url = self.API_URL.format(endpoint, self.API_VERSION)
        base_params = {
            'locale': 'en',
            'device_id': self.DEVICE_ID,
            'device_type': self.APP_PACKAGE,
            'access_token': self.ACCESS_TOKEN,
            'version': self.APP_CODE
        }
        params.update(base_params)
        r = requests.get(request_url, params=params)
        if r.status_code == 200:
            request_data = r.json()
            success = request_data['code'] == 'ok'
            if success is True:
                return request_data['data']
        raise CrunchyrollApiError('Error in request: %s (status code %s)' % (r.text, r.status_code)) 
