from optparse import OptionParser
from sys import exit
from xml.dom.minidom import Node, parse
import subprocess

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

for test in tests:
	result = subprocess.run(["./" + test.execute.command] + test.execute.arguments, stdout=subprocess.PIPE)
	if result.stdout.decode("utf-8") != test.expected_output:
		print("Failed with test.")
		exit(1)

print("All tests successful.")
