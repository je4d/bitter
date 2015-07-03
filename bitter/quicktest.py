#!/usr/bin/python
import random
import itertools
import copy
import sys

random.seed(1234)

#testvec_bits=128
#def gen(testvec_bits):
#    for i in xrange(0,testvec_bits):
#        for j in xrange(i,testvec_bits):
#            for k in xrange(0,testvec_bits-(j-i)):
#                yield (i,j,k)
#
#for i, j, k in sorted(random.sample(list(gen(testvec_bits)), 1000)):
#    print("{ %3i, %3i, %3i }," % (i,j,k))

ITER, BO, UL, YO = range(4)

iter_options = [
        [
            "%s<%s,%s,%s>",
            "std::reverse_iterator<\n                   %s<%s,%s,%s>>",
        ],
        [
            "bit_order::msb0",
            "bit_order::lsb0",
        ],
        [
            "uint8_t",
            "uint16_t",
            "uint32_t",
            "uint64_t",
        ],
        [
            "byte_order::none",
            "byte_order::msb0",
            "byte_order::lsb0",
        ],
    ]

def trim(s):
    return {
            "bit_order::msb0": "msb0",
            "bit_order::lsb0": "lsb0",
            "byte_order::none": "none",
            "byte_order::msb0": "msb0",
            "byte_order::lsb0": "lsb0",
    }[s]

configs = []

def matchopt(n, opt):
    return n & 1<<(3-opt)

def randopts(opts, matchbits):
    ret = list(copy.deepcopy(opts))
    if not matchopt(matchbits, ITER):
        ret[ITER] = random.sample(set(iter_options[ITER]) - set([opts[ITER]]),1)[0]
    if not matchopt(matchbits, BO):
        ret[BO] = random.sample(set(iter_options[BO]) - set([opts[BO]]),1)[0]

    possible_uls = set([opts[UL]]) if matchopt(matchbits, UL) else (set(iter_options[UL]) - set([opts[UL]]))
    if opts[YO] == "byte_order::none":
        if matchopt(matchbits, YO):
            possible_uls &= set(["uint8_t"])
        elif not matchopt(matchbits, YO):
            possible_uls -= set(["uint8_t"])
    if not len(possible_uls):
        return None
    ret[UL] = random.sample(possible_uls, 1)[0]

    possible_yos = set([opts[YO]]) if matchopt(matchbits, YO) else (set(iter_options[YO]) - set([opts[YO]]))
    if ret[UL] == "uint8_t":
        possible_yos &= set(["byte_order::none"])
    else:
        possible_yos -= set(["byte_order::none"])
    if not len(possible_yos):
        return None
    ret[YO] = random.sample(possible_yos, 1)[0]
    return ret

for opts in itertools.product(*iter_options):
    if (opts[2] == "uint8_t") != (opts[3] == "byte_order::none"):
        continue
    for i in xrange(16):
        ropts = randopts(opts, i)
        if ropts:
            if "srcs" in sys.argv[1:]:
                configs.append(copy.deepcopy((opts, ropts)))
            elif "dests" in sys.argv[1:]:
                configs.append(copy.deepcopy((ropts, opts)))


const_pfx = "const_" if "const" in sys.argv[1:] else ""
fn_name = "test_non_aliasing_copy_quick_%s%s" % (const_pfx, "srcs" if "srcs" in sys.argv[1:] else "dests")
configs = sorted(configs)
fn_num = 1

print """\
#include <test_bit_iterator_copy_quick.hpp>"""

while configs:

    print """\
template <template <bit_order, typename, byte_order> class InIter>
void %s_impl_%d()
{
    using namespace bitter;
""" % (fn_name, fn_num)
    fn_num += 1

    prev = None
    for in_opts, out_opts in configs[:10]:
        if (in_opts, out_opts) == prev:
            continue
        prev = copy.deepcopy((in_opts, out_opts))
        in_iter = in_opts[0] % tuple(["bit_iterator"]+list(in_opts[1:]))
        out_iter = out_opts[0] % tuple(["bit_iterator"]+list(out_opts[1:]))
        fill = random.sample(xrange(2),1)[0]
        print """\
        it(\"correctly performs non-aliasing copies from %s<%s,%s,%s> \"
           \"to %s<%s,%s,%s> (fill=%s)\",
           [] {
               test_non_aliasing_copy<
                   %s,
                   %s>(
                   for_each_quicktest_range{}, bitter::bit(%s));
           });""" % (
                ("reverse" if "reverse" in in_opts[0] else ""), trim(in_opts[1]), in_opts[2], trim(in_opts[3]),
                ("reverse" if "reverse" in out_opts[0] else ""), trim(out_opts[1]), out_opts[2], trim(out_opts[3]),
                fill, in_iter, out_iter, "true" if fill else "false")
    configs = configs[10:]

    print """\
}"""


print """
void %s()
{""" % fn_name
for n in xrange(1,fn_num):
    print "    %s_impl_%d<bitter::%sbit_iterator>();" % (fn_name, n, const_pfx)
print "}"

