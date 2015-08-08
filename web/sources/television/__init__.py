import re
from datetime import datetime

from client import (TelevisionApi, pad)
from common import COUNTRY_CODES as country_codes


API = TelevisionApi()

def list_latest_episodes(*args, **kwargs):
    raise NotImplemented

def fetch_episodes_data_for_country(country, date):
    """
    Lists the new episodes for the specified country on the specified
    date.  Expects date to be a datetime object.
    """
    if not isinstance(date, datetime):
        raise TypeError('Expected date to be a datetime object')
    date = '{}-{}-{}'.format(pad(date.year), pad(date.month), pad(date.day))
    return API.get_latest(country, date)

def list_latest_episodes_for_country(country, date):
    data = fetch_episodes_data_for_country(country, date)
    networks, episodes, shows = [], [], []
    for episode_metadata in data:
        episode_mapping = {
            'name': 'name',
            'airdate': 'date',
            'season': 'season',
            'image': 'image',
            'number': 'number',
            'summary': 'summary',
            'airtime': 'time',
            'airstamp': 'timestamp',
            'runtime': 'runtime'
        }

        # Populate the episode data from the metadata contained within
        # the returned data set
        episode = dict()
        for (orig_field, new_field) in episode_mapping.iteritems():
            episode[new_field] = episode_metadata.get(orig_field, None)
            if new_field == 'image' and isinstance(episode[new_field], dict):
                episode[new_field] = episode[new_field].get('original', '')

        if episode['summary'] is not None:
            episode['summary'] = re.sub('<[^>]*>', '', episode['summary'])

        # Check if we have metadata to create a show, if so do that.
        show_metadata = episode_metadata.get('show', None)
        if show_metadata is not None:
            show_mapping = {
                'genres': 'genres',
                'name': 'name',
                'summary': 'description',
                'runtime': 'runtime'
            }

            show = dict()
            for (orig_field, new_field) in show_mapping.iteritems():
                show[new_field] = show_metadata.get(orig_field, None)

            if show['description'] is not None:
                show['description'] = re.sub('<[^>]*>', '', show['description'])

            if 'image' in show_metadata and show_metadata['image'] is not None:
                show['image'] = show_metadata['image'].get('original', None)

            show['extra_data'] = { 'id': show_metadata.get('id', None) }

            # If we dont' haev any genres assigned, then our genres are an
            # empty list.
            if 'genres' not in show:
                show['genres'] = []

            # We assume the type of show (e.g. 'Game Show') is also what would be
            # considered a genre.
            if 'type' in show_metadata and show_metadata['type'] is not None:
                show['genres'].append(show_metadata['type'])

            # Update the episode with the show information
            episode['series_name'] = show['name']
            episode['extra_data'] = { 'series_id': show['extra_data']['id'] }

            # Check if we have metadata to create the network object.
            network_metadata = show_metadata.get('network', None)
            if network_metadata is not None:
                network = dict()
                country = network_metadata.get('country', None)
                if country is not None:
                    network['country_code'] = country.get('code', None)
                    network['timezone'] = country.get('timezone', None)

                network['name'] = network_metadata.get('name', None)
                network['extra_data'] = { 'id': network_metadata.get('id', None) }

                # Update the show and episode with the network information
                episode['network_name'] = network['name']
                episode['extra_data']['network_id'] = network['extra_data']['id']
                episode['country_code'] = network.get('country_code', None)

                show['network_name'] = network['name']
                show['extra_data']['network_id'] = network['extra_data']['id']

                networks.append(network)

            # Push the show into the list
            shows.append(show)

        # Finally push the episode into the list
        episodes.append(episode)

    return { 'networks': networks, 'episodes': episodes, 'shows': shows }

def search_for_series(query, allowed_countries=None, limit=None):
    if allowed_countries is None:
        allowed_countries = API.COUNTRY_CODES

    data = API.search(query)
    shows, networks = [], []
    iteration = 0

    for result in data:
        if result is None or result.get('show', None) is None:
            continue

        if limit is not None and iteration >= limit:
            break

        iteration += 1

        show_mapping = {
            'genres': 'genres',
            'name': 'name',
            'summary': 'description',
            'runtime': 'runtime'
        }

        show_metadata = result.get('show')
        show = dict()

        for (orig_field, new_field) in show_mapping.iteritems():
            show[new_field] = show_metadata.get(orig_field, None)

        if show['description'] is not None:
            show['description'] = re.sub('<[^>]*>', '', show['description'])

        if 'image' in show_metadata and show_metadata['image'] is not None:
            show['image'] = show_metadata['image'].get('original', None)

        show['extra_data'] = { 'id': show_metadata.get('id', None) }
        show['extra_data']['url'] = show_metadata.get('url', None)

        # If we dont' haev any genres assigned, then our genres are an
        # empty list.
        if 'genres' not in show:
            show['genres'] = []

        # We assume the type of show (e.g. 'Game Show') is also what would be
        # considered a genre.
        if 'type' in show_metadata and show_metadata['type'] is not None:
            show['genres'].append(show_metadata['type'])

        # Check if we have metadata to create the network object.
        network_metadata = show_metadata.get('network', None)
        if network_metadata is not None:
            network = dict()
            country = network_metadata.get('country', None)
            country_code = None
            if country is not None:
                country_code = country.get('code', None)
                network['country_code'] = country.get('code', None)
                network['timezone'] = country.get('timezone', None)

            network['name'] = network_metadata.get('name', None)
            network['extra_data'] = { 'id': network_metadata.get('id', None) }

            # Update the show with the network information
            show['network_name'] = network['name']
            show['extra_data']['network_id'] = network['extra_data']['id']

            if country_code not in allowed_countries:
                continue

            networks.append(network)

        # Push the show into the list
        shows.append(show)

    return { 'networks': networks, 'episodes': [], 'shows': shows }


if __name__ == '__main__':
    import json
    data = search_for_series('Walking Dead')
    print json.dumps(data, indent=4, sort_keys=True)
