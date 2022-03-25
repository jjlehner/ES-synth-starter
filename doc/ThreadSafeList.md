# ThreadSafeList\<T>

A wrapper around a generic list\<T> which ensures that a list can be accessed by multiple tasks and an interrupt service routine without race conditions and deadlocks. The member functions of ThreadSafeList are chosen to be the same as for the standard list container. One difference between ThreadSafeList and List is that the readISR function must be called from an interrupt service routine. ThreadSafeList does not use semaphores, rather it wraps list mutations in critical sections. This allows the ThreadSafeList type to be used within interrupt service routines. 

# Methods

- void push_back(T elem)
    - Appends item to list
- void remove(const T& val)
    - Removes from the list all the elements are equal to val
- std::pair<T, bool> find(const T& val)
    - Finds the element T in the list and returns a pair containing the value found and a bool to indicate if the value was in the list.
- std::pair<std::array<T, 8>, size_t> readISR
    - A function returns the first 8 elements of a list. As no mallocs are permitted inside an interrupt service routine, the elements are returned as an array wrapped in a pair. The second element of the pair indicates up to which element to read from the array, in case the list had fewer than 8 elements.
- void clear
    - Empties the list