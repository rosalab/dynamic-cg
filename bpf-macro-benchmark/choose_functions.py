import random

def read_system_map(filepath):
    """
    Reads the System.map file and extracts lines containing function addresses.
    """
    with open(filepath, 'r') as file:
        lines = file.readlines()

    # Filter lines containing 'T' which indicates a global function (in the text segment)
    functions = [line.split()[-1] for line in lines if ' T ' in line]
    return functions

def select_random_functions(functions, seed, count=30):
    """
    Selects `count` random functions from the list using a user-defined seed.
    """
    random.seed(seed)  # Set the seed for reproducibility
    return random.sample(functions, count)

def main():
    system_map_path = '/lib/modules/6.11.0-rc5/build/System.map'

    # User-defined seed
    seed = 12333456

    functions = read_system_map(system_map_path)

    # Select 30 random functions
    random_functions = select_random_functions(functions, seed)

    # Print the selected functions
    print("Randomly selected functions:")
    for func in random_functions:
        print(func)

if __name__ == "__main__":
    main()
