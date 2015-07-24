from client import CrunchyrollApi
import dateutil.parser


API = CrunchyrollApi()


def list_latest_episodes(num_series=10):
    series_list = API.get_latest_episodes(limit=num_series)
    simulcast_data = { 'series': [], 'episodes': [] }
    for series_metadata in series_list:
        episodes = series_metadata['episodes']
        series = dict()

        series['name'] = series_metadata.get('name', None)
        series['description'] = series_metadata.get('description', None)
        series['genres'] = [series_metadata['media_type']] if 'media_type' in series_metadata else []
        series['image'] = series_metadata.get('portrait_image', {}).get('full_url', None)

        # TODO: Figure out what the runtime of the series are.
        series['runtime'] = None

        # Populate hte extra data fields with Crunchyroll specific information.
        series['extra_data'] = {}
        series['extra_data']['url'] = series_metadata.get('url', None)
        series['extra_data']['series_id'] = series_metadata.get('series_id', None)

        simulcast_data['series'].append(series)

        for episode_metadata in episodes:
            episode = dict()
            if not 'available_time' in episode_metadata:
                continue

            datestring = episode_metadata.get('available_time')
            date = dateutil.parser.parse(datestring)

            episode['name'] = episode_metadata.get('name', None)
            episode['date'] = date.strftime('%Y-%m-%d')

            # While anime does have seasons, they typically re-release the series under a new
            # name as opposed to having a new season.  Or they just tack it on under the same
            # name, either way, we don't really care.
            episode['season'] = None

            episode['image'] = None
            if 'screenshot_image' in episode_metadata:
                screenshot = episode_metadata['screenshot_image']
                if screenshot is not None:
                    episode['image'] = screenshot.get('full_url', None)

            if not 'episode_number' in episode_metadata:
                continue

            episode['number'] = int(episode_metadata.get('episode_number'))
            episode['summary'] = episode_metadata.get('description', None)
            episode['time'] = date.strftime('%H:%M')
            episode['timestamp'] = datestring

            # Right now we don't have an idea of how long the episodes are, so we can't make an
            # educated guess.
            episode['runtime'] = None

            episode['extra_data'] = {}
            extra_fields = [
                'premium_available_time',
                'premium_available',
                'free_available_time',
                'free_available',
                'series_id',
                'media_id'
            ]

            for field in extra_fields:
                episode['extra_data'][field] = episode_metadata[field]

            simulcast_data['episodes'].append(episode)

    return simulcast_data


if __name__ == '__main__':
    import json

    data = list_latest_episodes()
    print json.dumps(data, indent=4, sort_keys=True)
