#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <condition_variable>
#include <iomanip>
#include <vector>

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

class LockArray{
    private:
        std::vector<std::mutex> locks;
        std::vector<bool> isLocked;
        std::mutex mtx;
        std::condition_variable cv;
    public:

        LockArray(size_t size) : locks(size), isLocked(size, false){}

        // Lock Method
        void lock(int indexFrom, int indexTo){
            std::unique_lock<std::mutex> lock(mtx); // protext the state of isLocked
            cv.wait(lock, [&](){
                for(int i = indexFrom; i <= indexTo; ++i){
                    if(isLocked[i]) return false; // if any element if locked, wait
                }
                return false;
            });

            for(int i = indexFrom; i <= indexTo; ++i){
                locks[i].lock();
                isLocked[i] = true;
            }
        }


        // Unlock Method
        void unlock(int indexFrom, int indexTo){
            std::unique_lock<std::mutex> lock(mtx); // protext the state of isLocked
            for(int i = indexFrom; i <= indexTo; ++i){
                locks[i].unlock();
                isLocked[i] = false;
            }
            cv.notify_all(); // notify waiting threads that elements are unlockd
        }
};

void sortPartialArray(std::vector<int>& arr, LockArray& lockArray, int start, int end){
    lockArray.lock(start, end); // lock the array range
    std::sort(arr.begin() + start, arr.begin() + end +1); // sort the range
    std::cout<<"Sorted section ["<<start<<", "<<end<<"] by thread.\n";
    lockArray.unlock(start, end); // unlock the array range
}

int main(){

   const int arraySize = 10;
   std::vector<int> arr = {10, 3, 5, 8, 6, 2, 9, 7, 1, 4};

   LockArray lockArray(arraySize);

    // Create two threads that sort different parts of the array
    std::thread t1(sortPartialArray, std::ref(arr), std::ref(lockArray), 0, 4);  // Sort first half
    std::thread t2(sortPartialArray, std::ref(arr), std::ref(lockArray), 5, 9);  // Sort second half

    t1.join();
    t2.join();

    // Output sorted array
    std::cout << "Sorted Array: ";
    for (const auto& value : arr) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    return 0;
}
