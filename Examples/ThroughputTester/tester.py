import sys
import time
import uuid
import random
import threading
import subprocess
from datetime import datetime


subprocess.check_call([sys.executable, '-m', 'pip', 'install', 
'requests'])

import requests

MAX_THREADS = 100
USLEEP = lambda x: time.sleep(x/1000000.0)
BASE_URL = "http://localhost:8000"

def thread_function(idx, result):  
    # Sleep between 0 and 10 us.
    USLEEP(random.randint(0, 10))

    test_duration = random.randint(300,600)
    start_time = datetime.now()
    delta = 0
    while(delta < test_duration):
        stats_json = {
            "id": str(uuid.uuid4()),
            "metadata": {
            },
            "metrics": {
                "mean_fps": {
                    "description": "The average fps",
                    "value": random.randint(0, 60)
                },
                "mean_frametime": {
                    "description": "The average frame time",
                    "value": random.randint(0, 60)
                },
                "mean_gamethreadtime": {
                    "description": "The average game thread time",
                    "value": random.randint(0, 60)
                },
                "mean_gputime": {
                    "description": "The average gpu time",
                    "value": random.randint(0, 60)
                },
                "mean_rendertime": {
                    "description": "The average render thread time",
                    "value": random.randint(0, 60)
                },
                "mean_rhithreadtime": {
                    "description": "The average rhi thread time",
                    "value": random.randint(0, 60)
                },
                "num_hangs": {
                    "description": "The average number of hangs over the recorded interval",
                    "value": random.randint(0, 60)
                },
                "memory_virtual": {
                    "description": "The average virtual memory usage",
                    "value": random.randint(0, 60)
                },
                "memory_physical": {
                    "description": "The average physical memory usage",
                    "value": random.randint(0, 60)
                },
                "memory_gpu": {
                    "description": "The average gpu memory usage",
                    "value": random.randint(0, 60)
                }
            }
        }

        res = requests.post(url=BASE_URL + "/stats", json=stats_json)
        if res.status_code != requests.codes.ok:
            result[idx] = False
            return
        
        USLEEP(1000000)
        delta = (datetime.now() - start_time).total_seconds() / 60.0

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

    for _, thread in enumerate(threads):
        thread.join()

    if all(results):
        print("Success")
    else:
        print("Fail")

main()