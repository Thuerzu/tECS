#pragma once 

#include <cstddef>
#include <type_traits>

namespace tECS
{
    template <size_t N>
    struct Bitset
    {
        using Byte = unsigned char;
        constexpr static size_t ByteCount = (N + 7) / 8;
    public:
        Bitset()
        {
            Clear();
        };

        void Set(size_t index)
        {
            Bits[index / 8] |= (1 << (index % 8));
        }

        void Reset(size_t index)
        {
            Bits[index / 8] &= ~(1 << (index % 8));
        }

        bool Test(size_t index) const
        {
            return Bits[index / 8] & (1 << (index % 8));
        }

        bool Any() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[i] != 0)
                    return true;
            }
            return false;
        }

        bool None() const
        {
            return !Any();
        }

        bool All() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[i] != 0xFF)
                    return false;
            }
            return true;
        }

        void Clear()
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] = 0;
        }

        void Fill()
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] = 0xFF;
        }

        void Flip(size_t index)
        {
            Bits[index / 8] ^= (1 << (index % 8));
        }

        void Flip()
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] ^= 0xFF;
        }

        void Or(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] |= other.Bits[i];
        }

        void And(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] &= other.Bits[i];
        }

        void Xor(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] ^= other.Bits[i];
        }

        void Nand(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] &= ~other.Bits[i];
        }

        void Nor(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] |= ~other.Bits[i];
        }

        void Xnor(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] ^= ~other.Bits[i];
        }
        
        void Assign(const Bitset<N>& other)
        {
            for (size_t i = 0; i < ByteCount; i++)
                Bits[i] = other.Bits[i];
        }

        void SetSequence(size_t offset, size_t length, size_t sequence)
        {
            for (size_t i = 0; i < length; i++)
            {
                if (sequence & (1 << i))
                    Set(offset + i);
                else
                    Reset(offset + i);
            }
        }

        size_t GetSequence(size_t offset, size_t length) const
        {
            //static_assert(length <= sizeof(size_t) * 8, "Length exceeds size_t capacity");
            size_t outSequence = 0;
            for (size_t i = 0; i < length; i++)
            {
                if (Test(offset + i))
                    outSequence |= (1 << i);
            }
            return outSequence;
        }

        size_t Popcount() const
        {
            size_t count = 0;
            for (size_t i = 0; i < ByteCount / 8; i++)
                count += std::popcount(*reinterpret_cast<const uint64_t*>(&Bits[i * 8]));
            for (size_t i = (ByteCount / 8) * 8; i < ByteCount; i++)
                count += std::popcount(Bits[i]);
            return count;
        }

        size_t CountrZero() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[i] != 0)
                {
                    return i * 8 + std::countr_zero(Bits[i]);
                }
            }
            return N;
        }

        size_t CountrOne() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[i] != 0xFF)
                {
                    return i * 8 + std::countr_one(Bits[i]);
                }
            }
            return N;
        }

        size_t CountLeadingZero() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[ByteCount - 1 - i] != 0)
                {
                    return i * 8 + std::countl_zero(Bits[ByteCount - 1 - i]);
                }
            }
            return N;
        }

        size_t CountLeadingOne() const
        {
            for (size_t i = 0; i < ByteCount; i++)
            {
                if (Bits[ByteCount - 1 - i] != 0xFF)
                {
                    return i * 8 + std::countl_one(Bits[ByteCount - 1 - i]);
                }
            }
            return N;
        }

    private:
        Byte Bits[ByteCount];
    };
}