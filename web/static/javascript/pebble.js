(function(window, $, _, setTimeout) {
  var api;
  var searchText = '';
  var defaultPage = 1;
  var currentPage = 1;
  var endOfResults = false;

  function API(user_token) {
    if (!(this instanceof API)) {
      return new API(user_token);
    }

    this.version = 'v1';
    this.baseUrl = '/api/' + this.version + '/';
    this.transactionId = 0;

    this.user_token = user_token;
    this.user_subscriptions = [];
    this.crunchyroll_premium = false;
    this.funimation_premium = false;
    this.country = 'US';

    return this;
  }

  API.prototype.getUser = function(callback) {
    if (!this.user_token) {
      return;
    }
    var self = this;
    var endpoint = 'user/' + this.user_token;
    var url = this.baseUrl + endpoint;

    callback = callback || function() {};
    $.getJSON(url, function(data) {
      callback(data);
    });
  };

  API.prototype.updateUser = function(data, callback) {
    if (!this.user_token) {
      return;
    }
    var self = this;
    var endpoint = 'user/' + this.user_token;
    var url = this.baseUrl + endpoint;

    callback = callback || function() {};
    $.ajax({
      type: 'POST',
      url: url,
      dataType: 'json',
      async: true,
      data: JSON.stringify(data),
      success: function(data) {
        callback(data);
      }
    });
  };

  API.prototype.getUserSubscriptions = function(callback) {
    if (!this.user_token) {
      return;
    }

    var self = this;
    var endpoint = 'user/' + this.user_token + '/subscriptions';
    var url = this.baseUrl + endpoint;
    callback = callback || function() {};
    $.getJSON(url, function(data) {
      callback(data);
    });
  };

  API.prototype.search = function(query, page, callback) {
    var endpoint = 'series';
    var self = this;
    var url = this.baseUrl + endpoint;
    var tid = this.transactionId + 1;
    this.transactionId = tid;
    url += formatQueryParameters({ 'q': query, 'page': page });
    callback = callback || function() {};
    $.getJSON(url, function(data) {
      if (self.transactionId == tid) {
        callback(data);
      }
    });    
  };

  function formatQueryParameters(params) {
    var queryParams = [];
    var keys = Object.keys(params);
    for (var i = 0; i < keys.length; i++) {
      var key = keys[i];
      var val = params[key];
      queryParams.push(encodeURIComponent(key) + '=' + encodeURIComponent(val));
    }
    return '?' + queryParams.join('&');
  }

  function createSeriesElemFromData(data) {
    var tmpl = $('#series-template').html();
    var rendered = _.template(tmpl);
    if (data && data.latest && data.latest.air_date) {
      var date = data.latest.air_date;
      var d = new Date(date.substring(0, date.length - 1));
      data.latest.air_date = d.toDateString() + ' ' + d.toLocaleTimeString();
    }
    if (api.user_token) {
      data.subscribed = api.user_subscriptions.indexOf(data.id) != -1;
    }
    data.user_token = api.user_token;
    rendered = rendered(data);
    return $(rendered);
  };

  function loadSubscriptions() {
    api.getUserSubscriptions(function(data) {
      var items = data.items || [];
      var target = $('#subscriptions-list');
      target.children().remove();
      $(items).each(function(idx, data) {
        target.append(createSeriesElemFromData(data));
      });
    });
  }

  function showSpinner() {
    $('#spinner').show();
  }

  function hideSpinner() {
    $('#spinner').hide();
  }

  function showErrorWindow() {
    var errorDiv = $('#error-overlay');
    errorDiv.show();
    setTimeout(function() {
      errorDiv.hide();
    }, 1000);
  }

  function showEndOfResults() {
    var noResults = $('#no-results');
    noResults.show();
    setTimeout(function() {
      noResults.hide();
    }, 1000);
  }

  function onUser(data) {
    if (!data) {
      return;
    }
    var crunchyrollSetting = $('[data-settings-type="crunchyroll_premium"] .checkbox');
    api.crunchyroll_premium = data.crunchyroll_premium;
    if (!api.crunchyroll_premium) {
      crunchyrollSetting.removeClass('selected');
    } else {
      crunchyrollSetting.addClass('selected');
    }

    var funimationSetting = $('[data-settings-type="funimation_premium"] .checkbox');
    api.funimation_premium = data.funimation_premium;
    if (!api.funimation_premium) {
      funimationSetting.removeClass('selected');
    } else {
      funimationSetting.addClass('selected');
    }

    api.country = data.country;
    api.user_subscriptions = data.subscriptions;

    loadSubscriptions();
  }

  function onSearchResults(data) {
    if (typeof data == 'undefined') {
      endOfResults = true;
    }

    var items = data.items;
    var current_page = data.metadata.current_page;
    var num_pages = data.metadata.num_pages;
    var target = $('#search-results');

    currentPage = current_page;
    if (currentPage == defaultPage) {
      target.children().remove();
    }

    if (current_page >= num_pages || items.length == 0) {
      endOfResults = true;
    }

    $(items).each(function(idx, data) {
      target.append(createSeriesElemFromData(data));
    });

    hideSpinner();
    if (endOfResults) {
      showEndOfResults();
    }
  }

  function attachListeners() {
    var $links = $([]);
    var $targets = $([]);
    var $window = $(window);
    $('a').each(function() {
      var href = $(this).attr('href');
      if (href && href[0] == '#' && href.length > 1) {
        $links = $links.add($(this));
        $targets = $targets.add($(href));
      }
    });

    $links.click(function(ev) {
      var idx = $links.index($(this));
      ev.preventDefault();
      ev.stopPropagation();
      $targets
        .css('display', 'none')
        .eq(idx)
        .css('display', 'block');
    });

    if ($links.length) {
      $links.first().click();
    }

    $('input#search').on('keyup', function(ev) {
      if (ev.which == 13) {
        searchText = $(this).val();
        ev.preventDefault();
        showSpinner();
        api.search(searchText, defaultPage, onSearchResults);
      }
    });

    $('[data-type="checkbox"]').click(function(ev) {
      var item = $(this).parent('.setting');
      var settingType = item.data('settings-type');
      var val = !$(this).hasClass('selected');
      var data = {};
      data[settingType] = val;
      api.updateUser(data, function(data) {
        api.getUser(onUser);
      });
    });

    $(document.body).on('click', '.description-short', function() {
      var $series = $(this).parent('.series');
      $series.find('.description-short').css('display', 'none');
      $series.find('.description-long').css('display', 'block');
    });

    $(document.body).on('click', '.description-long', function() {
      var $series = $(this).parent('.series');
      $series.find('.description-long').css('display', 'none');
      $series.find('.description-short').css('display', 'block');
    });

    $(document.body).on('click', '[data-type="subscribe"]', function() {
      var $series = $(this).parents('.series');
      var id = $series.data('id');
      var self = $(this);
      var data = {};
      if (api.user_subscriptions.indexOf(id) != -1) {
        data.unsubscribe = [ id ];
      } else {
        data.subscribe = [ id ];
      }
      showSpinner();
      api.updateUser(data, function(data) {
        if (data.status && data.status == 'Error') {
          hideSpinner();
          showErrorWindow();
        } else {
          api.getUser(function(data) {
            var subscriptions = data.subscriptions;
            self.text((subscriptions.indexOf(id) != -1 ? 'Unsubscribe' : 'Subscribe'));
            onUser(data);
            hideSpinner();
          });
        }
      });
    });

    $window.on('scroll', function(ev) {
      ev.preventDefault();
      ev.stopPropagation();

      if ($window.data('ajaxready') == false || !searchText || !searchText.length) {
        return;
      }

      if ($targets.eq(0).css('display') == 'none') {
        return;
      }

      // Mark false so that won't load more content
      $window.data('ajaxready', false);

      // Determine if we actually should load in new content
      var shouldLoad = !endOfResults;
      shouldLoad = shouldLoad && ($window.scrollTop() >= $(document).height() - $window.outerHeight());
      if (shouldLoad) {
        showSpinner();
        api.search(searchText, currentPage + 1, function(data) {
          onSearchResults(data);
          $window.data('ajaxready', true);
        });
      } else {
        $window.data('ajaxready', true);
      }
    });
  }

  $(window).ready(function() {
    var userToken = $.QueryString['user_token'];
    api = API(userToken);
    api.getUser(onUser);
    attachListeners();
  });
})(window, window.$ || window.jQuery, window._ || window.underscore, window.setTimeout);
