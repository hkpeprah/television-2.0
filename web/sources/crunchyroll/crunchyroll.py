if __name__ == '__main__':
    import json
    from client import CrunchyrollApi

    crunchy = CrunchyrollApi()
    print json.dumps(crunchy.get_latest_episodes(), indent=4)
