if __name__ == '__main__':
    import json
    from client import FunimationApi

    fun = FunimationApi()
    print json.dumps(fun.get_latest_episodes(), indent=4)
