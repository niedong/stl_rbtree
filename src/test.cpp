/*
 * Copyright (c) 2020 niedong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <sstream>
#include <functional>
#include <exception>

#include <vector>
#include <array>
#include <string>
#include <map>
#include <set>

#include <cstdio>
#include <ctime>

#include "rbtree.h"

#define ARRSZ(arr) (sizeof(arr) / sizeof(*(arr)))

class Timer
{
protected:
    std::chrono::high_resolution_clock::time_point m_Start, m_Stop;
    std::chrono::duration<double> m_Duration;
public:
    void
    start(void)
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    void
    stop(void)
    {
        m_Stop = std::chrono::high_resolution_clock::now();
    }

    double
    time(void)
    {
        m_Duration = m_Stop - m_Start;
        return m_Duration.count();
    }
};

template<class T>
class Ordered
{
public:
    T m_Hold;
    rb_node m_Node;

    Ordered()
    {
    }

    Ordered(const T& val)
        : m_Hold(val)
    {
    }

    Ordered(T&& val)
        : m_Hold(std::move(val))
    {
    }

    static T &
    convert(const rb_node *conv)
    {
        return RB_CONV(Ordered, conv, m_Node)->m_Hold;
    }
};

template<class T>
static int
cmpf(const rb_node *n1, const rb_node *n2, void *args)
{
    return Ordered<T>::convert(n1) < Ordered<T>::convert(n2);
}

template<class T>
static T
get_sample(void)
{
    static std::default_random_engine e(static_cast<unsigned int>(time(NULL)));
    static std::uniform_int_distribution<T> gen;
    return gen(e);
}

template<class T, class ST, int multi>
class Suit
{
protected:
    ST m_STL;
    rb_tree m_RBT;
    
    std::vector<T> m_Samples;
    std::vector<Ordered<T>> m_Ordered;

    Timer m_Timer_stl, m_Timer_rbt;

    void
    init_sample(const size_t ss)
    {
        for (size_t i = 0; i < ss; ++i)
        {
            m_Samples.push_back(get_sample<T>());
            m_Ordered.push_back(Ordered<T>(m_Samples[i]));
        }
    }

    void
    stl_insert(void)
    {
        m_Timer_stl.start();

        for (const auto &samples : m_Samples)
        {
            m_STL.insert(samples);
        }

        m_Timer_stl.stop();
    }

    void
    stl_erase(void)
    {
        m_Timer_stl.start();

        for (const auto &samples : m_Samples)
        {
            m_STL.erase(samples);
        }

        m_Timer_stl.stop();
    }

    void
    rbt_insert(void)
    {
        m_Timer_rbt.start();

        int succ;

        for (auto &ordered : m_Ordered)
        {
            rb_insert(&m_RBT, &ordered.m_Node, &succ);
        }

        m_Timer_rbt.stop();
    }

    void
    rbt_erase(void)
    {
        m_Timer_rbt.start();

        for (const auto &samples : m_Samples)
        {
            Ordered<T> val(samples);

            rb_erase_val(&m_RBT, &val.m_Node);
        }

        m_Timer_rbt.stop();
    }

    void
    tst_insert(void)
    {
        std::cout << "<insert|insert> Multi: " << multi
            << ". Sample size: " << sample_size() << std::endl;

        static const std::function<void()> insert_func[] = {
            [&] { stl_insert(); }, [&] { rbt_insert(); }
        };

        std::array<std::thread, ARRSZ(insert_func)> thr;

        for (size_t i = 0; i < thr.size(); ++i)
        {
            thr[i] = std::thread(insert_func[i]);
        }

        for (auto &th : thr)
        {
            th.join();
        }

        finish(validate());
    }

    void
    tst_erase(void)
    {
        std::cout << "<erase|erase_val> Multi: " << multi
            << ". Current size: " << m_STL.size() << std::endl;

        static const std::function<void()> erase_func[] = {
            [&] { stl_erase(); }, [&] { rbt_erase(); }
        };

        std::array<std::thread, ARRSZ(erase_func)> thr;

        for (size_t i = 0; i < thr.size(); ++i)
        {
            thr[i] = std::thread(erase_func[i]);
        }

        for (auto &th : thr)
        {
            th.join();
        }

        finish(validate());
    }

    void
    tst_eqrange(void) const
    {
        for (const auto &samples : m_Samples)
        {
            Ordered<T> val(samples);

            const auto &stl_pr = m_STL.equal_range(samples);
            rb_pair rbt_pr = rb_eqrange(&m_RBT, &val.m_Node);
            
            if (!validate(stl_pr.first, stl_pr.second,
                rbt_pr.first, rbt_pr.second))
            {
                throw std::runtime_error("<equal_range|eqrange> failed");
            }
        }
    }

    void
    tst_count(void) const
    {
        for (const auto &samples : m_Samples)
        {
            Ordered<T> val(samples);

            if (!(m_STL.count(samples) == rb_vcnt(&m_RBT, &val.m_Node)))
            {
                throw std::runtime_error("<count|vcnt> failed");
            }
        }
    }

    void
    tst_bound(void) const
    {
        for (const auto &samples : m_Samples)
        {
            Ordered<T> val(samples);

            if (!validate(m_STL.lower_bound(samples), m_STL.upper_bound(samples),
                rb_lbnd(&m_RBT, &val.m_Node), rb_ubnd(&m_RBT, &val.m_Node)))
            {
                throw std::runtime_error("<lower_bound|lbnd>, <upper_bound|ubnd> failed");
            }
        }
    }

    void
    tst_find(void) const
    {
        for (const auto &samples : m_Samples)
        {
            Ordered<T> val(samples);

            if ((m_STL.find(samples) == m_STL.end() &&
                rb_find(&m_RBT, &val.m_Node) != rb_head(&m_RBT)) ||
                (*m_STL.find(samples) != Ordered<T>::convert(&val.m_Node)))
            {
                throw std::runtime_error("<find|find> failed");
            }
        }
    }

    void
    tst_clear(void)
    {
        m_STL.clear();
        rb_clear(&m_RBT);

        if (!validate())
        {
            throw std::runtime_error("<clear|clear> failed");
        }
    }

    void
    get_stl_content(typename ST::const_iterator stl_begin,
        typename ST::const_iterator stl_end,
        std::stringstream &content) const
    {
        content << std::distance(stl_begin, stl_end);

        for (auto &it = stl_begin; it != stl_end; ++it)
        {
            content << *it;
        }
    }

    void
    get_rbt_content(const rb_node *rbt_begin, const rb_node *rbt_end,
        std::stringstream &content) const
    {
        content << rb_dist(&m_RBT, rbt_begin, rbt_end);

        for (const rb_node *it = rbt_begin; it != rbt_end; it = rb_next(it))
        {
            content << Ordered<T>::convert(it);
        }
    }

    int
    validate(typename ST::const_iterator stl_begin,
        typename ST::const_iterator stl_end,
        const rb_node *rbt_begin,
        const rb_node *rbt_end) const
    {
        std::stringstream stl_con;
        std::stringstream rbt_con;

        get_stl_content(stl_begin, stl_end, stl_con);
        get_rbt_content(rbt_begin, rbt_end, rbt_con);

        return stl_con.str() == rbt_con.str();
    }

    int
    validate(void) const
    {
        return validate(m_STL.cbegin(), m_STL.cend(),
            rb_lmst(&m_RBT), rb_head(&m_RBT));
    }

    int
    finish(int succ)
    {
        static const char *result_str[] = { "failed", "success" };

        printf("  STL: %lfs, rb: %lfs. Status: %s\n",
            m_Timer_stl.time(), m_Timer_rbt.time(), result_str[succ]);

        return succ;
    }
public:
    Suit()
    {
        rb_init(&m_RBT, multi, cmpf<T>, NULL);
    }

    Suit(const size_t size)
    {
        rb_init(&m_RBT, multi, cmpf<T>, NULL);

        m_Samples.reserve(size);
        m_Ordered.reserve(size);

        init_sample(size);
    }

    size_t
    sample_size(void) const
    {
        return m_Samples.size();
    }

    int
    run(void)
    {
        static const std::function<void()> op_func[] = {
            [&] { tst_eqrange(); }, [&] { tst_count(); },
            [&] { tst_bound(); }, [&] { tst_find(); }
        };

        try
        {
            tst_insert();

            std::array<std::thread, ARRSZ(op_func)> thr;

            for (size_t i = 0; i < thr.size(); ++i)
            {
                thr[i] = std::thread(op_func[i]);
            }

            for (auto &th : thr)
            {
                th.join();
            }
            
            tst_erase();
            
            tst_clear();
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << std::endl;

            return 1;
        }

        return 0;
    }
};

int main(int argc, char **argv)
{
    static constexpr size_t tstc = 1 << 20;

    Suit<size_t, std::set<size_t>, 0> s1(tstc);
    Suit<size_t, std::multiset<size_t>, 1> s2(tstc);

    return s1.run() | s2.run();
}
