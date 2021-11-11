#
# --- file_entropy.py ---
# 
# Shannon Entropy of a given file
# Is capable of calculating H(X), H(X|X) and H(X|XX)
# Nov. 2020
import sys
import numpy as np


def entropy(x_count):
    p_x = compute_px(x_count)
    entropy = -np.sum(p_x * np.nan_to_num(np.log2(p_x)) )
    return entropy


def compute_px(x_count):
    p_x = np.nan_to_num(x_count / np.sum(x_count))
    return p_x

def compute_pxy(xy_count):
    p_x = np.sum(xy_count, axis=tuple(range(1, xy_count.ndim)))
    #p_xy = p_x * p_x
    p_x[np.isnan(p_x)] = 0
    #p_xy = np.nan_to_num(xy_count / np.sum(xy_count, axis=tuple(range(1, xy_count.ndim))) )
    #np.nan_to_num(np.sum(xy_count, axis=-1) / np.sum(xy_count) * np.sum(xy_count, axis=tuple(range(xy_count.ndim - 1))))
    return p_x

def conditional_entropy_given_x(xy_count, p_x):
    '''
        First of all, we calculate probability from the number of xy encounters
        Secondly, we calculate the entropy itself:
            H(Y|X) = -SUM[ p(x) * SUM( p(y|x) * log2 p(y|x)) ]
        Here axis0 is considered X and axis1 Y respectively
    '''
    p_xy = xy_count / np.sum(xy_count, axis=tuple(range(xy_count.ndim - 1)) )
    p_xy[np.isnan(p_xy)] = 0
    entropy = -np.sum( p_x * p_xy * np.nan_to_num(np.log2(p_xy)) )
    
    return entropy

def conditional_entropy_given_xx(xyz_count, p_xy):
    p_xy = compute_pxy(xyz_count).reshape((1,256))
    p_xyz = np.sum(xyz_count, axis=-1)
    p_xyz[np.isnan(p_xyz)] = 0
    e = p_xyz / np.sum(xyz_count) * np.nan_to_num(np.log2(p_xyz / p_xy))
    e[np.isnan(e)] = 0
    entropy = -np.sum( e )
    
    return entropy



def main():
    # Ignore numpy division by zero errors because log2 function always complaining
    np.seterr(all='ignore')

    if (len(sys.argv) > 1):
        filename = sys.argv[1]
    else:
        print("Encountered an error: No filename provided")
        print("Usage: [python] file_entropy.py path/to/file")
        return
    
    print(f'Calculating file entropy for {filename}:')

    with open(filename, "rb") as file:
        buffer = []

        for byte in file.read():  # read in chunks for large files
            buffer.append(byte)
        
        x0 = np.zeros((256))
        x1 = np.zeros((256, 256))
        x2 = np.zeros((256, 256, 256))

        prev1, prev2 = None, None
        for val in buffer:
            x0[val] += 1
            if prev1 is not None:
                x1[val, prev1] +=1
            if prev2 is not None:
                x2[val, prev1, prev2] += 1
            prev1 = val
            prev2 = prev1

        filesize = file.tell()  # we can get file size by reading current position

        print("  File size =", filesize, "bytes")

        entropy_x = entropy(x0)
        cond_entropy_x = conditional_entropy_given_x(x1, compute_px(x0))
        cond_entropy_xx = conditional_entropy_given_xx(x2, compute_pxy(x1))

        print("  Entropy H(X)    =", entropy_x, "=> Possible compression =", int(filesize * entropy_x / 8), "bytes")
        print("  Entropy H(X|X)  =", cond_entropy_x, "=> Possible compression =", int(filesize * cond_entropy_x / 8), "bytes")
        print("  Entropy H(X|XX) =", cond_entropy_xx, "=> Possible compression =", int(filesize * cond_entropy_xx / 8), "bytes")


if __name__ == "__main__":
    main()
