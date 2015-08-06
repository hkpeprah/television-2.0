/*global Pebble */

// Constants
////////////////////////////

var CONFIG_ROOT = 'http://de69c1a.ngrok.com';
var CONFIG_ENDPOINT = '/pebble';

// Local Globals
////////////////////////////

var userToken = null;

// Functions
////////////////////////////

function formatQueryParameters(params) {
  var queryParams = [];
  for (var key in params) {
    if (params.hasOwnProperty(key)) {
      var val = params[key];
      queryParams.push(encodeURIComponent(key) + '=' + encodeURIComponent(val));
    }
  }
  return '?' + queryParams.join('&');
}

// Event Listeners
////////////////////////////

Pebble.addEventListener('ready', function() {
  console.log('Ready');
});

Pebble.addEventListener('showConfiguration', function(e) {
  var url = CONFIG_ROOT + CONFIG_ENDPOINT;
  console.log(url);
  Pebble.getTimelineToken(function(token) {
    userToken = token;
    console.log(token);
    url += formatQueryParameters({ user_token: token });
    console.log(url);
    Pebble.openURL(url);
  });
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Webview Closed');
});
