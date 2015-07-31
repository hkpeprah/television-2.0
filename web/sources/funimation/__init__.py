from client import FunimationApi


API = FunimationApi()


def list_latest_episodes(limit=100):
    episodes_list = API.get_latest(limit=limit)
    return episodes_list


if __name__ == '__main__':
    import json

    print json.dumps(list_latest_episodes(), indent=4, sort_keys=True)
