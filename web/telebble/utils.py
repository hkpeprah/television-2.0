import re
import json
import logging

import pytz
import datetime
import dateutil.parser

from common import api
import constants
import sources
import timeline


def is_crunchyroll_source(series):
    return series.source_type == sources.CRUNCHYROLL_SOURCE

def is_funimation_source(series):
    return series.source_type == sources.FUNIMATION_SOURCE

def unix_time(dt):
    """
    Converts a datetime object to the seconds since epoch.  Converts to
    UTC as a preventative measure.
    """
    dt = dt.astimezone(pytz.utc)
    epoch = datetime.datetime(constants.EPOCH_YEAR, constants.EPOCH_MONTH, constants.EPOCH_DAY, tzinfo=pytz.utc)
    return int((dt - epoch).total_seconds())

def timestamp_to_iso(timestamp):
    """
    Converts a timestamp representating seconds since epoch in UTC to an
    equivalent datetime object.
    """
    utc = pytz.utc
    datetime_utc = datetime.datetime.utcfromtimestamp(int(timestamp))
    datetime_utc = datetime_utc.replace(tzinfo=utc)
    return '%sZ' % datetime_utc.isoformat()

def iso_to_timestamp(iso):
    """
    Converts an ISO datestring to the seconds since epoch in UTC.
    """
    dt = dateutil.parser.parse(iso)
    dt = dt.astimezone(pytz.utc)
    return unix_time(dt)

def normalize_description(description):
    """
    Descriptions have a max length when passed over, so this utility function allows us
    to trim the description down to a set of sentences that fits within the length limit.
    """
    sentences = re.split('\.[\s$]', description)
    pieces = []
    description_length = 0
    max_length = 600
    for sentence in sentences:
        description_length += len(sentence)
        if description_length >= max_length:
            break
        pieces.append(sentence)
    return '. '.join(pieces)

def create_generic_pin(media):
    """
    Creates a generic pin from a media object.
    """
    pin = timeline.Pin()
    pin.id = str(media._id)
    pin.time = timestamp_to_iso(media.timestamp).replace('+00:00', '')

    if media.runtime is not None:
        pin.duration = media.runtime

    pin.layout.type = timeline.resources.PIN_LAYOUTS['GENERIC']

    pin.layout.title = media.normalized_name
    pin.layout.shortTitle = media.normalized_name
    pin.layout.subtitle = media.series_name

    series = media.series
    description = media.summary
    if description is None or len(description) == 0:
        if series.description is not None and len(series.description) > 0:
            description = normalize_description(series.description)
        else:
            description = 'No Description'
    else:
        description = normalize_description(description)

    pin.layout.body = description

    icon = timeline.resources.ICONS['MISC']['SHOW']
    pin.layout.tinyIcon = icon
    pin.layout.smallIcon = icon
    pin.layout.largeIcon = icon

    colours = timeline.resources.COLOURS
    pin.layout.foregroundColor = colours['BLACK']
    pin.layout.secondaryColor = colours['BLACK']
    pin.layout.backgroundColor = colours['WHITE']

    pin.layout.add_section('Series', media.series_name)

    if media.season is not None:
        pin.layout.add_section('Season', str(media.season))

    pin.layout.add_section('Episode', str(media.number))

    action = timeline.Action()
    action.title = 'Open in Watchapp'
    action.type = timeline.resources.ACTION_TYPES['OPEN']
    pin.add_action(action)

    # The minutes before the time the event begins to show a reminder on
    # the watch.
    reminder_times = (0, 30, 60 * 24)
    messages = (
        '',
        '',
        '(You will get another reminder 30 minutes before it begins)'
    )

    for (minutes, message) in zip(reminder_times, messages):
        reminder = timeline.Reminder()
        reminder.layout.type = timeline.resources.REMINDER_LAYOUTS['GENERIC']
        reminder.layout.tinyIcon = icon
        reminder.layout.smallIcon = icon
        reminder.layout.largeIcon = icon
        reminder.layout.foregroundColor = colours['BLACK']
        reminder.layout.backgroundColor = colours['WHITE']
        reminder.layout.title = media.normalized_name
        reminder.layout.subtitle = media.series_name
        reminder.layout.shortTitle = media.normalized_name

        air_time = media.timestamp - (minutes * constants.SECONDS_IN_A_MINUTE)
        reminder.time = timestamp_to_iso(air_time).replace('+00:00', '')
        reminder.layout.body = '%s %s' % (description, message)
        pin.add_reminder(reminder)

    return pin

def create_pin_for_crunchyroll(media):
    pin = create_generic_pin(media)
    pin.layout.add_section('Service', 'Crunchyroll')
    return pin

def create_pin_for_funimation(media):
    pin = create_generic_pin(media)
    pin.layout.add_section('Service', 'Funimation')
    return pin

def create_pin_for_television(media):
    pin = create_generic_pin(media)
    pin.layout.add_section('Network', media.network.name)
    pin.layout.add_section('Country', media.country)
    return pin

def create_pin_for_media_object(media):
    """
    Creates a pin for a media object and returns the pin.
    """
    if media.source_type == sources.CRUNCHYROLL_SOURCE:
        return create_pin_for_crunchyroll(media)
    elif media.source_type == sources.FUNIMATION_SOURCE:
        return create_pin_for_funimation(media)
    elif media.source_type == sources.TELEVISION_SOURCE:
        return create_pin_for_television(media)
    return None

def send_pin_for_topic(topic, pin):
    try:
        pin.validate()
        serialized = pin.json()
        status, result = api.send_shared_pin([topic], serialized)
        if not status:
            if len(result.content) > 0:
                to_json = json.loads(result.content)
                logging.error(json.dumps(to_json, indent=4, sort_keys=True))
        return status
    except timeline.fields.ValidationException as e:
        logging.error('Failed to validate pin: %s' % str(e))
        logging.error('%s\n' % json.dumps(pin.json(), indent=4, sort_keys=True))
        return False
    return True

def send_pin(media):
    now = unix_time(datetime.datetime.utcnow().replace(tzinfo=pytz.utc))
    if media.timestamp < (now - constants.SECONDS_IN_A_DAY):
        logging.debug('Skipping %s, too far in the past: %s' % (media, timestamp_to_iso(media.timestamp)))
        return False

    pin = create_pin_for_media_object(media)
    topic = str(media.series._id)
    if is_crunchyroll_source(media):
        # We have to send a pin for both for the free and premium versions
        # of this data source, so we'll send the free one here as a copy.
        free_topic = topic + '-free'
        free_pin = create_pin_for_media_object(media)
        free_pin.id = str(media._id) + '-free'
        free_pin.time = timestamp_to_iso(iso_to_timestamp(media.extra_data['free_available_time'])).replace('+00:00', '')
        send_pin_for_topic(free_topic, free_pin)
        pin.id = str(media._id) + '-premium'
        topic += '-premium'
    elif is_funimation_source(media):
        if media.extra_data['premium']:
            topic += '-premium'
            pin.id = str(media._id) + '-premium'
        else:
            topic += '-free'
            pin.id = str(media._id) + '-free'
    return send_pin_for_topic(topic, pin)
