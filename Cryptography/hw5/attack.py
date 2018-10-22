import socket
import re

#Connect to server with given IP and port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.122.57", 31337))

#Encrypt prefix||message on server connected via socket
def encrpyThis(socket, prefix):
    request = str('-e ' + str(prefix))
    socket.send(request.encode())
    return socket.recv(1024).decode()

#Parse server reponse to get [num of bytes, ciphertext, IV]
def parseEncryption(message):
    parsed_response = re.search("b'Encryption: ([\d]*)\\\\n([\w ]*)\\\\n'\nIV: b'([\w]*)'", message)
    return [int(parsed_response.group(1)), "".join([c for c in parsed_response.group(2) if c != " "]), parsed_response.group(3)]

#Check if ciphertext is valid
def validCiphertext(socket, core, IV):
    request = str('-v ' + str(core) + ' ' + str(IV))
    socket.send(request.encode())
    return str(socket.recv(1024).decode()) == "Valid"

#Get block length, message length, and padding length
def getLengths(socket, total_bytes, IV):
    count = 0
    response = encrpyThis(s, "aa")
    while(int(re.search("(b'Encryption: )([\d]+)", response).group(2)) == total_bytes):
        response = encrpyThis(socket, "aa" * (count + 1))
        count = count + 1
    block_length = len(IV)//2
    message_length = total_bytes - 3 * block_length + count
    pad_length = total_bytes - message_length - block_length - 1
    return [block_length, message_length, pad_length]

# First block
OG_bytes, OG_ciphertext, OG_IV = parseEncryption(encrpyThis(s, ""))
OG_block_length, OG_message_length, OG_pad_length = getLengths(s, OG_bytes, OG_IV)
OG_ciphertext = OG_ciphertext[:-32] + OG_ciphertext[0:32]
while(validCiphertext(s, OG_ciphertext, OG_IV) == False):
    _, OG_ciphertext, OG_IV = parseEncryption(encrpyThis(s, ""))
    OG_ciphertext = OG_ciphertext[:-32] + OG_ciphertext[0:32]
print(chr(OG_pad_length ^ int(OG_IV[-2:],16) ^ int(OG_ciphertext[-34:-32], 16)))
for i in range(1, OG_block_length):
    _, ciphertext, IV = parseEncryption(encrpyThis(s, "00" * i))
    ciphertext = OG_ciphertext[:-32] + ciphertext[0:32]
    while(validCiphertext(s, ciphertext, OG_IV) == False):
        _, ciphertext, IV = parseEncryption(encrpyThis(s, "00" * i))
        ciphertext = OG_ciphertext[:-32] + ciphertext[0:32]
    print(chr(OG_pad_length ^ int(IV[-2:], 16) ^ int(ciphertext[-34:-32], 16)))

# Second block (not working)
OG_bytes, OG_ciphertext, OG_IV = parseEncryption(encrpyThis(s, ""))
OG_block_length, OG_message_length, OG_pad_length = getLengths(s, OG_bytes, OG_IV)
OG_ciphertext = OG_ciphertext[:-32] + OG_ciphertext[32:64]
while(validCiphertext(s, OG_ciphertext, OG_IV) == False):
    _, OG_ciphertext, OG_IV = parseEncryption(encrpyThis(s, ""))
    OG_ciphertext = OG_ciphertext[:-32] + OG_ciphertext[32:64]
print(chr(OG_pad_length ^ int(OG_ciphertext[30:32],16) ^ int(OG_ciphertext[-34:-32], 16)))
for i in range(1, OG_block_length):
    _, ciphertext, IV = parseEncryption(encrpyThis(s, "00" * i))
    ciphertext = OG_ciphertext[:-32] + ciphertext[32:64]
    while(validCiphertext(s, ciphertext, OG_IV) == False):
        _, ciphertext, IV = parseEncryption(encrpyThis(s, "00" * i))
        ciphertext = OG_ciphertext[:-32] + ciphertext[32:64]
    print(chr(OG_pad_length ^ int(ciphertext[30:32], 16) ^ int(ciphertext[-34:-32], 16)))

s.close()