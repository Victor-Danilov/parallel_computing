/*
   Array "lock" has operations
   - lock (int indexFrom, in indexTo)
   - unlock (int indexFrom, in indexTo).
   The method lock () locks the array elements from indexFrom to indexTo inclusive.
   If at least one of the elements already "locked in" another thread, the call to lock ()
   is blocked.
   Unlock - frees the locked elements (with the same indices).
   Please implement lock object and provide meaningful use of it (for example,
   sorting an array values)
*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

template <typename T>
class LockArray {
private:
    std::vector<T> data;               // The array elements
    std::vector<bool> isLocked;        // Lock status of each element
    std::mutex mtx;                    // Mutex for synchronization
    std::condition_variable cv;        // Condition variable for waiting threads

public:
    // Constructor to initialize the array with given size and default value
    LockArray(size_t size, T defaultValue = T()) : data(size, defaultValue), isLocked(size, false) {}

    // Access element at index (without bounds checking)
    T& operator[](size_t index) {
        return data[index]; // Changed from data.at(index) to data[index]
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

    // Get the size of the array
    size_t size() const {
        return data.size();
    }

    // Provide begin() and end() functions to get iterators
    typename std::vector<T>::iterator begin() {
        return data.begin();
    }

    typename std::vector<T>::const_iterator begin() const {
        return data.begin();
    }

    typename std::vector<T>::iterator end() {
        return data.end();
    }

    typename std::vector<T>::const_iterator end() const {
        return data.end();
    }

    // Lock method to lock elements from indexFrom to indexTo inclusive
    void lock(int indexFrom, int indexTo) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&]() {
            for (int i = indexFrom; i <= indexTo; ++i) {
                if (isLocked[i]) {
                    return false;  
                }
            }
            // All elements are unlocked, proceed to lock them
            for (int i = indexFrom; i <= indexTo; ++i) {
                isLocked[i] = true;
            }
            return true;
        });
    }

    // Unlock method to unlock elements from indexFrom to indexTo inclusive
    void unlock(int indexFrom, int indexTo) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (int i = indexFrom; i <= indexTo; ++i) {
                isLocked[i] = false;
            }
        }
        cv.notify_all();  // Notify waiting threads that elements have been unlocked
    }
};

// Function for threads to sort a portion of the array
void sortArraySection(LockArray<int>& lockArray, int indexFrom, int indexTo, int threadID) {
    
    lockArray.lock(indexFrom, indexTo);

    std::sort(lockArray.begin() + indexFrom, lockArray.begin() + indexTo + 1);

    std::cout << "Thread " << threadID << " sorted indices [" << indexFrom << ", " << indexTo << "].\n";

    lockArray.unlock(indexFrom, indexTo);
}

int main() {
    // Initialize LockArray with sample data
    std::vector<int> initialData = {10, 3, 5, 8, 6, 2, 9, 7, 1, 4};
    LockArray<int> lockArray(initialData.size());
    for (size_t i = 0; i < initialData.size(); ++i) {
        lockArray[i] = initialData[i];
    }

    // Display initial array
    std::cout << "Initial Array: ";
    for (size_t i = 0; i < lockArray.size(); ++i) {
        std::cout << lockArray[i] << " ";
    }
    std::cout << "\n";

    // Create threads to sort different sections of the array
    std::thread t1(sortArraySection, std::ref(lockArray), 0, 5, 1);  // Thread 1 sorts indices 0 to 5
    std::thread t2(sortArraySection, std::ref(lockArray), 3, 9, 2);  // Thread 2 sorts indices 3 to 9

    // Wait for threads to complete
    t1.join();
    t2.join();

    // Display sorted array
    std::cout << "Sorted Array: ";
    for (size_t i = 0; i < lockArray.size(); ++i) {
        std::cout << lockArray[i] << " ";
    }
    std::cout << "\n";

    return 0;
}
