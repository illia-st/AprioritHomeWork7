#pragma once

// it could be a little Craftily and I could just make
// a wrapper upon std::shared_ptr but I think it would be a cheat
// So I will operate with raw pointers

// suppose there are no smart pointers in C++ at all

// forward declaration
template<class T>
class MyWeakPtr;

// I need shared_ptr to notify weak_ptr that the object is deleted
template <class T>
class MySharedPtr final{
    friend MyWeakPtr<T>;
public:
    MySharedPtr(): m_raw_ptr(nullptr){
        try{
            counter = new std::size_t(0);
        }catch(const std::exception& ex){
            counter = nullptr;
            throw;
        }
    }
    explicit MySharedPtr(T* raw_ptr): m_raw_ptr(raw_ptr), counter(nullptr){
        try{
            // I prefer to use new in constructor body
            // when the object is already constructed
            if(raw_ptr == nullptr)
                counter = new std::size_t(0);
            else{
                counter = new std::size_t(1);
            }
        }catch(const std::exception& ex){
            counter = nullptr;
            throw;
        }
    }
    MySharedPtr(const MySharedPtr& other) noexcept: m_raw_ptr(other.m_raw_ptr), counter(other.counter){
        if(m_raw_ptr != nullptr){
            ++*counter;
        }
    }
    MySharedPtr(MySharedPtr&& other): m_raw_ptr(other.m_raw_ptr), counter(other.counter){
        other.m_raw_ptr = nullptr;
        other.counter = nullptr;
    }
    MySharedPtr& operator = (const MySharedPtr& other) noexcept{
        this->m_raw_ptr = other.m_raw_ptr;
        this->counter = other.counter;
        this->alive = other.alive;
        ++*counter;
        return *this;
    }
    MySharedPtr& operator = (MySharedPtr&& other) noexcept{
        this->m_raw_ptr = other.m_raw_ptr;
        this->counter = other.counter;
        this->alive = other.alive;
        other.m_raw_ptr = nullptr;
        other.counter = nullptr;
        other.alive = nullptr;
        return *this;
    }
    T& operator *() const noexcept{
        return *m_raw_ptr;
    }
    T* operator -> () const noexcept{
        return m_raw_ptr;
    }
    ~MySharedPtr(){
        delete_obj();
//        std::cout << "destuctor is called" << std::endl;
    }
    T* get(){
        return m_raw_ptr;
    }
    void reset(T* ptr = nullptr) {
        delete_obj();
        counter = new std::size_t(0);
        if(ptr != nullptr){
            ++*counter;
        }
        m_raw_ptr = ptr;
    }
    void swap(MySharedPtr& other) noexcept{
        std::swap(other.m_raw_ptr, this->m_raw_ptr);
        std::swap(other.counter, this->counter);
    }
    std::size_t use_count() const noexcept{
        if(counter == nullptr) return 0;
        return *counter;
    }
    // I will skip bool unique() because it is removed in C++20
    explicit operator bool() const noexcept{
        return m_raw_ptr != nullptr;
    }
    std::ostream& operator << (std::ostream& out){
        //Equivalent to out << this->get();
        out << m_raw_ptr;
        return out;
    }
private:
    void delete_obj(){
        if(counter != nullptr){
            std::cout << "Counter in delete shared_ptr > " << *counter << std::endl;
        }
        if(m_raw_ptr != nullptr && --*counter == 0){
            delete m_raw_ptr;
            delete counter;
            std::cout << "Object is deleted now" << std::endl;
        }
        m_raw_ptr = nullptr;
        counter = nullptr;
    }
    T* m_raw_ptr;
    std::size_t* counter;
};


template<class T>
class MyWeakPtr final{
public:
    constexpr MyWeakPtr() noexcept: m_raw_ptr(nullptr), shared_ptr_counter(nullptr) { }
    MyWeakPtr(const MyWeakPtr& other) noexcept: m_raw_ptr(other.m_raw_ptr), shared_ptr_counter(other.shared_ptr_counter){ }
    MyWeakPtr(const MySharedPtr<T>& other): m_raw_ptr(other.m_raw_ptr), shared_ptr_counter(other.counter){ }
    MyWeakPtr(MyWeakPtr&& other) noexcept:  m_raw_ptr(other.m_raw_ptr), shared_ptr_counter(other.shared_ptr_counter){
        other.m_raw_ptr = nullptr;
        other.shared_ptr_counter = nullptr;
    }
    MyWeakPtr& operator = (const MyWeakPtr& other) noexcept{
        this->m_raw_ptr = other.m_raw_ptr;
        this->shared_ptr_counter = other.shared_ptr_counter;
        return *this;
    }
    MyWeakPtr& operator = (MyWeakPtr&& other) noexcept{
        this->m_raw_ptr = other.m_raw_ptr;
        this->shared_ptr_counter = other.shared_ptr_counter;

        other.m_raw_ptr = nullptr;
        other.shared_ptr_counter = nullptr;

        return *this;
    }
    MyWeakPtr& operator = (const MySharedPtr<T>& other) noexcept{
        this->m_raw_ptr = other.m_raw_ptr;
        this->shared_ptr_counter = other.counter;
        return *this;
    }
    ~MyWeakPtr(){
        reset();
    }
    void reset() noexcept{
        m_raw_ptr = nullptr;
        shared_ptr_counter = nullptr;
    }
    void swap(MyWeakPtr& other){
        std::swap(this->m_raw_ptr, other.m_raw_ptr);
        std::swap(this->shared_ptr_counter, other.shared_ptr_counter);
    }
    std::size_t use_count() const{
        try{
            // shared_ptr_counter can be deleted at this moment
            // so we have to avoid everything
            if(shared_ptr_counter == nullptr) return 0;
            std::size_t counter = *shared_ptr_counter;
            return counter;
        } catch(const std::exception& ex){
            std::cerr << ex.what() << std::endl;
            const_cast<MyWeakPtr<T>*>(this)->reset();
        }
        return 0;
    }
    bool expired() const noexcept{
        return use_count() == 0;
    }
    MySharedPtr<T> lock() const{
        if(expired()){
            return MySharedPtr<T>();
        }
        MySharedPtr<T> ptr;
        ptr.m_raw_ptr = this->m_raw_ptr;
        ptr.counter = this->shared_ptr_counter;
        ++*ptr.counter;
        return std::move(ptr);
    }
private:
    T* m_raw_ptr;
    std::size_t* shared_ptr_counter;
};