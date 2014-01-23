#include "ruby.h"

void Init_fortitude_native_ext();

VALUE method_append_escaped_string(VALUE self, VALUE rb_output);
VALUE method_append_as_attributes(VALUE self, VALUE rb_output, VALUE prefix);

void Init_fortitude_native_ext() {
    VALUE string_class = rb_const_get(rb_cObject, rb_intern("String"));
    rb_define_method(string_class, "fortitude_append_escaped_string", method_append_escaped_string, 1);

    VALUE hash_class = rb_const_get(rb_cObject, rb_intern("Hash"));
    rb_define_method(hash_class, "fortitude_append_as_attributes", method_append_as_attributes, 2);
}

#define BUF_SIZE 256
#define MAX_SUBSTITUTION_LENGTH 6

void fortitude_escaped_strcpy(VALUE rb_output, const char * src) {
    char buf[BUF_SIZE + 1];
    char* buf_pos = buf;
    int buf_offset = 0;

    while (1) {
        if (buf_offset >= (BUF_SIZE - MAX_SUBSTITUTION_LENGTH)) {
            *buf_pos = '\0';
            rb_str_cat2(rb_output, buf);
            buf_pos = buf;
            buf_offset = 0;
        }

        char ch = *src;

        if (ch == '&') {
            *buf_pos++ = '&';
            *buf_pos++ = 'a';
            *buf_pos++ = 'm';
            *buf_pos++ = 'p';
            *buf_pos++ = ';';

            buf_offset += 5;
        } else if (ch == '<') {
            *buf_pos++ = '&';
            *buf_pos++ = 'l';
            *buf_pos++ = 't';
            *buf_pos++ = ';';

            buf_offset += 4;
        } else if (ch == '>') {
            *buf_pos++ = '&';
            *buf_pos++ = 'g';
            *buf_pos++ = 't';
            *buf_pos++ = ';';

            buf_offset += 4;
        } else if (ch == '"') {
            *buf_pos++ = '&';
            *buf_pos++ = 'q';
            *buf_pos++ = 'u';
            *buf_pos++ = 'o';
            *buf_pos++ = 't';
            *buf_pos++ = ';';

            buf_offset += 6;
        } else {
            if (ch == '\0') {
                break;
            }

            *buf_pos++ = ch;
            buf_offset += 1;
        }

        src++;
    }

    if (buf_offset > 0) {
        *buf_pos = '\0';
        rb_str_cat2(rb_output, buf);
    }
}

VALUE method_append_escaped_string(VALUE self, VALUE rb_output) {
    const char* c_self = RSTRING_PTR(self);
    VALUE html_safe = rb_iv_get(self, "@html_safe");

    if (RTEST(html_safe)) {
        rb_str_cat2(rb_output, c_self);
        return Qnil;
    }

    fortitude_escaped_strcpy(rb_output, c_self);
    return Qnil;
}

void fortitude_append_to(VALUE object, VALUE rb_output) {
    ID to_s;
    CONST_ID(to_s, "to_s");

    char buf[BUF_SIZE + 1];
    long value;
    int i;
    VALUE new_string, array_element;

    switch (TYPE(object)) {
        case T_STRING:
            fortitude_escaped_strcpy(rb_output, RSTRING_PTR(object));
            break;

        case T_SYMBOL:
            fortitude_escaped_strcpy(rb_output, rb_id2name(SYM2ID(object)));
            break;

        case T_ARRAY:
            value = RARRAY_LEN(object);
            for (i = 0; i < value; ++i) {
                array_element = rb_ary_entry(object, i);
                if (i > 0) {
                    rb_str_cat2(rb_output, " ");
                }
                fortitude_append_to(array_element, rb_output);
            }

        case T_NONE:
        case T_NIL:
            break;

        case T_FIXNUM:
            value = NUM2LONG(object);
            sprintf(buf, "%ld", value);
            rb_str_cat2(rb_output, buf);
            break;

        default:
            new_string = rb_funcall(object, to_s, 0);
            fortitude_escaped_strcpy(rb_output, RSTRING_PTR(new_string));
            break;
    }
}

struct fortitude_prefix_and_output {
    VALUE prefix;
    VALUE rb_output;
};

int fortitude_append_key_and_value(VALUE key, VALUE value, VALUE prefix_and_output_value) {
    struct fortitude_prefix_and_output * prefix_and_output = (struct fortitude_prefix_and_output *)prefix_and_output_value;

    VALUE prefix = prefix_and_output->prefix;
    VALUE rb_output = prefix_and_output->rb_output;

    ID fortitude_append_as_attributes_prefixed;
    CONST_ID(fortitude_append_as_attributes_prefixed, "fortitude_append_as_attributes_prefixed");

    char buf[BUF_SIZE + 1];

    if (TYPE(value) == T_HASH) {
        method_append_as_attributes(value, rb_output, key);
    } else {
        rb_str_cat2(rb_output, " ");
        if (TYPE(prefix) != T_NIL) {
            fortitude_append_to(prefix, rb_output);
            rb_str_cat2(rb_output, "-");
        }
        fortitude_append_to(key, rb_output);
        rb_str_cat2(rb_output, "=\"");
        fortitude_append_to(value, rb_output);
        rb_str_cat2(rb_output, "\"");
    }

    return 0;
}


VALUE method_append_as_attributes(VALUE self, VALUE rb_output, VALUE prefix) {
    struct fortitude_prefix_and_output prefix_and_output;

    prefix_and_output.prefix = prefix;
    prefix_and_output.rb_output = rb_output;

    rb_hash_foreach(self, fortitude_append_key_and_value, (VALUE) &prefix_and_output);
    return Qnil;
}

// int append_hash_attributes(VALUE key, VALUE val, VALUE current_output) {
//     static char buf[100];

//     char *c_key, *c_value;

//     sprintf(buf, " %s=\"%s\"", RSTRING_PTR(key), RSTRING_PTR(val));
//     rb_str_cat2(current_output, buf);

//     return ST_CONTINUE;
// }

// VALUE method_insert_element(VALUE self, VALUE rb_output, VALUE rb_name, VALUE rb_attributes) {
//     char* c_name = RSTRING_PTR(rb_name);

//     static char buf[100];

//     switch(TYPE(rb_attributes)) {
//         case T_STRING:
//             sprintf(buf, "<%s>", c_name);
//             rb_str_cat2(rb_output, buf);

//             rb_str_append(rb_output, rb_attributes);

//             sprintf(buf, "</%s>", c_name);
//             rb_str_cat2(rb_output, buf);
//         break;

//         case T_HASH:
//             sprintf(buf, "<%s ", c_name);
//             rb_str_cat2(rb_output, buf);

//             // fprintf(stderr, "About to append attributes...");

//             // rb_hash_foreach(rb_attributes, append_hash_attributes, rb_output);

//             // fprintf(stderr, "...appended attributes.");

//             if (rb_block_given_p()) {
//                 rb_str_cat2(rb_output, ">");

//                 rb_yield(Qnil);

//                 sprintf(buf, "</%s>", c_name);
//                 rb_str_cat2(rb_output, buf);
//             } else {
//                 rb_str_cat2(rb_output, "/>");
//                 sprintf(buf, ">");
//             }
//         break;

//         case T_NIL:
//             if (rb_block_given_p()) {
//                 sprintf(buf, "<%s>", c_name);
//                 rb_str_cat2(rb_output, buf);

//                 rb_yield(Qnil);

//                 sprintf(buf, "</%s>", c_name);
//                 rb_str_cat2(rb_output, buf);
//             } else {
//                 sprintf(buf, "<%s/>", c_name);
//                 rb_str_cat2(rb_output, buf);
//             }
//         break;

//         default:
//             /* raise */
//         break;
//     }

//     return Qnil;
// }

// /*
//       def #{element_name}(attributes = nil)
//         o = @output

//         if (! attributes)
//           if block_given?
//             o << FAST_ELEMENT_#{element_name.upcase}_OPEN
//             yield
//             o << FAST_ELEMENT_#{element_name.upcase}_CLOSE
//           else
//             o << FAST_ELEMENT_#{element_name.upcase}_ALONE
//           end
//         elsif attributes.kind_of?(String)
//           o << FAST_ELEMENT_#{element_name.upcase}_OPEN
//           o << attributes
//           o << FAST_ELEMENT_#{element_name.upcase}_CLOSE
//         else
//           o << FAST_ELEMENT_#{element_name.upcase}_PARTIAL_OPEN
//           _attributes(attributes)

//           if block_given?
//             o << FAST_ELEMENT_PARTIAL_OPEN_END
//             yield
//             o << FAST_ELEMENT_#{element_name.upcase}_CLOSE
//           else
//             o << FAST_ELEMENT_PARTIAL_OPEN_ALONE_END
//           end
//         end
//       end

// */