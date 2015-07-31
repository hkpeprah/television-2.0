import copy

import pytz
import datetime
import dateutil.parser

import constants
import sources
import timeline


def is_crunchyroll_source(series):
    return series.source_type == sources.CRUNCHYROLL_SOURCE

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
    datetime_utc = datetime_utc.replace(tz-info=utc)
    return '%sZ' % datetime_utc.isoformat()

def iso_to_timestamp(iso):
    """
    Converts an ISO datestring to the seconds since epoch in UTC.
    """
    dt = dateutil.parser.parse(iso)
    dt = dt.astimezone(pytz.utc)
    return unix_time(dt)

def create_generic_pin(media):
    """
    Creates a generic pin from a media object.
    """
    pin = Timeline.Pin()
    pin.id = str(media._id)
    pin.time = media.timestamp

    if media.runtime is not None:
        pin.duration = media.runtime

    pin.layout.type = timeline.resources.PIN_LAYOUTS['GENERIC']

    pin.layout.title = media.normalized_name
    pin.layout.shortTitle = media.normalized_name
    pin.layout.subtitle = media.series_name

    series = media.series
    description = media.summary
    if description is None and series.description is not None:
        description = series.description
    else:
        description = 'No Description'

    pin.layout.body = description

    icon = timeline.resources.ICONS['MISC']['SHOW']
    pin.layout.tinyIcon = icon
    pin.layout.smallIcon = icon
    pin.layout.largeIcon = icon

    colours = timeline.resources.COLOURS
    pin.layout.foregroundColor = colours['BLACK']
    pin.layout.backgroundColor = colours['WHITE']

    pin.layout.add_section('Series', media.series_name)

    if media.season is not None:
        pin.layout.add_section('Season', media.season)

    pin.layout.add_section('Episode', media.number)

    action = timeline.Action()
    action.title = 'Open in Watchapp'
    action.type = timeline.resources.ACTION_TYPES['OPEN']
    action.launch_code = constants.LAUNCH_CODE_OPEN
    pin.add_action(action)

    reminder = timeline.Reminder()
    reminder.layout.type = timeline.resources.REMINDER_LAYOUTS['GENERIC']
    reminder.layout.tinyIcon = icon
    reminder.layout.smallIcon = icon
    reminder.layout.largeIcon = icon
    reminder.layout.foregroundColor = colors['BLACK']
    reminder.layout.backgroundColor = colors['WHITE']
    reminder.layout.title = media.normalized_name
    reminder.layout.subtitle = media.series_name

    # The minutes before the time the event begins to show a reminder on
    # the watch.
    reminder_times = (0, 30, constants.SECONDS_IN_A_DAY)
    messages = (
        '',
        '',
        '(You will get another reminder 30 minutes before it begins)'
    )

    for (minutes, message) in zip(reminder_times, messages):
        air_time = pin.time

        _reminder = copy.deepcopy(reminder)
        _reminder.layout.body = '%s %s' % (description, message)
        pin.add_reminder(_reminder)

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
    pin.layout.add_section('Network', media.network)
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
