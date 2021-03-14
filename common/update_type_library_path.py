import os.path
import sys

if len(sys.argv) < 3:
	print("ERROR: You need to supply the path to the type library path include and the absolute path to the type library.")
else:
	if os.path.isfile(sys.argv[2]) == True:
		with open(sys.argv[2], "r") as file:
			old_content = file.read()
	else:
		old_content = ""
	new_content = f"const std::string g_TypeLibraryPath = \"{sys.argv[1]}\";\n"
	if old_content != new_content:
		print("Writing new library path to include file:", file = sys.stderr)
		print(f"    {sys.argv[1]}", file = sys.stderr)
		with open(sys.argv[2], "w") as file:
			file.write(new_content)
