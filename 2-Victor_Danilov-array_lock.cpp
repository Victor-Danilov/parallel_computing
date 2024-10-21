/*
 * Date: 20/10/2024
 * Author: Victor Danilov
 *
 * This program divides an array in segments
 * which every segment is ordinated by a separate thread.
 *
 * Each thread blocks his segment to avoid conflicts with other threads
 *
 * When all the segments are in order, the array is blocked to ensore that the final result is correct.
 *
 * The output shows every process fase.
 */


#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <set>
#include <algorithm>
#include <mutex>

// Mutex per sincronizzare l'accesso allo stdout
std::mutex printMutex;

class ArrayLock {
private:
    std::set<int> lockedIndices;  // Set to keep track of locked indices
    std::mutex mtx;  // Mutex to protect access to lockedIndices
    std::condition_variable cv;  // Condition variable to wait for unlocking

public:
    // Function to lock a range of indices in the array
    void lock(int indexFrom, int indexTo) {
        std::unique_lock<std::mutex> lock(mtx);
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Trying to lock indices from " << indexFrom << " to " << indexTo << "...\n";
        }
        // Wait until all the requested indices are available for locking
        cv.wait(lock, [&]() {
            for (int i = indexFrom; i <= indexTo; ++i) {
                if (lockedIndices.find(i) != lockedIndices.end()) {
                    std::lock_guard<std::mutex> printLock(printMutex);
                    std::cout << "Indices from " << indexFrom << " to " << indexTo << " are currently locked by another thread. Waiting...\n";
                    return false;
                }
            }
            return true;
        });
        // Lock the requested range of indices
        for (int i = indexFrom; i <= indexTo; ++i) {
            lockedIndices.insert(i);
        }
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Locked indices from " << indexFrom << " to " << indexTo << " successfully.\n";
        }
    }

    // Function to unlock a range of indices in the array
    void unlock(int indexFrom, int indexTo) {
        std::lock_guard<std::mutex> lock(mtx);
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Unlocking indices from " << indexFrom << " to " << indexTo << "...\n";
        }
        // Unlock the requested range of indices
        for (int i = indexFrom; i <= indexTo; ++i) {
            lockedIndices.erase(i);
        }
        // Notify all waiting threads that indices have been unlocked
        cv.notify_all();
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Unlocked indices from " << indexFrom << " to " << indexTo << " successfully.\n";
        }
    }
};

class Demo {
private:
    ArrayLock arrayLock;  // Instance of ArrayLock to manage locks on array segments
    std::vector<int> arr;  // The array to be sorted

public:
    // Constructor to initialize the array
    Demo(const std::vector<int>& inputArr) : arr(inputArr) {}

    // Function to sort a segment of the array
    void sortSegment(int start, int end) {
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Thread attempting to sort segment: [" << start << ", " << end << "]\n";
        }
        arrayLock.lock(start, end);  // Lock the segment before sorting
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Thread sorting segment: [" << start << ", " << end << "]\n";
        }
        std::sort(arr.begin() + start, arr.begin() + end + 1);  // Sort the specified segment
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Thread sorted segment: [" << start << ", " << end << "]\n";
        }
        arrayLock.unlock(start, end);  // Unlock the segment after sorting
    }

    // Function to run the demo, sorting segments of the array with multiple threads
    void runDemo() {
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Initial array: ";
            for (int val : arr) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }

        // Launch four threads trying to sort different parts of the array
        std::thread t1(&Demo::sortSegment, this, 0, 1);
        std::thread t2(&Demo::sortSegment, this, 2, 3);
        std::thread t3(&Demo::sortSegment, this, 4, 5);
        std::thread t4(&Demo::sortSegment, this, 6, 7);

        // Wait for all threads to complete
        t1.join();
        t2.join();
        t3.join();
        t4.join();

        // Sort the entire array to ensure the final result is fully sorted
        arrayLock.lock(0, arr.size() - 1);
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Sorting the entire array to finalize...\n";
        }
        std::sort(arr.begin(), arr.end());
        arrayLock.unlock(0, arr.size() - 1);

        // Print the sorted array
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Final sorted array: ";
            for (int val : arr) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    // Initialize the array with unsorted values
    std::vector<int> arr = {5, 3, 8, 4, 2, 7, 1, 6};
    // Create an instance of Demo with the given array
    Demo demo(arr);
    // Run the demo to demonstrate locking and sorting
    demo.runDemo();
    return 0;
}
