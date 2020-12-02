#!/usr/bin/env python3
import subprocess
import argparse

#settings/commands
CONN_NAME="ble1" #the connection to use (must be already setup in newtmgr)

cmd_list_images = ["newtmgr", "-c"+CONN_NAME, "image", "list"]
hash_prefix = "hash: " #the letter directly before the hashes in the output
hash_length = 64 #in characters 

cmd_test = ["newtmgr", "-c"+CONN_NAME, "image", "test"]
cmd_reset = ["newtmgr", "-c"+CONN_NAME, "reset"]
cmd_confirm = ["newtmgr", "-c"+CONN_NAME, "image", "confirm"]

tasks = ['split_test', 'split_confirm', 'split_update']

#set up arguments parser
parser = argparse.ArgumentParser(description='Simplify some tasks for mynewt development.')
parser.add_argument('task',choices=tasks, help='Which task to run')
args = parser.parse_args()

if args.task == 'split_test':
    #get image hash
    list_result = subprocess.run(cmd_list_images, stdout=subprocess.PIPE).stdout.decode('utf-8')
    first_hash_idx = list_result.find(hash_prefix) + len(hash_prefix)
    second_hash_idx = list_result.find(hash_prefix,first_hash_idx) + len(hash_prefix)
    slot1_hash = list_result[second_hash_idx:second_hash_idx+hash_length]
    
    #assemble test command with hash
    cmd_test_hash = []
    cmd_test_hash = cmd_test.copy()
    cmd_test_hash.append(slot1_hash)
    print("Marking for test:",slot1_hash)

    #mark for testing and reset
    test_result = subprocess.run(cmd_test_hash, stdout=subprocess.PIPE).stdout.decode('utf-8')
    print(test_result)
    print("Resetting")
    reset_result = subprocess.run(cmd_reset, stdout=subprocess.PIPE).stdout.decode('utf-8')
    print(reset_result)
elif args.task == 'split_confirm':
    print("Confirming currently running image")
    confirm_result = subprocess.run(cmd_confirm, stdout=subprocess.PIPE).stdout.decode('utf-8')
    print(confirm_result)
elif args.task == 'split_update':
    print("Not implemented yet")




