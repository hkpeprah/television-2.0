import datetime

import constants
import models
import sources
import utils


def fetch_from_funimation():
    data = sources.funimation.list_latest_episodes()
    series = data.get('series', [])
    episodes = data.get('episodes', [])

    source_type = sources.FUNIMATION_SOURCE

    network = models.Network.objects(source_type=source_type).first()
    if network is None:
        network = models.Network()
        network._type = 'Subscription'
        network.name = 'Funimation'
        network.description = 'You Should be Watching'
        network.country = 'US'
        network.source_type = source_type
        network.save()

    for series_data in series:
        extra_data = series_data.get('extra_data', {})
        _id = extra_data.get('series_id')
        query = models.Series.objects(source_id=_id, source_type=source_type)
        if query.first() is not None:
            continue

        series = models.Series()

        series.name = series_data['name']
        series.description = series_data['description'] or ''
        series.genres = series_data['genres']
        series.image = series_data['image'] or ''
        series.runtime = series_data['runtime']

        series.network = network

        series.source_id = _id
        series.source_type = source_type
        series.extra_data = extra_data

        series.save()

        network.update(add_to_set__series=[series])
        network.save()

    for episode_data in episodes:
        extra_data = episode_data.get('extra_data', {})
        series_id = extra_data.get('series_id')

        _id = extra_data.get('media_id')
        query = models.Media.objects(source_id=_id)
        query = filter(lambda m: m.source_type == source_type, query)
        media = query[0] if len(query) > 0 else models.Media()

        media.name = episode_data['name']
        media.summary = episode_data['summary']
        media.image = episode_data['image']
        media.season = int(episode_data['season'])
        media.number = int(episode_data['number'])
        media.timestamp = episode_data['timestamp']
        media._runtime = episode_data['runtime']

        series = models.Series.objects(source_type=source_type, source_id=series_id).first()
        media.series = series

        media.source_id = _id
        media.extra_data = extra_data

        media.save()

        series.update(add_to_set__media=[media])
        series.save()

def fetch_from_crunchyroll():
    data = sources.crunchyroll.list_latest_episodes(100)
    episodes = data.get('episodes', [])
    series = data.get('series', [])

    source_type = sources.CRUNCHYROLL_SOURCE

    network = models.Network.objects(source_type=source_type).first()
    if network is None:
        network = models.Network()
        network._type = 'Subscription'
        network.name = 'Crunchyroll'
        network.description = 'Official, legal streaming anime videos including Naruto Shippuden, ' + \
            'Attack on Titan, Sword Art Online, Skip Beat, and Shugo Chara. Start watching now.'
        network.country = 'US'
        network.source_type = source_type
        network.save()

    for series_data in series:
        extra_data = series_data.get('extra_data', {})
        _id = extra_data.get('series_id')
        query = models.Series.objects(source_id=_id, source_type=source_type)
        if query.first() is not None:
            continue

        series = models.Series()

        series.name = series_data['name']
        series.description = series_data['description'] or ''
        series.genres = series_data['genres']
        series.image = series_data['image'] or ''

        if series_data['runtime']:
            series.runtime = series_data['runtime']

        series.network = network

        series.source_id = _id
        series.source_type = source_type
        series.extra_data = extra_data

        series.save()

        network.update(add_to_set__series=[series])
        network.save()

    for episode_data in episodes:
        extra_data = episode_data.get('extra_data', {})
        series_id = extra_data.get('series_id')

        _id = extra_data.get('media_id')
        media_objects = models.Media.objects(source_id=_id)
        filtered_media_objects = filter(lambda m: m.source_type == source_type, media_objects)
        if len(filtered_media_objects) > 0:
            continue

        media = models.Media()

        media.name = episode_data['name']
        media.summary = episode_data['summary']
        media.image = episode_data['image'] or ''
        media.season = episode_data['season'] or 1
        media.number = episode_data['number']
        media.timestamp = utils.iso_to_timestamp(episode_data['timestamp'])

        if episode_data['runtime'] is not None:
            media._runtime = episode_data['runtime']

        series = models.Series.objects(source_type=source_type, source_id=series_id).first()
        media.series = series

        media.source_id = _id
        media.extra_data = extra_data

        media.save()

        series.update(add_to_set__media=[media])
        series.save()

def parse_television_data(data):
    source_type = sources.TELEVISION_SOURCE

    shows = data.get('shows', [])
    networks = data.get('networks', [])
    episodes = data.get('episodes', [])

    for network_data in networks:
        _id = network_data.get('extra_data', {}).get('id')
        query = models.Network.objects(source_id=_id, source_type=source_type)
        if query.first() is not None:
            continue

        network = models.Network()

        network._type = 'None'
        network.name = network_data['name']
        network.description = network_data.get('description', '')

        network.timezone = network_data['timezone']
        network.country = network_data['country_code']

        network.source_id = _id
        network.source_type = source_type

        network.save()

    for show_data in shows:
        extra_data = show_data.get('extra_data', {})
        _id = extra_data.get('id')
        network_id = extra_data.get('network_id')
        query = models.Series.objects(source_id=_id, source_type=source_type)
        if query.first() is not None:
            continue

        series = models.Series()

        series.name = show_data['name'] or ''
        series.description = show_data['description']
        series.genres = show_data['genres']
        series.image = show_data.get('image', '')
        series.runtime = show_data['runtime']

        network_query = models.Network.objects(source_id=network_id, source_type=source_type)
        network = network_query.first()
        series.network = network

        series.source_id = _id
        series.source_type = source_type

        series.extra_data = extra_data

        series.save()

        network.update(add_to_set__series=[series])
        network.save()

    for episode_data in episodes:
        extra_data = episode_data.get('extra_data', {})
        series_id = extra_data['series_id']
        network_id = extra_data['network_id']

        episode_number = episode_data['number']
        if episode_number is None:
            continue

        network = models.Network.objects(source_id=network_id, source_type=source_type).first()
        series = models.Series.objects(source_id=series_id, source_type=source_type).first()
        exists = False

        media = filter(lambda m: m.number == episode_number, series.media)
        if len(media) > 0:
            continue

        media = models.Media()

        media.name = episode_data['name'] or ''
        media.summary = episode_data['summary']
        media.image = episode_data['image'] or ''
        media.season = episode_data['season'] or 1
        media.number = episode_number
        media.timestamp = utils.iso_to_timestamp(episode_data['timestamp'])

        if episode_data['runtime'] is not None:
            media._runtime = int(episode_data['runtime'])

        media.series = series
        media.extra_data = extra_data

        media.save()

        series.update(add_to_set__media=[media])
        series.save()

def fetch_from_television(year, month, day):
    date = datetime.datetime(year, month, day)
    countries = constants.COUNTRY_CODES

    for country in countries:
        data = sources.television.list_latest_episodes_for_country(country, date)
        parse_television_data(data)

def search_from_television(query, limit=None):
    countries = constants.COUNTRY_CODES
    data = sources.television.search_for_series(query, countries, limit)
    parse_television_data(data)
