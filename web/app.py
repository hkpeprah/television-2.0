import math

from flask import Flask, request, render_template
from flask_restful import Api, Resource, reqparse

from telebble.models import Network, Series, Media, User
from telebble.subscription import subscribe, unsubscribe


app = Flask(__name__)
api = Api(app)
version = 'v1'

class BaseResource(Resource):
    LIMIT_PER_PAGE = 20

    def __init__(self, *args, **kwargs):
        super(BaseResource, self).__init__()
        self.parser = reqparse.RequestParser()
        self.parser.add_argument('page', type=int, location='args', required=False)

    def get(self, resource, singleton=False, limit_results=True, *args, **kwargs):
        page = request.args.get('page', 1)
        try:
            page = int(page)
        except ValueError:
            page = 1

        query = request.args.get('q', None)
        if query is not None:
            kwargs.update({ 'name__istartswith': query })

        objects = resource.objects(**kwargs)
        count = objects.count()

        limit = self.LIMIT_PER_PAGE if limit_results else count
        offset = (page - 1) * limit if limit_results else 0

        num_pages = int(math.ceil(count * 1.0 / max(limit, 1)))
        objects = objects.skip(offset).limit(limit)

        if singleton:
            obj = objects.first()
            return obj.json() if obj else {}

        serialized = map(lambda o: o.json(), objects)
        metadata = { 'current_page': page, 'num_pages': num_pages }
        return { 'count': len(serialized), 'items': serialized, 'metadata': metadata }

class NetworkResource(BaseResource):
    def get(self, network_id=None, **kwargs):
        singleton = False
        if network_id is not None:
            singleton = True
            kwargs.update({ '_id': network_id })
        return super(NetworkResource, self).get(Network, singleton, **kwargs)

class SeriesResource(BaseResource):
    def get(self, series_id=None, network_id=None, user_token=None, **kwargs):
        singleton = False
        limit_results = True
        if series_id is not None:
            singleton = True
            kwargs.update({ '_id': series_id })

        if user_token is not None:
            limit_results = False
            user = User.objects(token=user_token).first()
            subscriptions = user.subscriptions if user is not None else []
            kwargs.update({ '_id__in': map(lambda s: s._id, subscriptions) })

        if network_id is not None:
            kwargs.update({ 'network': network_id })

        return super(SeriesResource, self).get(Series, singleton, limit_results, **kwargs)

class MediaResource(BaseResource):
    def get(self, media_id=None, series_id=None, **kwargs):
        singleton = False
        if media_id is not None:
            singleton = True
            kwargs.update({ '_id': media_id })

        if series_id is not None:
            kwargs.update({ 'series': series_id })

        return super(MediaResource, self).get(Media, singleton, **kwargs)

class UserResource(BaseResource):
    def get(self, user_token):
        user = User.objects(token=user_token).first()
        if user is None:
            user = User()
            user.token = user_token
            user.save()

        return user.json()

    def post(self, user_token):
        json_data = request.get_json(force=True)
        user = User.objects(token=user_token).first()
        if user is None:
            return { 'status': 'Error' }

        if 'subscribe' in json_data:
            status = subscribe(user, json_data['subscribe'])
            return { 'status': 'Ok' if status else 'Error' }

        if 'unsubscribe' in json_data:
            status = unsubscribe(user, json_data['unsubscribe'])
            return { 'status': 'Ok' if status else 'Error' }

        settings_fields = (
            'country',
            'crunchyroll_premium',
            'funimation_premium'
        )

        for field in settings_fields:
            if field in json_data:
                setattr(user, field, json_data[field])

        user.save()
        return { 'status': 'Ok' }

def register_api_resource(resource, *paths):
    _paths = []
    for path in paths:
        frags = ['api', version, path]
        _paths.append('/%s' % '/'.join(map(lambda x: x.strip('/'), frags)))
    api.add_resource(resource, *_paths)

@app.route('/pebble')
def pebble_configuration():
    return render_template('pebble.html')

@app.errorhandler(400)
@app.errorhandler(404)
@app.errorhandler(500)
@app.errorhandler(503)
def error_handler(error):
    return render_template('error.html', error=error)

register_api_resource(NetworkResource, '/networks', '/networks/<int:network_id>')
register_api_resource(SeriesResource, '/series', '/series/<int:series_id>',
    '/networks/<int:network_id>/series', '/user/<user_token>/subscriptions')
register_api_resource(MediaResource, '/media', '/media/<int:media_id>', '/series/<int:series_id>/media')
register_api_resource(UserResource, '/user/<user_token>')
