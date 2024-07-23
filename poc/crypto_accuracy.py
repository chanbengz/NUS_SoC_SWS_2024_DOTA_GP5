import argparse
import numpy as np
import os
import pdb

# get rid of outliers
def clear_outlier(time_list):
    time_list = np.array(time_list)
    data_mean = time_list.mean()
    data_std = time_list.std()
    new_time_list = []
    for dp in time_list:
        if ((dp-data_mean)/data_std <= 3) and ((dp-data_mean)/data_std >= -3):
            new_time_list.append(dp)
    new_time_list = np.array(new_time_list)
    return new_time_list

def get_mean_std(file_path, object_name):
    with open(file_path, 'r') as f:
        time_list = [int(x) for x in f.readlines()]
    new_time_list = clear_outlier(time_list)
    print(f"{object_name}: mean->{new_time_list.mean()};std->{new_time_list.std()}")


parser = argparse.ArgumentParser()
parser.add_argument('--crypto', help="cryptographic algorithm", required=False, default='dh')
args = parser.parse_args()

# Output profile results
get_mean_std(f"crypto_attacker/{args.crypto}_0.txt", args.crypto)
get_mean_std(f"crypto_attacker/{args.crypto}_1.txt", args.crypto)

# Output accuracy results (only DMP directly recovered part)
if args.crypto == "rsa":
    # Load RSA private key
    with open("crypto_victim/rsa_priv.txt") as f_v:
        secret_key = f_v.read().splitlines()
        secret_key = [int(secret_key[i], 16) for i in range(len(secret_key))]
    big_prime_str = str(hex(max(secret_key)))[2:]
    big_prime_list = [big_prime_str[i:i+2] for i in range(0, len(big_prime_str), 2)]
    # Load RSA DMP leakage
    with open("crypto_attacker/rsa.txt") as f_a:
        guess_key = f_a.read().splitlines()
    # RSA DMP leakage accuracy
    print("///////////////////////")
    print("/// DMP Leakage Accuracy Results ///")
    print("///////////////////////")
    count = 0
    for i in range(len(guess_key)):
        if guess_key[i] == big_prime_list[i]:
            count += 1
        else:
            print(f"Byte {i} -> secret:{big_prime_list[i]}, guess:{guess_key[i]}")
    print(f"DMP Leakage Accuracy: {count}/{len(guess_key)}")

    # Execute Coppersmith
    os.system("python coppersmith.sage.py")

    # RSA Full Recovery Accuracy
    # Load RSA Fully recovered key
    with open("crypto_attacker/rsa.txt") as f_a:
        guess_key = f_a.read().splitlines()
    # RSA DMP leakage accuracy
    print("///////////////////////")
    print("/// RSA Key Recovery Accuracy Results ///")
    print("///////////////////////")
    count = 0
    for i in range(len(guess_key)):
        if guess_key[i] == big_prime_list[i]:
            count += 1
        else:
            print(f"Byte {i} -> secret:{big_prime_list[i]}, guess:{guess_key[i]}")
    print(f"RSA Key Recovery Accuracy: {count}/{len(guess_key)}")
else:
    exit(1)
