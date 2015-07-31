import copy
import os

import models
import sources
import timeline
import utils


api = timeline.api.TimelineApi(os.environ['PEBBLE_TIMELINE_API_KEY'])

def subscribe(user, topics):
    series = models.Series(_id__in=topics)
    user.update(add_to_set__subscriptions=series)
    for series in series:
        topic = str(series._id)
        if utils.is_crunchyroll_source(series):
            topic += '-%s' % ('premium' if user.crunchyroll_premium else 'free')
        api.subscribe(user.token, topic)
    user.save()
    user.reload()

def unsubscribe(user, topics):
    series = models.Series(_id__in=topics)
    for series in series:
        user.update(pull__subscriptions=series)
    user.save()
    user.reload()

def _send_pin(topic, pin):
    try:
        pin.validate()
        serialized = pin.json()
        api.send_shared_pin([topic], serialized)
    except timeline.fields.ValidationException:
        return False
    return True

def send_pin(media):
    pin = utils.create_pin_for_media_object(media)
    topic = str(media.series._id)
    if utils.is_crunchyroll_source(media):
        # We have to send a pin for both for the free and premium versions
        # of this data source, so we'll send the free one here as a copy.
        _topic = topic + '-free'
        topic += '-premium'
        _pin = copy.deepcopy(pin)
        _pin.id = str(media._id) + '-free'
        # TODO: Change the time to the Free Available time
        _send_pin(_topic, _pin)
    return _send_pin(topic, pin)
