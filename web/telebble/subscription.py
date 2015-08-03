import copy
import os

import models
import sources
import timeline.api
import timeline.fields
import utils


api = timeline.api.TimelineApi(os.environ['PEBBLE_TIMELINE_API_KEY'])

def subscribe(user, topics):
    series = models.Series.objects(_id__in=topics)
    ids = map(lambda s: s._id, series)
    user.update(add_to_set__subscriptions=ids)
    for series in series:
        topic = str(series._id)
        if utils.is_crunchyroll_source(series):
            topic += '-%s' % ('premium' if user.crunchyroll_premium else 'free')
        elif utils.is_funimation_source(series):
            topic += '-%s' % ('premium' if user.funimation_premium else 'free')
        api.subscribe(user.token, topic)
    user.save()
    user.reload()

def unsubscribe(user, topics):
    series = models.Series.objects(_id__in=topics)
    ids = map(lambda s: s._id, series)
    for (topic, series) in zip(ids, series):
        user.update(pull__subscriptions=series)
        api.unsubscribe(user.token, topic)
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
        _pin = copy.deepcopy(pin)
        _pin.id = str(media._id) + '-free'
        _pin.timestamp = utils.iso_to_timestamp(media.extra_data['free_available_time'])
        _send_pin(_topic, _pin)
        topic += '-premium'
    elif utils.is_funimation_source(media):
        if media.extra_data['premium']:
            topic += '-premium'
        else:
            topic += '-free'
    return _send_pin(topic, pin)
