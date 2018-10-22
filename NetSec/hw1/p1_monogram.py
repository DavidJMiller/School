import matplotlib.pylab as plt
from collections import OrderedDict

letter_freq = {}
with open("p1.txt") as file:
   total = 0 
   for line in file:
      for char in line:
         if char in letter_freq:
            letter_freq[char] = letter_freq[char] + 1
         else:
            letter_freq[char] = 1
         total = total + 1

letter_freq = OrderedDict(sorted(letter_freq.items(), key=lambda i: i[1], reverse=True))
for key in letter_freq.keys():
   letter_freq[key] = letter_freq[key]/total

plt.bar(range(len(letter_freq)), list(letter_freq.values()), align='center')
plt.xticks(range(len(letter_freq)), list(letter_freq.keys()))
plt.title('Frequency of Letters')
plt.xlabel('Letter')
plt.ylabel('Frequency')
plt.savefig('p1.png')
