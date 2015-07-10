Funimation API
==============
This API is based on [Sinap](https://github.com/Sinap)'s work done in their plugin project: https://github.com/Sinap/plugin.video.funimation
In particular, the logic is a bit more straightforward and cleaner for what work I need done here.

## Note
Funimation requires authentication be able to receive the newest episode data (shows that are now airing), since we can't
authenticate the user, this may limit the amount of available results returned in `latest` and `simulcast` calls.
