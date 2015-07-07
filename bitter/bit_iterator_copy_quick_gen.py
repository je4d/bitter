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

def changeopt(n, opt):
    return n & 1<<(3-opt)

def randopts(opts, diffbits):
    ret = list(copy.deepcopy(opts))
    if changeopt(diffbits, ITER):
        ret[ITER] = random.sample(set(iter_options[ITER]) - set([opts[ITER]]),1)[0]
    if changeopt(diffbits, BO):
        ret[BO] = random.sample(set(iter_options[BO]) - set([opts[BO]]),1)[0]

    possible_uls = set([opts[UL]]) if not changeopt(diffbits, UL) else (set(iter_options[UL]) - set([opts[UL]]))
    if opts[YO] == "byte_order::none":
        if changeopt(diffbits, YO):
            possible_uls -= set(["uint8_t"])
        else:
            possible_uls &= set(["uint8_t"])
    if not len(possible_uls):
        return None
    ret[UL] = random.sample(possible_uls, 1)[0]

    possible_yos = set([opts[YO]]) if not changeopt(diffbits, YO) else (set(iter_options[YO]) - set([opts[YO]]))
    if ret[UL] == "uint8_t":
        possible_yos &= set(["byte_order::none"])
    else:
        possible_yos -= set(["byte_order::none"])
    if not len(possible_yos):
        return None
    ret[YO] = random.sample(possible_yos, 1)[0]
    return tuple(ret)

def indentwrap(string, indent, wrap):
    ret = ""
    maxlen = wrap - indent
    while len(string) > maxlen:
        spaceidx = string.rfind(" ", 0, maxlen-1)
        if spaceidx == -1:
            spaceidx = string.find(" ", maxlen-1)
        if spaceidx == -1:
            break
        ret += string[:spaceidx+1] + '"\n' + ' '*indent
        string = '"' + string[spaceidx+1:]
    return ret + string

noalias_configs = []
noalias_configs_srcs = []
noalias_configs_dests = []
alias_configs = []
alias_configs_srcs = []
alias_configs_dests = []

for opts in itertools.product(*iter_options):
    if (opts[2] == "uint8_t") != (opts[3] == "byte_order::none"):
        continue
    for i in xrange(16):
        ropts = randopts(opts, i)
        if ropts:
            cfg = (opts, ropts)
            noalias_configs.append(cfg)
            noalias_configs_srcs.append(cfg)

        ropts = randopts(opts, i)
        if ropts:
            cfg = (ropts, opts)
            noalias_configs.append(cfg)
            noalias_configs_dests.append(cfg)

        if not changeopt(i, UL):
            ropts = randopts(opts, i)
            if ropts:
                cfg = (opts, ropts)
                alias_configs.append(cfg)
                alias_configs_srcs.append(cfg)

            ropts = randopts(opts, i)
            if ropts:
                cfg = (ropts, opts)
                alias_configs.append(cfg)
                alias_configs_dests.append(cfg)

for cfg in noalias_configs:
    if cfg in noalias_configs_srcs and cfg in noalias_configs_dests:
        (noalias_configs_srcs, noalias_configs_dests)[random.getrandbits(1)].remove(cfg)
for cfg in alias_configs:
    if cfg in alias_configs_srcs and cfg in alias_configs_dests:
        (alias_configs_srcs, alias_configs_dests)[random.getrandbits(1)].remove(cfg)

noalias_configs = []
alias_configs = []

if "srcs" in sys.argv[1:]:
    noalias_configs += noalias_configs_srcs
    alias_configs += alias_configs_srcs
if "dests" in sys.argv[1:]:
    noalias_configs += noalias_configs_dests
    alias_configs += alias_configs_dests

const_pfx = "const_" if "const" in sys.argv[1:] else ""
fn_name = "test_copy_quick_%s%s" % (const_pfx, "srcs" if "srcs" in sys.argv[1:] else "dests")
impl_fn_name_template = "test_%%saliasing_copy_quick_%s%s_impl_%%d" % (const_pfx, "srcs" if "srcs" in sys.argv[1:] else "dests")
impl_fn_names=[]

print """\
#include <test_bit_iterator_copy_quick.hpp>"""

for configs, opt_non_lowline, opt_non_dash, has_fill in ((noalias_configs, "non_", "non-", True),
                                                         (alias_configs, "", "", False)):
    fn_num = 1
    while configs:
        impl_fn_name = impl_fn_name_template % (opt_non_lowline, fn_num)
        impl_fn_names += [impl_fn_name]

        print """
template <template <bit_order, typename, byte_order> class InIter>
void %s()
{
    using namespace bitter;
""" % impl_fn_name
        fn_num += 1

        prev = None
        for in_opts, out_opts in configs[:10]:
            if (in_opts, out_opts) == prev:
                continue
            prev = copy.deepcopy((in_opts, out_opts))
            in_iter = in_opts[0] % tuple(["bit_iterator"]+list(in_opts[1:]))
            out_iter = out_opts[0] % tuple(["bit_iterator"]+list(out_opts[1:]))
            fill = (random.sample(xrange(2),1)[0] if has_fill else None)
            description = '"correctly performs %saliasing copies from %s<%s,%s,%s> to %s<%s,%s,%s>%s",' % (
                opt_non_dash,
                ("reverse" if "reverse" in in_opts[0] else ""), trim(in_opts[1]), in_opts[2], trim(in_opts[3]),
                ("reverse" if "reverse" in out_opts[0] else ""), trim(out_opts[1]), out_opts[2], trim(out_opts[3]),
                " (fill=%s)" % fill if fill != None else "")
            print """\
    it(%s
       [] {
           test_%saliasing_copy<
               %s,
               %s>(
               for_each_quicktest_range{}%s);
       });""" % (
                indentwrap(description, len("    it("), 80),
                opt_non_lowline, in_iter, out_iter,
                ", bitter::bit(%s)" % ("true" if fill else "false") if fill != None else "")
        configs = configs[10:]

        print """\
}"""


print """
void %s()
{""" % fn_name
for impl_fn_name in impl_fn_names:
    print "    %s<bitter::%sbit_iterator>();" % (impl_fn_name, const_pfx)
print "}"

