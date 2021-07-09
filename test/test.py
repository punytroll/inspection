import hashlib
from optparse import OptionParser
from sys import exit
from xml.dom.minidom import Node, parse
import os
import queue
import subprocess
import threading
import time

Black = "\x1b[30m"
Red = "\x1b[31m"
Green = "\x1b[32m"
Yellow = "\x1b[33m"
Blue = "\x1b[34m"
Magenta = "\x1b[35m"
Cyan = "\x1b[36m"
White = "\x1b[37m"
BrightBlack = "\x1b[90m"
BrightRed = "\x1b[91m"
BrightGreen = "\x1b[92m"
BrightYellow = "\x1b[93m"
BrightBlue = "\x1b[94m"
BrightMagenta = "\x1b[95m"
BrightCyan = "\x1b[96m"
BrightWhite = "\x1b[97m"
Reset = "\x1b[0m"

class Execute(object):
    def __init__(self):
        self.command = None
        self.arguments = list()
    
    def __str__(self):
        return f"Execute({self.command}, {self.arguments})"

class Setup(object):
    def __init__(self):
        self.description = None
        self.expected_output = None
        self.execute = Execute()
    
    def __str__(self):
        return f"Setup(\"{self.description}\", \"{self.expected_output}\", {self.execute})"
    
    def get_hash(self):
        return hashlib.sha256(str(self).encode("utf-8")).hexdigest()
    
class Statistics(object):
    def __init__(self):
        self.runtime = None
        self.success = None
    
    def __str__(self):
        return f"Statistics({self.success}, {self.runtime})"

class Run(object):
    def __init__(self):
        self.return_code = None
        self.output = None
        self.statistics = Statistics()

class Test(object):
    def __init__(self):
        self.setup = Setup()
        self.last_run_statistics = None
        self.this_run = Run()
    
    def __str__(self):
        return f"Test({self.setup}, {self.last_run_statistics})"

def get_text(nodes):
    texts = list()
    for node in nodes:
        if node.nodeType == node.TEXT_NODE:
            texts.append(node.data)
    return "".join(texts)

def get_space_appended(list):
    return " ".join(list)

def get_s_if_greater_one(number):
    if number > 1:
        return "s"
    else:
        return ""

def build_test_dictionary(test_list):
    result = dict()
    for test in test_list:
        test_hash = test.setup.get_hash()
        if test_hash in result:
            print(f"{BrightRed}ERROR{Reset}: Duplicated tests:")
            print(f"Test 1: {BrightYellow}{test_dictionary[test_hash].setup}{Reset}")
            print(f"Test 2: {BrightYellow}{test.setup}{Reset}")
            exit(1)
        else:
            result[test_hash] = test
    return result

def read_last_run_statistics():
    result = dict()
    try:
        with open(".test_statistics.csv", "r") as file:
            for line in file:
                line_parts = line[:-1].split(",")
                result[line_parts[0]] = (bool(line_parts[1]), float(line_parts[2]))
    except FileNotFoundError:
        pass
    return result

def test_runner(test_queue, finished_queue):
    while True:
        test = None
        try:
            test = test_queue.get_nowait()
            start_time = time.perf_counter()
            result = subprocess.run([test.setup.execute.command] + test.setup.execute.arguments, stdout=subprocess.PIPE)
            end_time = time.perf_counter()
            test.this_run.statistics.runtime = end_time - start_time
            test.this_run.return_code = result.returncode
            test.this_run.output = result.stdout.decode("utf-8")
            finished_queue.put_nowait(test)
        except queue.Empty:
            break

def test_scheduler(test_list, finished_queue):
    test_queue = queue.SimpleQueue()
    for test in test_list:
        test_queue.put_nowait(test)
    number_of_threads = os.cpu_count() * 2
    threads = list()
    for thread_index in range(number_of_threads):
        thread = threading.Thread(target = test_runner, args = (test_queue, finished_queue))
        thread.start()
        threads.append(thread)
    for thread in threads:
        thread.join()
    # this signals the end of processing
    finished_queue.put_nowait(None)

def load_tests_from_test_suite(test_suite_file_path):
    result = list()
    # now parse the test suite file
    test_suite_document = parse(test_suite_file_path)
    tests_element = test_suite_document.documentElement
    for node in tests_element.childNodes:
        if node.nodeType == Node.ELEMENT_NODE and node.tagName == "test":
            test_element = node
            test = Test()
            for node in test_element.childNodes:
                if node.nodeType == Node.ELEMENT_NODE:
                    if node.tagName == "description":
                        test.setup.description = get_text(node.childNodes)
                    elif node.tagName == "execute":
                        execute_element = node
                        for node in execute_element.childNodes:
                            if node.nodeType == Node.ELEMENT_NODE:
                                if node.tagName == "command":
                                    test.setup.execute.command = get_text(node.childNodes)
                                elif node.tagName == "argument":
                                    test.setup.execute.arguments.append(get_text(node.childNodes))
                    elif node.tagName == "expected-output":
                        test.setup.expected_output = get_text(node.childNodes)
            result.append(test)
    return result

def add_last_run_statistics(test_list):
    # read statistics of last run and add data to tests
    last_run_statistics = read_last_run_statistics()
    for test in test_list:
        test_hash = test.setup.get_hash()
        if test_hash in last_run_statistics:
            test.last_run_statistics = Statistics()
            test.last_run_statistics.success = last_run_statistics[test_hash][0]
            test.last_run_statistics.runtime = last_run_statistics[test_hash][1]
            del last_run_statistics[test_hash]

def sort_test_list(test_list):
    def key_function(test):
        # False < True
        if test.last_run_statistics == None:
            # assures that new tests are always run first
            return (False, None, None)
        else:
            # assures that failed tests and successful short tests are run first
            return (True, (test.last_run_statistics.success), (test.last_run_statistics.runtime))
    
    # add the statistics from the last run
    add_last_run_statistics(test_list)
    # sorting the test list by (new, success, runtime)
    test_list.sort(key = key_function)

def execute_test_suite(test_suite_file_path):
    test_list = load_tests_from_test_suite(test_suite_file_path)
    sort_test_list(test_list)
    magnitude_of_number_of_tests = len(str(len(test_list)))
    number_of_tests = 0
    number_of_successes = 0
    number_of_failures = 0
    finished_tests = list()
    finished_queue = queue.SimpleQueue()
    scheduler_thread = threading.Thread(target = test_scheduler, args = (test_list, finished_queue))
    scheduler_thread.start()
    # while the scheduler is waiting for the workers to finish, the main thread prints out information about finished tests
    while True:
        finished_test = finished_queue.get()
        if finished_test == None:
            break
        else:
            number_of_tests += 1
            print(f"{BrightWhite}[{BrightBlue}{str(number_of_tests).zfill(magnitude_of_number_of_tests)}{BrightWhite} / {BrightBlue}{len(test_list)}{BrightWhite}]{Reset}", end = "")
            if finished_test.setup.description != None:
                print(f" {BrightYellow}{finished_test.setup.description}{Reset}")
            else:
                print()
            print(f"    Running \"{finished_test.setup.execute.command} {get_space_appended(finished_test.setup.execute.arguments)}\"")
            if finished_test.this_run.return_code == 0:
                if finished_test.setup.expected_output != None:
                    if finished_test.this_run.output == finished_test.setup.expected_output:
                        number_of_successes += 1
                        print(f"        => \"{BrightGreen}{finished_test.this_run.output}{Reset}\"")
                        finished_test.this_run.statistics.success = True
                    else:
                        number_of_failures += 1
                        print(f"        => \"{BrightRed}{finished_test.this_run.output}{Reset}\"")
                        print(f"    Test failed! Expected output was \"{BrightBlue}{finished_test.setup.expected_output}{Reset}\".")
                        finished_test.this_run.statistics.success = False
                else:
                    number_of_successes += 1
                    print(f"        => {BrightGreen}Succeeded{Reset}")
                    finished_test.this_run.statistics.success = True
            else:
                number_of_failures += 1
                print(f"        => {BrightRed}Failed{Reset}")
                print(f"    Test failed! The return code of the test was {BrightBlue}{finished_test.this_run.return_code}{Reset} instead of {BrightYellow}0{Reset}.")
                finished_test.this_run.statistics.success = False
            print()
            finished_tests.append(finished_test)
    print()
    # clean up scheduler thread
    scheduler_thread.join()
    # output summary
    if number_of_failures == 0:
        print(f"All {Yellow}{str(number_of_successes)}{Reset} test{get_s_if_greater_one(number_of_successes)} {BrightGreen}succeeded{Reset}.")
    else:
        print(f"Out of {Yellow}{str(number_of_tests)}{Reset} test{get_s_if_greater_one(number_of_tests)}, {Yellow}{str(number_of_successes)}{Reset} test{get_s_if_greater_one(number_of_successes)} {BrightGreen}succeeded{Reset} and {Yellow}{str(number_of_failures)}{Reset} test{get_s_if_greater_one(number_of_failures)} {BrightRed}failed{Reset}.")
    # write statistics file
    with open(".test_statistics.csv", "w") as file:
        for test in finished_tests:
            file.write(f"{test.setup.get_hash()},{test.this_run.statistics.success},{test.this_run.statistics.runtime}\n")
    # exit, possibly with error code
    if number_of_failures > 0:
        exit(1)

if __name__ == "__main__":
    return_code = 1
    # initialize command line option parser
    parser = OptionParser()
    parser.add_option("-i", "--in", dest="test_suite", help="The test suite file.")
    # read command line options and validate
    (options, args) = parser.parse_args()
    if options.test_suite == None:
        print("Set a test suite with '--in'.")
    else:
        execute_test_suite(options.test_suite)
