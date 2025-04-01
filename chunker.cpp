#include <cstddef>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

//
// Класс - доступ к которому предоставляет итератор
//   Тут мы обращаемся к данным и обновляем их
//
template<typename T>
class Chunk
{
public:
    Chunk(T *start_ptr, const size_t chunk_offset, const size_t chunk_size):
        m_data(start_ptr),
        m_size(chunk_size),
        m_offset(chunk_offset)
    {
    }

    //
    // Обновление данных. Если вся работа велась указателем на константный массив
    //   то не соберется именно тут
    //
    bool copyFrom(const char *value, const size_t value_size)
    {         
        if (!value || !m_data)
            return false;

        std::memcpy(m_data + m_offset, value, std::min(value_size, m_size));
        return true;
    }

    //
    // Двигаем блок по области памяти. Хоть внутренние переменные и меняются
    //    данные остаются неизменными
    //
    void move_chunk(const size_t chunk_offset, const size_t chunk_size) const
    {
        m_size = chunk_size;
        m_offset = chunk_offset;
    }

    const char *data() const
    { 
        return reinterpret_cast<const char *>(m_data + m_offset); 
    }

    size_t size() const
    { 
        return m_size; 
    }

    size_t offset() const
    { 
        return m_offset; 
    }

private:
    T *m_data{nullptr};
    mutable size_t m_size{0};
    mutable size_t m_offset{0};
};


//
// Сам итератор
//
template<typename T>
class ChunkIterator
{
public:
    using chunk_type = Chunk<T>;    
    using iterator_category = std::output_iterator_tag;
    using value_type = chunk_type;
    using difference_type = chunk_type;
    using pointer = chunk_type*;
    using reference = chunk_type&;

    ChunkIterator(T *data, const size_t data_size, const size_t chunk_offset, const size_t chunk_size):
        m_data(data),
        m_data_size(data_size),
        m_chunk_size(chunk_size),
        m_chunk(data, chunk_offset, std::min(data_size - chunk_offset, chunk_size))
    {
    }    

    ChunkIterator &operator++()
    {
        const auto new_offset = m_chunk.offset() + m_chunk_size;
        size_t new_size = 0;
        if (new_offset < m_data_size)
            new_size = std::min(m_data_size - new_offset, m_chunk_size);
        m_chunk.move_chunk(new_offset, new_size);
        return *this;
    }

    bool operator==(const ChunkIterator &other) const
    {
        if ((m_data != other.m_data) || m_data_size != other.m_data_size)
            return false;

        if (!is_valid() && !other.is_valid())
            return true;

        return (m_chunk.data() == other.m_chunk.data()) && (m_chunk.size() == other.m_chunk.size());        
    }

    bool operator!=(const ChunkIterator &other) const
    {
        return !(*this == other);
    }

    reference operator*()
    { 
        return m_chunk;
    }

    pointer operator->()
    {
        return &m_chunk;
    }

    bool is_valid() const
    {
        return m_chunk.data() < reinterpret_cast<const char*>(m_data + m_data_size);
    }
    
private:
    chunk_type m_chunk;
    T *m_data{nullptr};
    size_t m_data_size{0};
    size_t m_chunk_size{0};
};

//
// Контейнер-чанкер.
//
template <typename T>
class Chunker
{
public:
    using char_type = typename std::conditional<std::is_const<T>::value, const char, char>::type;
    using value_type = Chunk<char_type>;
    using iterator = ChunkIterator<char_type>;
    using const_iterator = ChunkIterator<const char_type>;

    //
    // Размеры данных и чанка - в байтах
    //
    Chunker(T *data, const size_t data_size, const size_t chunk_size):
        m_data(reinterpret_cast<char_type *>(data)),
        m_data_size(data_size),
        m_chunk_size(chunk_size)
    {
    }

    iterator begin()
    {
        return iterator(m_data, m_data_size, 0, m_chunk_size);
    }

    iterator end()
    {
        return iterator(m_data, m_data_size, m_data_size, m_chunk_size);
    }

    //
    // Возвращает итератор по номеру чанка
    //
    iterator at(const size_t index)
    {
        return iterator(m_data, m_data_size, m_chunk_size * index, m_chunk_size);
    }

    //
    // Возвращаем итератор просто по сдвигу в байтах
    //
    iterator at_offset(const size_t offset)
    {
        return iterator(m_data, m_data_size, offset, m_chunk_size);
    }

    const_iterator begin() const
    {
        return const_iterator(m_data, m_data_size, 0, m_chunk_size);
    }

    const_iterator end() const
    {
        return const_iterator(m_data, m_data_size, m_data_size, m_chunk_size);
    }

    //
    // Возвращает итератор по номеру чанка
    //
    const_iterator at(const size_t index) const
    {
        return const_iterator(m_data, m_data_size, m_chunk_size * index, m_chunk_size);
    }

    //
    // Возвращаем итератор просто по сдвигу в байтах
    //
    const_iterator at_offset(const size_t offset) const
    {
        return const_iterator(m_data, m_data_size, offset, m_chunk_size);
    }

private:
    char_type *m_data{nullptr};
    size_t m_data_size{0};
    size_t m_chunk_size{0};
};

int main() {
    static const size_t CHUNK_SIZE = 4;

    // int32_t type version
    const int32_t dbl[] = {0,1,2,3,4,5,6,7,8,9,10,11,122,42,251*100};
    auto ch = Chunker(dbl, sizeof(dbl), CHUNK_SIZE);
    std::cout << "Integer array: " << dbl << std::endl;
    for (auto &item: ch) {
        // char newText[CHUNK_SIZE + 1] = "777";
        // item.copyFrom(newText, CHUNK_SIZE);
        std::cout << std::to_string(item.offset()) << ":" << std::to_string(item.size()) << ": \"";
        for (size_t i = 0; i < item.size(); ++i) 
            std::cout << std::setbase(16) << " " << std::setw(2) << std::setfill('0') << static_cast<int32_t>(item.data()[i] & 0xff);
        std::cout << "\"" << std::endl;
    }
    std::cout << std::endl;

    // updatable version
    char text[] = "hello my friend!";
    std::cout << "Non const text (write): " << text << std::endl;
    auto c = Chunker(text, strlen(text), CHUNK_SIZE);
    auto upd_it = c.at_offset(5);
    if (upd_it != c.end()) {
        char newText[CHUNK_SIZE + 1] = "777";
        upd_it->copyFrom(newText, CHUNK_SIZE);
    }
    
    std::cout << "Non const new text (write): " << text << std::endl;
    for (auto it = c.begin(); it != c.end(); ++it) 
        std::cout << std::to_string(it->offset()) << ":" << std::to_string(it->size()) << ": \"" << std::string(it->data(), it->size()) << "\"" << std::endl;    

    std::cout << std::endl;
    
    // constant version
    const char const_text[] = "hello my friend!";
    auto const_c = Chunker(const_text, strlen(const_text), CHUNK_SIZE);
    auto const_upd_it = const_c.at_offset(5);
    if (const_upd_it != const_c.end()) {
        char newText[CHUNK_SIZE + 1] = "777";
        // const_upd_it->copyFrom(newText, CHUNK_SIZE);
    }

    std::cout << "Const version (read): " << const_text << std::endl;
    for (const auto &item: const_c) 
        std::cout << std::to_string(item.offset()) << ":" << std::to_string(item.size()) << ": \"" << std::string(item.data(), item.size()) << "\"" << std::endl;

    return 0;
}