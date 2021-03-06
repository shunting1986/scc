/*
 * Handle the parsing for expression
 */
#include <assert.h>
#include <inc/parser.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/dynarr.h>
#include <inc/syntree-visitor.h>

#define D 1

static struct cast_expression *parse_cast_expression(struct parser *parser);
static struct unary_expression *parse_unary_expression(struct parser *parser);

static struct primary_expression *parse_primary_expression(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct primary_expression *prim_expr = primary_expression_init();

	if (tok.tok_tag == TOK_IDENTIFIER) {
		prim_expr->id = tok.id.s;
	} else if (tok.tok_tag == TOK_STRING_LITERAL) {
		prim_expr->str = tok.str.s;
	} else if (tok.tok_tag == TOK_CONSTANT_VALUE) {
		prim_expr->const_val_tok = tok;
	} else if (tok.tok_tag == TOK_LPAREN) {
		struct expression *expr = parse_expression(parser);
		expect(parser->lexer, TOK_RPAREN);
		prim_expr->expr = expr;
	} else {
		file_reader_dump_remaining(parser->lexer->cstream);
		assert(0);
		// we may destroy primary expression first
		panic("ni %s", token_tag_str(tok.tok_tag));
	}

	return prim_expr;
}

static struct argument_expression_list *parse_argument_expression_list(struct parser *parser) {
	struct argument_expression_list *arg_list = argument_expression_list_init();
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_RPAREN) {
		return arg_list;
	}
	lexer_put_back(parser->lexer, tok);
	while (1) {
		struct assignment_expression *assign_expr = parse_assignment_expression(parser);
		dynarr_add(arg_list->list, assign_expr);
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_RPAREN) {
			break;
		} else if (tok.tok_tag != TOK_COMMA) {
			file_reader_dump_remaining(parser->lexer->cstream);
			panic("argument expression list expects ',', but %s found", token_tag_str(tok.tok_tag));
		}
	}
	return arg_list;
}

static struct postfix_expression *parse_postfix_expression(struct parser *parser) {
	struct primary_expression *prim_expr = parse_primary_expression(parser);
	struct postfix_expression *post_expr = postfix_expression_init(prim_expr);
	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_LPAREN) {
			struct argument_expression_list *arg_expr_list = parse_argument_expression_list(parser);
			struct postfix_expression_suffix *suf = mallocz(sizeof(*suf));
			suf->arg_list = arg_expr_list;
			dynarr_add(post_expr->suff_list, suf);
		} else if (tok.tok_tag == TOK_INC) {
			struct postfix_expression_suffix *suff = mallocz(sizeof(*suff));
			suff->is_inc = 1;
			dynarr_add(post_expr->suff_list, suff);
		} else if (tok.tok_tag == TOK_DEC) {
			struct postfix_expression_suffix *suff = mallocz(sizeof(*suff));
			suff->is_dec = 1;
			dynarr_add(post_expr->suff_list, suff);
		} else if (tok.tok_tag == TOK_DOT) {
			struct postfix_expression_suffix *suff = mallocz(sizeof(*suff));
			union token tok = expect(parser->lexer, TOK_IDENTIFIER);
			suff->dot_id = tok.id.s;
			dynarr_add(post_expr->suff_list, suff);
		} else if (tok.tok_tag == TOK_PTR_OP) {
			struct postfix_expression_suffix *suff = mallocz(sizeof(*suff));

			// disable typedef when parsing identifier
			int old_disable_typedef = lexer_push_config(parser->lexer, disable_typedef, 1);
			union token tok = expect(parser->lexer, TOK_IDENTIFIER);
			lexer_pop_config(parser->lexer, disable_typedef, old_disable_typedef);

			suff->ptr_id = tok.id.s;
			dynarr_add(post_expr->suff_list, suff);
		} else if (tok.tok_tag == TOK_LBRACKET) {
			struct expression *expr = parse_expression(parser);	
			struct postfix_expression_suffix *suff = mallocz(sizeof(*suff));
			suff->ind = expr;
			dynarr_add(post_expr->suff_list, suff);
			expect(parser->lexer, TOK_RBRACKET);
		} else { 
			lexer_put_back(parser->lexer, tok);
			break;
		}
	}
	return post_expr;
}

static int is_unary_op(int tok_tag) {
	return tok_tag == TOK_AMPERSAND ||
		tok_tag == TOK_STAR ||
		tok_tag == TOK_ADD ||
		tok_tag == TOK_SUB ||
		tok_tag == TOK_BITREVERSE ||
		tok_tag == TOK_EXCLAMATION;
}

static void parse_sizeof(struct parser *parser, struct unary_expression *unary_expr) {
	union token tok;
	tok = lexer_next_token(parser->lexer);

	if (tok.tok_tag != TOK_LPAREN) {
		lexer_put_back(parser->lexer, tok);
		unary_expr->sizeof_expr = parse_unary_expression(parser);
	} else {
		union token nxtok = lexer_next_token(parser->lexer);
		if (initiate_type_specifier(nxtok) || initiate_type_qualifier(nxtok)) {
			lexer_put_back(parser->lexer, nxtok);
			unary_expr->sizeof_type = parse_type_name(parser);
			expect(parser->lexer, TOK_RPAREN);
		} else {
			lexer_put_back(parser->lexer, nxtok);
			lexer_put_back(parser->lexer, tok);
			unary_expr->sizeof_expr = parse_unary_expression(parser);
		}
	}
}

static struct unary_expression *parse_unary_expression(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct unary_expression *unary_expr = unary_expression_init();

	if (tok.tok_tag == TOK_INC) {
		unary_expr->inc_unary = parse_unary_expression(parser);
		return unary_expr;
	} else if (tok.tok_tag == TOK_DEC) {
		unary_expr->dec_unary = parse_unary_expression(parser);
		return unary_expr;
	} else if (is_unary_op(tok.tok_tag)) {
		unary_expr->unary_op	= tok.tok_tag;
		unary_expr->unary_op_cast = parse_cast_expression(parser);
		return unary_expr;
	} else if (tok.tok_tag == TOK_SIZEOF) {
		parse_sizeof(parser, unary_expr);
		return unary_expr;
	} else {
		lexer_put_back(parser->lexer, tok);
		struct postfix_expression *post_expr = parse_postfix_expression(parser);
		unary_expr->postfix_expr = post_expr;
		return unary_expr;
	}
}

// only handle the case for unary_expression right now
static struct cast_expression *parse_cast_expression(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct cast_expression *cast_expr = cast_expression_init();

	if (tok.tok_tag == TOK_LPAREN) {
		union token nxtok = lexer_next_token(parser->lexer);
		if (initiate_type_name(nxtok)) {
			lexer_put_back(parser->lexer, nxtok);
			cast_expr->type_name = parse_type_name(parser);
			expect(parser->lexer, TOK_RPAREN);
			cast_expr->cast_expr = parse_cast_expression(parser);
			assert(cast_expr->unary_expr == NULL);
			return cast_expr;
		}
		lexer_put_back(parser->lexer, nxtok);
		lexer_put_back(parser->lexer, tok);
	} else {
		lexer_put_back(parser->lexer, tok); 
	}

	cast_expr->unary_expr = parse_unary_expression(parser);
	assert(cast_expr->type_name == NULL);
	assert(cast_expr->cast_expr == NULL);
	return cast_expr;
}

static struct multiplicative_expression *parse_multiplicative_expression(struct parser *parser) {
	struct cast_expression *cast_expr = parse_cast_expression(parser);
	struct multiplicative_expression *mul_expr = multiplicative_expression_init(cast_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_STAR && tok.tok_tag != TOK_DIV && tok.tok_tag != TOK_MOD) {
			lexer_put_back(parser->lexer, tok);
			break; 
		}

		dynarr_add(mul_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(mul_expr->cast_expr_list, parse_cast_expression(parser));
	}
	return mul_expr;
}

static struct additive_expression *parse_additive_expression(struct parser *parser) {
	struct multiplicative_expression *mul_expr = parse_multiplicative_expression(parser);
	struct additive_expression *add_expr = additive_expression_init(mul_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_ADD && tok.tok_tag != TOK_SUB) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(add_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(add_expr->mul_expr_list, parse_multiplicative_expression(parser));
	}
	return add_expr;
}

static struct shift_expression *parse_shift_expression(struct parser *parser) {
	struct additive_expression *add_expr = parse_additive_expression(parser);
	struct shift_expression *shift_expr = shift_expression_init(add_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LSHIFT && tok.tok_tag != TOK_RSHIFT) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(shift_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(shift_expr->add_expr_list, parse_additive_expression(parser));
	}
	return shift_expr;
}

static struct relational_expression *parse_relational_expression(struct parser *parser) {
	struct shift_expression *shift_expr = parse_shift_expression(parser);
	struct relational_expression *rel_expr = relational_expression_init(shift_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LT && tok.tok_tag != TOK_LE && tok.tok_tag != TOK_GT && tok.tok_tag != TOK_GE) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(rel_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(rel_expr->shift_expr_list, parse_shift_expression(parser));
	}
	return rel_expr;
}

static struct equality_expression *parse_equality_expression(struct parser *parser) {
	struct relational_expression *rel_expr = parse_relational_expression(parser);
	struct equality_expression *eq_expr = equality_expression_init(rel_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_EQ && tok.tok_tag != TOK_NE) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(eq_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(eq_expr->rel_expr_list, parse_relational_expression(parser));
	}
	return eq_expr;
}

static struct and_expression *parse_and_expression(struct parser *parser) {
	struct equality_expression *eq_expr = parse_equality_expression(parser);
	struct and_expression *and_expr = and_expression_init(eq_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_AMPERSAND) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(and_expr->eq_expr_list, parse_equality_expression(parser));
	}
	return and_expr;
}

static struct exclusive_or_expression *parse_exclusive_or_expression(struct parser *parser) {
	struct and_expression *and_expr = parse_and_expression(parser);
	struct exclusive_or_expression *xor_expr = exclusive_or_expression_init(and_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_XOR) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(xor_expr->and_expr_list, parse_and_expression(parser));
	}
	return xor_expr;
}

static struct inclusive_or_expression *parse_inclusive_or_expression(struct parser *parser) {
	struct exclusive_or_expression *xor_expr = parse_exclusive_or_expression(parser);
	struct inclusive_or_expression *or_expr = inclusive_or_expression_init(xor_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_VERT_BAR) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(or_expr->xor_expr_list, parse_exclusive_or_expression(parser));
	}
	return or_expr;
}

static struct logical_and_expression *parse_logical_and_expression(struct parser *parser) {
	struct inclusive_or_expression *or_expr = parse_inclusive_or_expression(parser);
	struct logical_and_expression *logic_and_expr = logical_and_expression_init(or_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LOGIC_AND) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(logic_and_expr->or_expr_list, parse_inclusive_or_expression(parser));
	}
	return logic_and_expr;
}

static struct logical_or_expression *parse_logical_or_expression(struct parser *parser) {
	struct logical_and_expression *and_expr = parse_logical_and_expression(parser);
	struct logical_or_expression *or_expr = logical_or_expression_init(and_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LOGIC_OR) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(or_expr->and_expr_list, parse_logical_and_expression(parser));
	}
	return or_expr;
}

struct conditional_expression *parse_conditional_expression(struct parser *parser) {
	struct logical_or_expression *or_expr = parse_logical_or_expression(parser);
	struct conditional_expression *cond_expr = conditional_expression_init(or_expr);
	union token tok;
	struct expression *expr;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_QUESTION) {
			lexer_put_back(parser->lexer, tok);
			break;
		}
		expr = parse_expression(parser);
		expect(parser->lexer, TOK_COLON);
		or_expr = parse_logical_or_expression(parser);
		
		dynarr_add(cond_expr->or_expr_list, or_expr);
		dynarr_add(cond_expr->inner_expr_list, expr);
	}
	return cond_expr;
}

static int is_assign_op(int tok_tag) {
	return tok_tag == TOK_ASSIGN ||
		tok_tag == TOK_MUL_ASSIGN ||
		tok_tag == TOK_DIV_ASSIGN ||
		tok_tag == TOK_MOD_ASSIGN ||
		tok_tag == TOK_ADD_ASSIGN ||
		tok_tag == TOK_SUB_ASSIGN ||
		tok_tag == TOK_LSHIFT_ASSIGN ||
		tok_tag == TOK_RSHIFT_ASSIGN ||
		tok_tag == TOK_AND_ASSIGN ||
		tok_tag == TOK_OR_ASSIGN ||
		tok_tag == TOK_XOR_ASSIGN;
}

/*
 * assignment_expression
 *   : conditional_expression
 *   | unary_expression assignment_operator assignment_expression
 *   ;
 *
 * It's tricky to find out which alternatives to use.
 *
 * 1) We can always parse conditional_expression first and if the conditional_expression turned
 *    to be a unary_expression and is followed by an assignment_operator, then we select
 *    the second one.
 * 2) We can parse cast_expression which is the first *FORK* node for conditional_expression and 
 *    also a parent of unary_expression
 *
 * XX I'll choose 2) solution since the first one will nest a unary_expression deeply in the 
 * XX expression tree which is not so efficient. We need make expression parsing efficient since
 * XX it's common.
 *
 * I choose 1). 1) has the drawback to nest a unary_expression deeply in the expression tree. However,
 * that's not a problem since anyway this happens a lot: f(a, 1), a and 1 will be nested in
 * assignment expression. The benefit we get is a cleaner function definitions.
 */
struct assignment_expression *parse_assignment_expression(struct parser *parser) {
	struct conditional_expression *cond_expr = parse_conditional_expression(parser);
	struct assignment_expression *assign_expr = assignment_expression_init();
	union token tok;
	struct unary_expression *unary_expr;

	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (!is_assign_op(tok.tok_tag)) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		// cond_expr will be in invalid state after this, memory will be released
		unary_expr = degen_to_unary_expr((struct syntreebasenode *) cond_expr); 

		dynarr_add(assign_expr->unary_expr_list, unary_expr);
		dynarr_add(assign_expr->oplist, (void *) (long) tok.tok_tag);

		cond_expr = parse_conditional_expression(parser);
	}
	
	assign_expr->cond_expr = cond_expr;
	return assign_expr;
}

struct expression *parse_expression(struct parser *parser) {
	struct dynarr *darr = dynarr_init();
	union token tok;
	struct assignment_expression *assign_expr = parse_assignment_expression(parser);

	dynarr_add(darr, assign_expr);
	tok = lexer_next_token(parser->lexer);
	while (tok.tok_tag == TOK_COMMA) {
		assign_expr = parse_assignment_expression(parser);	
		dynarr_add(darr, assign_expr);
		tok = lexer_next_token(parser->lexer);
	}
	lexer_put_back(parser->lexer, tok);

	return expression_init(darr);
}
