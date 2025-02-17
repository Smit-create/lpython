#include <libasr/asr.h>
#include <libasr/containers.h>
#include <libasr/exception.h>
#include <libasr/asr_utils.h>
#include <libasr/asr_verify.h>
#include <libasr/pass/global_stmts.h>


namespace LFortran {

using ASR::down_cast;

using LFortran::ASRUtils::EXPR;

/*
 * This ASR pass transforms (in-place) the ASR tree and wraps all global
 * statements and expressions into a program.
 *
 */
void pass_wrap_global_stmts_into_program(Allocator &al,
            ASR::TranslationUnit_t &unit) {
    std::string fn_name_s = "_lfortran_main_program";
    SymbolTable *current_scope = al.make_new<SymbolTable>(unit.m_global_scope);
    std::string prog_name = "main_program";
    Vec<ASR::stmt_t*> prog_body;
    prog_body.reserve(al, 1);
    if (unit.n_items > 0) {
        pass_wrap_global_stmts_into_function(al, unit, fn_name_s);
        ASR::symbol_t *fn = unit.m_global_scope->scope[fn_name_s];
        if (ASR::is_a<ASR::Subroutine_t>(*fn)) {
            ASR::asr_t *stmt = ASR::make_SubroutineCall_t(
                    al, unit.base.base.loc,
                    fn, nullptr,
                    nullptr, 0,
                    nullptr);
            prog_body.push_back(al, ASR::down_cast<ASR::stmt_t>(stmt));
        } else {
            throw LFortranException("Return type not supported yet");
        }
    }
    ASR::asr_t *prog = ASR::make_Program_t(
        al, unit.base.base.loc,
        /* a_symtab */ current_scope,
        /* a_name */ s2c(al, prog_name),
        nullptr,
        0,
        /* a_body */ prog_body.p,
        /* n_body */ prog_body.n);
    unit.m_global_scope->scope[prog_name] = ASR::down_cast<ASR::symbol_t>(prog);
    LFORTRAN_ASSERT(asr_verify(unit));
}

} // namespace LFortran
