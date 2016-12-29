common_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-pedantic',
    '-std=c99'
]

def FlagsForFile(filename, **kwargs):
    return {'flags': common_flags}
