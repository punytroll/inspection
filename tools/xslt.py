import lxml.etree as ET
import os
import sys

def process_file(file_path, transform):
	xml_document = ET.parse(file_path)
	transformed_document = transform(xml_document)
	with open(file_path, "w") as output_file:
		output_file.write(ET.tostring(transformed_document).decode("utf-8") + "\n")

transform_document = ET.parse(sys.argv[2])
transform = ET.XSLT(transform_document)
for (root, directories, files) in os.walk(sys.argv[1]):
	directories.sort()
	files.sort()
	for file in files:
		process_file(os.path.join(root, file), transform)
