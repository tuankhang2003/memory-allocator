from helper_function import *

# Test basic function

def main():
    tests = ["test_alloc_free", "test_coalesce"]
    env = os.environ.copy()
    env["LD_PRELOAD"] = LIB_MALLOC_PATH

    for test in tests:
        print(green(f"Running {test}"))
        subprocess.run(["./" + test], env=env, check=True)
    
    print(green("Tests basic function passed"))

if __name__ == "__main__":
    main()