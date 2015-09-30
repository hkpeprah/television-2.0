import logging

from mongoengine import Document, fields, queryset_manager, signals

import sources
import utils


__version__ = '0.01'

class Serializable(object):
    """
    Inherited from a subclass to provide it with a .json method for serialization
    from the API.
    """
    def json(self):
        serialized = {}
        serializable_fields = self.serializable_fields or []
        for _field in serializable_fields:
            field = _field.strip('_')
            value = getattr(self, _field, None)
            if hasattr(value, 'json'):
                value = value.json()
            serialized[field] = value
        return serialized

class Network(Serializable, Document):
    """
    A Network acts a parent to various different series media objects. For
    subscription-based services, the network is considered to be the hosting
    service that provides access to the media (E.g. Crunchyroll or Hulu).
    For simplicity, we don't make a distinction between networks based on
    the type of content that they show.
    """
    NETWORK_TYPES = (
        'None',
        'Subscription',
        'Movie'
    )

    serializable_fields = (
        '_id',
        '_type',
        'name',
        'description',
        'timezone',
        'country'
    )

    _id = fields.SequenceField(primary_key=True)
    _type = fields.StringField(choices=NETWORK_TYPES, required=True)
    name = fields.StringField(required=True)
    description = fields.StringField(required=False)
    series = fields.ListField(fields.ReferenceField('Series'))

    # The timezone is a required field unless the type of Network is not None.
    # This allows us to know the timezone in which the media objects are airing
    # to show the user.
    timezone = fields.StringField(required=False)

    # Similar to timezone, this is only a required field if the type of network
    # is not None.
    country = fields.StringField(required=False)

    # Used for uniqueness to prevent duplication of Networks.  We do a search on this
    # these fields together to determine if a Network already exists for our source.
    source_id = fields.IntField(required=False)
    source_type = fields.StringField(choices=sources.DATA_SOURCES, required=True)

    @queryset_manager
    def objects(doc_cls, queryset):
        return queryset.order_by('+name')

    def json(self):
        serialized = super(Network, self).json()
        serialized['series_count'] = len(self.series)
        return serialized

class Series(Serializable, Document):
    """
    A series is a collection of media objects that all belong to the same show/franchise/etc.
    Whenever a new media object for the series is detected, it will be added to the existing
    series, provided it exists, otherwise a new series will be created.  To ensure that we
    avoid duping data, we use the `series_id` of the source to perform a query for
    uniqueness.
    """
    serializable_fields = (
        '_id',
        'name',
        'description',
        'genres',
        'image',
        'runtime',
        'network',
        'latest'
    )

    _id = fields.SequenceField(primary_key=True)
    name = fields.StringField(required=True)
    description = fields.StringField(required=True)
    genres = fields.ListField(fields.StringField(), default=[])
    image = fields.StringField(required=False)
    runtime = fields.IntField(required=False)
    media = fields.ListField(fields.ReferenceField('Media'), default=[])

    # Back reference to the Network, should be added when the model is created to prevent
    # additional query requests against the database.
    network = fields.ReferenceField('Network', required=True)

    # We use this information to check for uniqueness and for possible queries for more
    # information about the series in particular.
    source_id = fields.IntField(required=True)
    source_type = fields.StringField(choices=sources.DATA_SOURCES, required=True)

    # Extra data
    extra_data = fields.DictField(default={})

    @property
    def topic(self):
        prefix = '{}-'.format(__version__)
        return prefix + str(self._id)

    @property
    def country(self):
        return self.network.country

    @property
    def latest(self):
        if len(self.media) == 0:
            return None
        ordered = sorted(self.media, key=lambda m: m.timestamp, reverse=True)
        return ordered[0]

    @queryset_manager
    def objects(doc_cls, queryset):
        return queryset.order_by('+name')

    def json(self):
        serialized = super(Series, self).json()
        serialized['episode_count'] = len(self.media)
        return serialized

class Media(Serializable, Document):
    """
    Represents an Episode, Movie, etc.
    """
    serializable_fields = (
        '_id',
        'name',
        'summary',
        'image',
        'season',
        'number',
        'runtime',
        'series_id',
        'series_name',
        'network_id',
        'network_name',
        'timestamp',
        'air_date'
    )

    _id = fields.SequenceField(primary_key=True)
    name = fields.StringField(required=False, default='')
    summary = fields.StringField(default='')
    image = fields.StringField(required=False)
    season = fields.IntField(required=False)
    number = fields.IntField(required=True)
    timestamp = fields.IntField(required=True)
    _runtime = fields.IntField(required=False)

    # Back reference to the Series, should be added when the modal is created to prevent
    # additional query requests against the database.
    series = fields.ReferenceField('Series', required=True)

    # We use this information to check for uniqueness and for possible queries for more
    # information about the series in particular.
    source_id = fields.IntField(required=False)

    # Extra data
    extra_data = fields.DictField(default={})

    @property
    def air_date(self):
        if self.timestamp is not None:
            return utils.timestamp_to_iso(self.timestamp)
        return None

    @property
    def country(self):
        return self.series.country

    @property
    def series_id(self):
        return self.series._id

    @property
    def series_name(self):
        return self.series.name

    @property
    def source_type(self):
        return self.series.source_type

    @property
    def network(self):
        return self.series.network

    @property
    def network_id(self):
        return self.network._id

    @property
    def network_name(self):
        return self.network.name

    @property
    def runtime(self):
        """
        Either this media object has a runtime or we default to the runtime that the series
        says its media have.  We do this because sometimes a particular episode of a show
        may have a special with a different runtime than normal.
        """
        if self._runtime is not None:
            return self._runtime
        return self.series.runtime

    @property
    def normalized_name(self):
        if len(self.name) > 0:
            return self.name
        return 'Episode %s' % self.number

    @queryset_manager
    def objects(doc_cls, queryset):
        return queryset.order_by('+name')

    @classmethod
    def post_save(cls, sender, document, **kwargs):
        logging.debug('Sending pin for: %s' % document)
        pin = utils.create_pin_for_media_object(document)
        status = utils.send_pin(document)

    def __str__(self):
        return '<Media %s - %s>' % (self.series_name, self.normalized_name)

signals.post_save.connect(Media.post_save, sender=Media)

class User(Serializable, Document):
    """
    Represents a Pebble Time user.
    """
    serializable_fields = (
        '_id',
        'token',
        'country',
        'crunchyroll_premium',
        'funimation_premium',
        'num_subscriptions'
    )

    _id = fields.SequenceField(primary_key=True)
    token = fields.StringField(unique=True)
    subscriptions = fields.ListField(fields.ReferenceField('Series'))

    # Purely for tracking purposes, doesn't mean anything
    country = fields.StringField(default='US')

    # If this flag is true, then we subscribe to the premium version of series.
    # Note: When this changes, we have to unsubscribe the user from all their
    # previous subscriptions to the premium channel.
    crunchyroll_premium = fields.BooleanField(default=True)
    # TODO: Use this in a meaningful way
    funimation_premium = fields.BooleanField(default=False)

    @property
    def num_subscriptions(self):
        return len(self.subscriptions)

    def json(self):
        serialized = super(User, self).json()
        serialized['subscriptions'] = map(lambda s: s._id, self.subscriptions)
        return serialized
