#ifndef LCOMPILERS_PASS_MANAGER_H
#define LCOMPILERS_PASS_MANAGER_H

#include <libasr/asr.h>
#include <libasr/string_utils.h>
#include <libasr/alloc.h>

// TODO: Remove lpython/lfortran includes, make it compiler agnostic
#if __has_include(<lfortran/utils.h>)
    #include <lfortran/utils.h>
#endif

#if __has_include(<lpython/utils.h>)
    #include <lpython/utils.h>
#endif

#include <libasr/pass/do_loops.h>
#include <libasr/pass/for_all.h>
#include <libasr/pass/implied_do_loops.h>
#include <libasr/pass/array_op.h>
#include <libasr/pass/select_case.h>
#include <libasr/pass/global_stmts.h>
#include <libasr/pass/param_to_const.h>
#include <libasr/pass/print_arr.h>
#include <libasr/pass/print_list.h>
#include <libasr/pass/arr_slice.h>
#include <libasr/pass/flip_sign.h>
#include <libasr/pass/div_to_mul.h>
#include <libasr/pass/fma.h>
#include <libasr/pass/loop_unroll.h>
#include <libasr/pass/sign_from_value.h>
#include <libasr/pass/class_constructor.h>
#include <libasr/pass/unused_functions.h>
#include <libasr/pass/inline_function_calls.h>
#include <libasr/pass/dead_code_removal.h>
#include <libasr/pass/for_all.h>
#include <libasr/pass/select_case.h>
#include <libasr/pass/loop_vectorise.h>
#include <libasr/pass/update_array_dim_intrinsic_calls.h>
#include <libasr/pass/pass_array_by_data.h>
#include <libasr/pass/pass_list_expr.h>
#include <libasr/pass/pass_compare.h>
#include <libasr/pass/subroutine_from_function.h>
#include <libasr/asr_verify.h>

#include <map>
#include <vector>

namespace LCompilers {

    typedef void (*pass_function)(Allocator&, LFortran::ASR::TranslationUnit_t&,
                                  const LCompilers::PassOptions&);

    class PassManager {
        private:

        std::vector<std::string> _passes;
        std::vector<std::string> _with_optimization_passes;
        std::vector<std::string> _user_defined_passes;
        std::map<std::string, pass_function> _passes_db = {
            {"do_loops", &LFortran::pass_replace_do_loops},
            {"global_stmts", &LFortran::pass_wrap_global_stmts_into_function},
            {"implied_do_loops", &LFortran::pass_replace_implied_do_loops},
            {"array_op", &LFortran::pass_replace_array_op},
            {"arr_slice", &LFortran::pass_replace_arr_slice},
            {"print_arr", &LFortran::pass_replace_print_arr},
            {"print_list", &LFortran::pass_replace_print_list},
            {"class_constructor", &LFortran::pass_replace_class_constructor},
            {"unused_functions", &LFortran::pass_unused_functions},
            {"flip_sign", &LFortran::pass_replace_flip_sign},
            {"div_to_mul", &LFortran::pass_replace_div_to_mul},
            {"fma", &LFortran::pass_replace_fma},
            {"sign_from_value", &LFortran::pass_replace_sign_from_value},
            {"inline_function_calls", &LFortran::pass_inline_function_calls},
            {"loop_unroll", &LFortran::pass_loop_unroll},
            {"dead_code_removal", &LFortran::pass_dead_code_removal},
            {"forall", &LFortran::pass_replace_forall},
            {"select_case", &LFortran::pass_replace_select_case},
            {"loop_vectorise", &LFortran::pass_loop_vectorise},
            {"array_dim_intrinsics_update", &LFortran::pass_update_array_dim_intrinsic_calls},
            {"pass_list_expr", &LFortran::pass_list_expr},
            {"pass_array_by_data", &LFortran::pass_array_by_data},
            {"subroutine_from_function", &LFortran::pass_create_subroutine_from_function},
            {"pass_compare", &LFortran::pass_compare}
        };

        bool is_fast;
        bool apply_default_passes;
        std::string skip_pass;

        void _apply_passes(Allocator& al, LFortran::ASR::TranslationUnit_t* asr,
                           std::vector<std::string>& passes, PassOptions &pass_options,
                           LFortran::diag::Diagnostics &diagnostics) {
            for (size_t i = 0; i < passes.size(); i++) {
                // TODO: rework the whole pass manager: construct the passes
                // ahead of time (not at the last minute), and remove this much
                // earlier
                // Note: this is not enough for rtlib, we also need to include
                // it
                if (rtlib && passes[i] == "unused_functions") continue;
                _passes_db[passes[i]](al, *asr, pass_options);
            #if defined(WITH_LFORTRAN_ASSERT)
                if (!LFortran::asr_verify(*asr, true, diagnostics)) {
                    std::cerr << diagnostics.render2();
                    throw LFortran::LCompilersException("Verify failed");
                };
            #endif
            }
        }

        public:

        bool rtlib=false;

        PassManager(): is_fast{false}, apply_default_passes{false} {
            _passes = {
                "global_stmts",
                "class_constructor",
                "implied_do_loops",
                "pass_array_by_data",
                "pass_list_expr",
                "arr_slice",
                "subroutine_from_function",
                "array_op",
                "print_arr",
                "print_list",
                "array_dim_intrinsics_update",
                "do_loops",
                "forall",
                "select_case",
                "inline_function_calls",
                "unused_functions"
            };

            _with_optimization_passes = {
                "global_stmts",
                "class_constructor",
                "implied_do_loops",
                "pass_array_by_data",
                "arr_slice",
                "subroutine_from_function",
                "array_op",
                "print_arr",
                "print_list",
                "loop_vectorise",
                "loop_unroll",
                "array_dim_intrinsics_update",
                "do_loops",
                "forall",
                "dead_code_removal",
                "select_case",
                "unused_functions",
                "flip_sign",
                "sign_from_value",
                "div_to_mul",
                "fma",
                "inline_function_calls"
            };

            _user_defined_passes.clear();
        }

        void parse_pass_arg(std::string& arg_pass, std::string& s_pass) {
            _user_defined_passes.clear();
            skip_pass = s_pass;
            if (arg_pass == "") {
                return ;
            }

            std::string current_pass = "";
            for( size_t i = 0; i < arg_pass.size(); i++ ) {
                char ch = arg_pass[i];
                if (ch != ' ' && ch != ',') {
                    current_pass.push_back(ch);
                }
                if (ch == ',' || i == arg_pass.size() - 1) {
                    current_pass = LFortran::to_lower(current_pass);
                    if( _passes_db.find(current_pass) == _passes_db.end() ) {
                        std::cerr << current_pass << " isn't supported yet.";
                        std::cerr << " Only the following passes are supported:- "<<std::endl;
                        for( auto it: _passes_db ) {
                            std::cerr << it.first << std::endl;
                        }
                        exit(1);
                    }
                    _user_defined_passes.push_back(current_pass);
                    current_pass.clear();
                }
            }
        }

        void apply_passes(Allocator& al, LFortran::ASR::TranslationUnit_t* asr,
                          PassOptions& pass_options,
                          LFortran::diag::Diagnostics &diagnostics) {
            if( !_user_defined_passes.empty() ) {
                pass_options.fast = true;
                _apply_passes(al, asr, _user_defined_passes, pass_options,
                    diagnostics);
            } else if( apply_default_passes ) {
                pass_options.fast = is_fast;
                if( is_fast ) {
                    _apply_passes(al, asr, _with_optimization_passes, pass_options,
                        diagnostics);
                } else {
                    _apply_passes(al, asr, _passes, pass_options, diagnostics);
                }
            }
        }

        void use_optimization_passes() {
            is_fast = true;
        }

        void do_not_use_optimization_passes() {
            is_fast = false;
        }

        void use_default_passes() {
            apply_default_passes = true;
        }

        void do_not_use_default_passes() {
            apply_default_passes = false;
        }

    };

}
#endif
