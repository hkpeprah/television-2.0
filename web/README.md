Telebble Web Application
==============================
Web backend for the Pebble Time `Telebble` application.  Provides the data sources and the interface
for subscribing to them.  Users access the search page provided by the backend through the configuration
page on their phone.  When a user subscribes to a source here, it uses Pebble's Timeline API to subscribe
therein, similarly for when a user unsubscribes.  This API can also be hit directly, ignoring the web page.

## Setup
You have to have `pip` installed, so install `pip` following the instructions for your
[platform](https://pip.pypa.io/en/stable/).  You then need to install the requirements by running
`pip install -r requirements.txt`.  Once you have those installed, you'll want to start up a mongo service by
running [`mongod`](http://docs.mongodb.org/manual/installation/).  To start running the service, you'll need to have
a timeline API key, which you can get from Pebble's developer portal.

## Running the Service
To run, once you have `mongod` setup and running, run `PEBBLE_TIMELINE_API_KEY={YOUR_API_KEY} python wsgi.py`, this
will start the server and the worker.  To run the worker by itself, run
`PEBBLE_TIMELINE_API_KEY={YOUR_API_KEY} python worker.py`.  The database used by both is a MongoDB named
`telepebble`.


## Deploying a Server
1.  Set up a 'DNS' record (usually a 'CNAME') for the application to connect through.
2.  Install ['gunicorn'](http://gunicorn.org/) and ['supervisord'](http://supervisord.org/).
3.  Copy the template configuration file to '/etc/supervisor/conf.d/telebble.conf'.
4.  Update the configuration file with the correct path and API keys.
5.  Update your APACHE or NGINX configuration to point to the application (add an entry).
6.  `reread` and `update` gunicorn, then `start telebble`.
7.  Enjoy!


## Endpoints

| Endpoint                           | Description                                                                        |
| ---------------------------------- | ---------------------------------------------------------------------------------- |
| `/networks`                        | Returns a json object containing a set of network objects                          |
| `/networks/<network_id>`           | Returns a json object representing the network object matching the given id        |
| `/networks/<network_id>/series`    | Returns a json object containing a set of series objects belonging to that network |
| `/series`                          | Returns a json object containing a set of series objects                           |
| `/series/<series_id>`              | Returns a json object representing the series object matching the given id         |
| `/series/<series_id>/media`        | Returns a json object containing a set of media (episode) objects for the series   |
| `/media/`                          | Returns a json object containing a set of media objects                            |
| `/media/<media_id>`                | Returns a json object for the specified media object                               |
| `/user/<user_token>`               | Returns a json object representing the settings for a user object                  |
| `/user/<user_token>/subscriptions` | Returns a json object containing the set of all the user's subscriptions           |


## Query Parameters

| Query String          | Type   | Required | Default | Description                                                         |
| --------------------- | ------ | -------- | ------- | ------------------------------------------------------------------- |
| `?page=<page>`        | Number | No       | 1       | Used for pagination to get the next set of results beyond page `1`  |
| `?q=<query>`          | String | No       | None    | Specify a query to use when hitting an endpoint to perform a search |
| `?user_token=<token>` | String | No       | None    | Specify a user token to allow subscribing/unsubcribing to series    |


## Results Set

Endpoints that return a set of results, will return data confirming to the following style:

```
{
  metadata: {
    current_page: Number,
    num_pages: Number
  },
  count: Number,
  items: array of json objects
}
```

Other endpoints that return a single element will always return a json object (which may or may not be empty, so check
to see if there are keys).

### Media Object

```
{
  id: Number,
  name: String,
  summary: String,
  image: String,
  season: Number,
  number: Number,
  runtime: Number,
  series_id: Number,
  series_name: String,
  network_id: Number,
  network_name: String,
  timestamp: Number,
  air_date: String
}
```

### Series Object

```
{
  id: Number,
  name: String,
  description: String,
  genres: array of Strings,
  image: String,
  runtime: Number,
  network: Network object,
  latest: Media object,
  episode_count: Number
}
```

### Network Object

```
{
  id: Number,
  type: String,
  name: String,
  description: String,
  timezone: String,
  country: String,
  series_count: Number
}
```

### User Object

```
{
  id: Number,
  token: String,
  country: String,
  crunchyroll_premium: Boolean,
  funimation_premium: Boolean
  subscriptions: array of Numbers,
  num_subscriptions: Number
}
```
