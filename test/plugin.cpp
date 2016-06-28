#include <boost/dll/alias.hpp>
#include <mw/gdb/break_point.hpp>
#include <mw/gdb/frame.hpp>
#include <vector>
#include <memory>

using namespace mw::gdb;


struct f_ptr : break_point
{
    f_ptr() : break_point("f(int* )")
    {

    }

    void invoke(frame & fr, const std::string & file, int line) override
    {
        fr.set("p", 0, "1");
        fr.set("p", 1, "2");
        fr.set("p", 2, "3");
    }
};

struct f_ref : break_point
{
    f_ref() : break_point("f(int & )")
    {
    }

    void invoke(frame & fr, const std::string & file, int line) override
    {
        fr.set("ref", "42");
    }
};



struct f_ret : break_point
{
    f_ret() : break_point("f()")
    {

    }
    void invoke(frame & fr, const std::string & file, int line) override
    {
        fr.return_("42");
    }
};


extern "C" BOOST_SYMBOL_EXPORT std::vector<std::unique_ptr<mw::gdb::break_point>> mw_gdb_setup_bps();

std::vector<std::unique_ptr<mw::gdb::break_point>> mw_gdb_setup_bps()
{
    std::vector<std::unique_ptr<mw::gdb::break_point>> vec;

    vec.push_back(std::make_unique<f_ptr>());
    vec.push_back(std::make_unique<f_ref>());
    vec.push_back(std::make_unique<f_ret>());

    return vec;
};

