import sys
import time
import random
import threading
import subprocess

subprocess.check_call([sys.executable, '-m', 'pip', 'install', 
'requests'])

import requests

MAX_THREADS = 500
USLEEP = lambda x: time.sleep(x/1000000.0)
BASE_URL = "http://localhost:8000"
IDS = [None] * MAX_THREADS

def thread_function(idx, result):  
    # Sleep between 0 and 10 us.
    USLEEP(random.randint(0, 10))
    if IDS[idx] != None:
        id_data = {
            "metadata": {
                "id": IDS[idx]
            }
        }

    res = requests.post(url=BASE_URL + "/setup", json=(id_data if IDS[idx] != None else {}))
    if res.status_code != requests.codes.created:
        result[idx] = False
        return
    res_data = res.json()
    IDS[idx] = res_data["id"]
    
    setup_json = {
        "metadata": {
            "id": res_data["id"]
        },
        "metrics": {
            "mean_fps": {
                "description": "The average fps",
                "type": "gauge"
            },
            "mean_frametime": {
                "description": "The average fps",
                "type": "gauge"
            },
            "mean_gamethreadtime": {
                "description": "The average fps",
                "type": "gauge"
            },
            "mean_gputime": {
                "description": "The average fps",
                "type": "gauge"
            },
            "mean_rendertime": {
                "description": "The average fps",
                "type": "gauge"
            },
            "mean_rhithreadtime": {
                "description": "The average fps",
                "type": "gauge"
            },
            "num_hangs": {
                "description": "The average fps",
                "type": "gauge"
            },
            "memory_virtual": {
                "description": "The average fps",
                "type": "gauge"
            },
            "memory_physical": {
                "description": "The average fps",
                "type": "gauge"
            },
            "memory_gpu": {
                "description": "The average fps",
                "type": "gauge"
            }
        }
    }

    res = requests.post(url=BASE_URL + "/setup", json=setup_json)
    if res.status_code != requests.codes.created:
        result[idx] = False
        return
    
    for i in range(25):
        metric_json = {
            "id": res_data["id"],
            "mean_fps": i,
            "mean_frametime": i,
            "mean_gamethreadtime": i,
            "mean_gputime": i,
            "mean_rendertime": i,
            "mean_rhithreadtime": i,
            "num_hangs": i,
            "memory_virtual": i,
            "memory_physical": i,
            "memory_gpu": i
        }
        res = requests.post(url=BASE_URL + "/stats", json=metric_json)
        if res.status_code != requests.codes.ok:
            result[idx] = False
            return
        USLEEP(1000000)

    result[idx] = True


def main():
    for thread_limit in range(10,MAX_THREADS,10):
        print("Testing {} concurrent instances".format(thread_limit))
        threads = list()
        results = list()
        for idx in range(thread_limit):
            x = threading.Thread(target=thread_function, args=(idx, results))
            threads.append(x)
            results.append(None)
            x.start()

        for index, thread in enumerate(threads):
            thread.join()

        if all(results):
            print("Success")
        else:
            print("Fail")
            return
    


main()