from optparse import OptionParser
from sys import exit
from xml.dom.minidom import Node, parse
import subprocess

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

def get_text(nodes):
	texts = list()
	for node in nodes:
		if node.nodeType == node.TEXT_NODE:
			texts.append(node.data)
	return "".join(texts)

class Execute(object):
	def __init__(self):
		self.command = None
		self.arguments = list()

class Test(object):
	def __init__(self):
		self.expected_output = None
		self.execute = Execute()

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
				if node.tagName == "execute":
					execute_element = node
					for node in execute_element.childNodes:
						if node.nodeType == Node.ELEMENT_NODE:
							if node.tagName == "command":
								test.execute.command = get_text(node.childNodes)
							elif node.tagName == "argument":
								test.execute.arguments.append(get_text(node.childNodes))
				elif node.tagName == "expected-output":
					test.expected_output = get_text(node.childNodes)
		tests.append(test)

number_of_tests = 0
number_of_successes = 0
number_of_failures = 0
for test in tests:
	number_of_tests += 1
	print("Running \"" + test.execute.command + " " + " ".join(test.execute.arguments) + "\"")
	result = subprocess.run([test.execute.command] + test.execute.arguments, stdout=subprocess.PIPE)
	result = result.stdout.decode("utf-8")
	if result == test.expected_output:
		number_of_successes += 1
		print("    => \"" + BrightGreen + result + Reset + "\"")
	else:
		number_of_failures += 1
		print("    => \"" + BrightRed + result + Reset + "\"")
		print("Test failed! Expected output was \"" + BrightBlue + test.expected_output + Reset + "\".")
print()
if number_of_failures == 0:
	print("All " + Yellow + str(number_of_successes) + Reset + " test(s) " + BrightGreen + "successful" + Reset + ".")
else:
	print("Out of " + Yellow + str(number_of_tests) + Reset + " test(s), " + Yellow + str(number_of_successes) + Reset + " test(s) were " + BrightGreen + "successful" + Reset + " and " + Yellow + str(number_of_failures) + Reset + " test(s) " + BrightRed + "failed" + Reset + ".")
