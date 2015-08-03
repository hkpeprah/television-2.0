import copy
import logging
import os

from common import api
import models
import sources
import timeline.api
import timeline.fields
import utils


def subscribe(user, topics):
    series = models.Series.objects(_id__in=topics)
    ids = map(lambda s: s._id, series)
    for series in series:
        topic = str(series._id)
        if utils.is_crunchyroll_source(series):
            topic += '-%s' % ('premium' if user.crunchyroll_premium else 'free')
        elif utils.is_funimation_source(series):
            topic += '-%s' % ('premium' if user.funimation_premium else 'free')
        status, result = api.subscribe(user.token, topic)
        if not status:
            logging.error(result)
            return False

    user.update(add_to_set__subscriptions=ids)
    user.save()
    user.reload()

    return True

def unsubscribe(user, topics):
    series = models.Series.objects(_id__in=topics)
    ids = map(lambda s: s._id, series)
    for (topic, series) in zip(ids, series):
        status, result = api.unsubscribe(user.token, topic)
        if not status:
            return False
        user.update(pull__subscriptions=series)

    user.save()
    user.reload()

    return True
