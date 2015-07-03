import json
import requests


class TimelineApi(object):
    API_ROOT = 'https://timeline-api.getpebble.com'
    API_VERSION = 'v1'

    ERROR_CODES = {
        '400': 'Pin object is invalid',
        '403': 'API key is invalid',
        '410': 'User token does not exist',
        '429': 'Rate limit exceeded',
        '500': 'Timeline service is experiencing difficulty',
        '503': 'Service current unavailable'
    }

    URLS = {
        'shared': '{}/{}/shared/pins/{}',
        'user': '{}/{}/user/pins/{}',
        'delete': '{}/{}/user/pins/{}',
        'subscribe': '{}/{}/user/subscription/{}',
        'unsubscribe': '{}/{}/user/subscription/{}'
    }

    def __init__(self, api_key=None, api_root=None, *args, **kwargs):
        self.api_key = api_key
        self.api_root = api_root if api_root is not None else self.API_ROOT
        self.api_version = self.API_VERSION

    def send_shared_pin(self, topics, pin):
        url = self.URLS['shared'].format(self.api_root, self.api_version, pin['id'])
        result = requests.put(url, headers={
            'X-API-Key': self.api_key,
            'X-Pin-Topics': ','.join(str(t) for t in topics),
            'Content-Type': 'application/json'
        }, data=json.dumps(pin))
        return (result.status_code == requests.codes.ok), result

    def send_user_pin(self, user_token, pin):
        url = self.URLS['user'].format(self.api_root, self.api_version, pin['id'])
        result = requests.put(url, headers={
            'X-User-Token': user_token,
            'Content-Type': 'application/json'
        }, data=json.dumps(pin))
        return (result.status_code == requests.codes.ok), result

    def delete_user_pin(self, user_token, pin):
        url = self.URLS['delete'].format(self.api_root, self.api_version, pin['id'])
        result = requests.delete(url, headers={
            'X-User-Token': user_token
        })
        return (result.status_code == requests.codes.ok), result

    def subscribe(self, user_token, topic):
        url = self.URLS['subscribe'].format(self.api_root, self.api_version, topic)
        result = requests.post(url, headers={
            'X-User-Token': user_token
        })
        return (result.status_code == requests.codes.ok), result

    def unsubscribe(self, user_token, topic):
        url = self.URLS['unsubscribe'].format(self.api_root, self.api_version, topic)
        result = requests.delete(url, headers={
            'X-User-Token': user_token
        })
        return (result.status_code == requests.codes.ok), result
