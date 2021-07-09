import hashlib
from optparse import OptionParser
from sys import exit
from xml.dom.minidom import Node, parse
import subprocess
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

parser = OptionParser()
parser.add_option("-i", "--in", dest="test_suite", help="The test suite file.")

# read the options and validate
(options, args) = parser.parse_args()
if options.test_suite == None:
    print("Set a test suite with '--in'.")
    exit(1)

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
        return f"Test({self.description}, {self.expected_output}, {self.execute})"
    
    def get_hash(self):
        return hashlib.sha256(str(self).encode("utf-8")).hexdigest()
    
class Statistics(object):
    def __init__(self):
        self.runtime = None
        self.success = None

class Run(object):
    def __init__(self):
        self.statistics = Statistics()

class Test(object):
    def __init__(self):
        self.setup = Setup()
        self.last_run_statistics = None
        self.this_run = Run()

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
    result = list()
    try:
        with open(".test_statistics.csv", "r") as file:
            for line in file:
                line_parts = line[:-1].split(",")
                result.append((line_parts[0], bool(line_parts[1]), float(line_parts[2])))
    except FileNotFoundError:
        pass
    return result

tests = list()

# now parse the in_file
test_suite_document = parse(options.test_suite)
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
        tests.append(test)

# calculate test identifiers and build test dictionary
test_dictionary = build_test_dictionary(tests)

# read statistics of last run and add data to tests
last_run_statistics = read_last_run_statistics()
for statistics in last_run_statistics:
    # here, deleted tests are dropped silently
    if statistics[0] in test_dictionary:
        test_dictionary[statistics[0]].last_run_statistics = Statistics()
        test_dictionary[statistics[0]].last_run_statistics.success = statistics[1]
        test_dictionary[statistics[0]].last_run_statistics.runtime = statistics[2]

def sort_tests_key(test):
    # False < True
    if test.last_run_statistics == None:
        # assures that new tests are always run first
        return (False, None, None)
    else:
        # assures that failed tests and successful short tests are run first
        return (True, (test.last_run_statistics.success), (test.last_run_statistics.runtime))

tests.sort(key = sort_tests_key)

magnitude_of_number_of_tests = len(str(len(tests)))
number_of_tests = 0
number_of_successes = 0
number_of_failures = 0
for test in tests:
    number_of_tests += 1
    print(f"{BrightWhite}[{BrightBlue}{str(number_of_tests).zfill(magnitude_of_number_of_tests)}{BrightWhite} / {BrightBlue}{len(tests)}{BrightWhite}]{Reset}", end = "")
    if test.setup.description != None:
        print(f" {BrightYellow}{test.setup.description}{Reset}")
    else:
        print()
    print(f"    Running \"{test.setup.execute.command} {get_space_appended(test.setup.execute.arguments)}\"")
    start_time = time.perf_counter()
    result = subprocess.run([test.setup.execute.command] + test.setup.execute.arguments, stdout=subprocess.PIPE)
    end_time = time.perf_counter()
    test.this_run.statistics.runtime = end_time - start_time
    if result.returncode == 0:
        result = result.stdout.decode("utf-8")
        if test.setup.expected_output != None:
            if result == test.setup.expected_output:
                number_of_successes += 1
                print(f"        => \"{BrightGreen}{result}{Reset}\"")
                test.this_run.statistics.success = True
            else:
                number_of_failures += 1
                print(f"        => \"{BrightRed}{result}{Reset}\"")
                print(f"    Test failed! Expected output was \"{BrightBlue}{test.setup.expected_output}{Reset}\".")
                test.this_run.statistics.success = False
        else:
            number_of_successes += 1
            print(f"        => {BrightGreen}Succeeded{Reset}")
            test.this_run.statistics.success = True
    else:
        number_of_failures += 1
        print(f"        => {BrightRed}Failed{Reset}")
        print(f"    Test failed! The return code of the test was {BrightBlue}{result.setup.returncode}{Reset} instead of {BrightYellow}0{Reset}.")
        test.this_run.statistics.success = False
    print()
print()
if number_of_failures == 0:
    print(f"All {Yellow}{str(number_of_successes)}{Reset} test{get_s_if_greater_one(number_of_successes)} {BrightGreen}succeeded{Reset}.")
else:
    print(f"Out of {Yellow}{str(number_of_tests)}{Reset} test{get_s_if_greater_one(number_of_tests)}, {Yellow}{str(number_of_successes)}{Reset} test{get_s_if_greater_one(number_of_successes)} {BrightGreen}succeeded{Reset} and {Yellow}{str(number_of_failures)}{Reset} test{get_s_if_greater_one(number_of_failures)} {BrightRed}failed{Reset}.")

# write statistics file
with open(".test_statistics.csv", "w") as file:
    for test in tests:
        file.write(f"{test.setup.get_hash()},{test.this_run.statistics.success},{test.this_run.statistics.runtime}\n")

# exit, possibly with error code
if number_of_failures > 0:
    exit(1)
