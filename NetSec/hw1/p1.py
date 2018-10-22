import matplotlib.pylab as plt

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

for key in letter_freq.keys():
   letter_freq[key] = letter_freq[key]/total
   print(key, " : ", letter_freq[key])

plt.bar(letter_freq.keys(), letter_freq.values())
plt.show()
