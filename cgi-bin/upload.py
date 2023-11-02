# Open a file in write mode ('w')
with open('output.txt', 'w') as file:
    # Write "bla" to the file
    file.write('bla')

import time

while True:
    time.sleep(1)

import sys

print('Hello alkdjh laskhjklldkhlk hdlkh ldh ldash lhljhlh lworld', file=sys.stderr)
# Print a message to indicate that the text has been written to the file
print('Text has been written to the file.')