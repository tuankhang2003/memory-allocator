LIB_MALLOC_PATH = "../libmymalloc.so"

def green(text):
    return f"\033[92m{text}\033[0m"

def red(text):
    return f"\033[91m{text}\033[0m"

def print_green(text):
    print(green(text))