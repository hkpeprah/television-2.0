import re
from datetime import datetime

import pytz
import tzlocal

from client import FunimationApi


API = FunimationApi()

def list_latest_episodes(limit=10):
    series_list = API.get_latest(limit=limit)
    simulcast_data = { 'series': [], 'episodes': [] }
    epoch = datetime(1970, 1, 1, tzinfo=pytz.utc)

    for series_metadata in series_list:
        series = dict()
        extra_data = {}

        episodes = API.get_episodes_for_series(series_metadata)

        series['name'] = series_metadata['series_name']
        series['description'] = series_metadata['series_description']
        series['genres'] = series_metadata['genres'].split(',') if series_metadata['genres'] else []
        series['image'] = series_metadata['thumbnail_medium'] if 'thumbnail_medium' in series_metadata else \
            series_metadata['poster_art']

        if len(episodes) > 0:
            # Runtime is stored as seconds as opposed to minutes in the source data structure
            series['runtime'] = int(episodes[0]['duration'] / 60)

        # Populate the extra data fields with Funimation specific information
        series['extra_data'] = {}
        series['extra_data']['pubDate'] = series_metadata['pubDate']
        series['extra_data']['url'] = series_metadata['link']
        series['extra_data']['series_id'] = series_metadata['asset_id']

        simulcast_data['series'].append(series)

        latest_free_video = series_metadata['latest_video_free'] or {}
        latest_free_video_id = latest_free_video.get('video_id')

        latest_premium_video = series_metadata['latest_video_subscription'] or {}
        latest_premium_video_id = latest_premium_video.get('video_id')

        premium_in_episode_list = False
        free_in_episode_list = False

        for episode_metadata in episodes:
            episode = dict()

            episode['name'] = episode_metadata['title']
            episode['season'] = episode_metadata.get('season_number', 1)
            episode['image'] = episode_metadata.get('thumbnail_medium', '')
            episode['number'] = episode_metadata['number']
            episode['summary'] = episode_metadata.get('description', '')
            episode['runtime'] = int(episode_metadata['duration'] / 60)
            episode['timestamp'] = 0

            video_data = None
            if episode_metadata['asset_id'] == latest_premium_video_id:
                video_data = latest_premium_video
                premium_in_episode_list = True
            elif episode_metadata['asset_id'] == latest_free_video_id:
                video_data = latest_free_video
                free_in_episode_list = True

            if video_data is not None:
                timestamp = int(video_data['release_date'])
                datetime_local = datetime.fromtimestamp(timestamp, tzlocal.get_localzone())
                datetime_utc = datetime_local.astimezone(pytz.utc)
                episode['timestamp'] = int((datetime_utc - epoch).total_seconds())

            episode['extra_data'] = {}
            episode['extra_data']['media_id'] = episode_metadata['asset_id']
            episode['extra_data']['series_name'] = episode_metadata['show_name']
            episode['extra_data']['series_id'] = series_metadata['asset_id']
            episode['extra_data']['video_url'] = episode_metadata['video_url']
            episode['extra_data']['extended_title'] = episode_metadata.get('extended_title', '')
            episode['extra_data']['funimation_id'] = episode_metadata['funimation_id']
            episode['extra_data']['language'] = episode_metadata['language']

            simulcast_data['episodes'].append(episode)

        for (data, found) in zip([latest_free_video, latest_premium_video],
                                 [free_in_episode_list, premium_in_episode_list]):
            if found or len(data.keys()) == 0 or len(episodes) < 1:
                continue
            free = data == latest_free_video
            episode = dict()

            timestamp = int(data['release_date'])
            datetime_local = datetime.fromtimestamp(timestamp, tzlocal.get_localzone())
            datetime_utc = datetime_local.astimezone(pytz.utc)
            episode['timestamp'] = int((datetime_utc - epoch).total_seconds())

            patt = 'Episode ([0-9]+?)'
            result = re.match(patt, data['title'])
            if result is None:
                continue

            episode['name'] = data['title']
            episode['season'] = episodes[-1].get('season_number', 1)
            episode['image'] = ''
            episode['number'] = result.group(1)
            episode['summary'] = ''
            episode['runtime'] = int(episodes[-1]['duration'] / 60)

            episode['extra_data'] = {}
            episode['extra_data']['premium'] = not free
            episode['extra_data']['series_name'] = series_metadata['series_name']
            episode['extra_data']['media_id'] = data['video_id']
            episode['extra_data']['series_id'] = series_metadata['asset_id']

            simulcast_data['episodes'].append(episode)

    return simulcast_data

if __name__ == '__main__':
    import json

    print json.dumps(list_latest_episodes(), indent=4, sort_keys=True)
