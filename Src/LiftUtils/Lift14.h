#ifndef LIFT14_H
#define LIFT14_H

#include <string>
#include <list>
#include <XMLUtils.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <assert.h>
#include "Lift.h"

using std::stringstream;
using std::vector;
using std::list;
using std::wstring;
using std::exception;
using namespace Lift;

namespace Lift14 {

typedef wstring url;
typedef wstring datetime;
typedef wstring key;
typedef wstring lang;
typedef wstring href;
typedef wstring clazz;
typedef wstring refid;

class multitext;

class span : public lift_base {
public:
    span(LPCTSTR _name) :
        lift_base(_name),
        lang(L"lang"),
        href(L"href"),
        clazz(L"clazz"),
        _span(SPAN) {
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(lang,in);
        load_attribute(href,in);
        load_attribute(clazz,in);
        load_value(pcdata,in);
        load_element(_span,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_attribute(lang,out);
        store_attribute(href,out);
        store_attribute(clazz,out);
        store_value(pcdata,out);
        store_element(_span,out);
        return out;
    };

   bool operator==(const span & right) const {
		if (lang!=right.lang) return false;
		if (href!=right.href) return false;
		if (clazz!=right.clazz) return false;
		if (_span!=right._span) return false;
		if (pcdata.compare(right.pcdata)!=0) return false;
		return true;
    }

    optional<lang> lang;
    optional<href> href;
    optional<clazz> clazz;
    wstring pcdata;
    zero_more<span> _span;
};

class annotation;

class text : public lift_base {
public:
    text(LPCTSTR _name) :
        lift_base(_name),
		span(SPAN) {
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
    }

    void load_derived(Element * in) {
        load_element(span,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        return out;
    }

    void store_derived(Element * out) {
        store_element(span,out);
    }

    bool operator==(const text & right) const {
		if (span!=right.span) {
			return false;
		}
		return true;
    }

	zero_more<span> span;
};

class trait;
class form;

class multitext : public text {
public:
    multitext(LPCTSTR _name) :
        text(_name),
        form(L"form"),
        trait(L"trait") {
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
    };

    void load_derived(Element * in) {
        text::load_derived(in);
        load_element(form,in);
        load_element(trait,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        return out;
    };

    void store_derived(Element * out) {
        text::store_derived(out);
        store_element(form,out);
        store_element(trait,out);
    }

    // elements
    zero_more<form> form;
    optional<trait> trait;
};

class annotation : public multitext {
public:
    annotation(LPCTSTR _name) :
        multitext(_name),
        name(L"name"),
        value(L"value"),
        who(L"who"),
        when(L"when") {
    };

    void load(Element * in) {
        expect(in,lift_base::name);
        load_attribute(name,in);
        load_attribute(value,in);
        load_attribute(who,in);
        load_attribute(when,in);
    };

    Element * store() {
        Element * out = new Element(lift_base::name);
        store_attribute(name,out);
        store_attribute(value,out);
        store_attribute(who,out);
        store_attribute(when,out);
        return out;
    };

    // attributes
    required<key> name;
    required<key> value;
    optional<key> who;
    optional<datetime> when;
};

class form : public lift_base {
public:
    form(LPCTSTR _name) :
        lift_base(_name),
        lang(L"lang"),
        text(L"text"),
        annotation(ANNOTATION) {
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
    };

    void load_derived(Element * in) {
        load_attribute(lang,in);
        load_element(text,in);
        load_element(annotation,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        return out;
    };

    void store_derived(Element * out) {
        store_attribute(lang,out);
        store_element(text,out);
        store_element(annotation,out);
    }

    required<lang> lang;
    required<text> text;
    zero_more<annotation> annotation;
};

class trait : public lift_base {
public:
    trait(LPCTSTR _name) :
        lift_base(_name),
        name(L"name"),
        value(L"value"),
        id(L"id"),
        annotation(ANNOTATION) {
    };

    void load(Element * in) {
        expect(in,lift_base::name);
        load_attribute(name,in);
        load_attribute(value,in);
        load_attribute(id,in);
        load_element(annotation,in);
    };

    Element * store() {
        Element * out = new Element(lift_base::name);
        store_attribute(name,out);
        store_attribute(value,out);
        store_attribute(id,out);
        store_element(annotation,out);
        return out;
    }

    // attributes
    required<key> name;
    required<key> value;
    optional<key> id;
    // elements
    zero_more<annotation> annotation;
};

class trait;
class annotation;

class field : public multitext {
public:
    field(LPCTSTR _name) :
        multitext(_name),
        type(L"type"),
        datecreated(DATE_CREATED),
        datemodified(DATE_MODIFIED),
        //trait(L"trait"),
        form(L"form"),
        annotation(ANNOTATION) {
    };

    void load(Element * in) {
        expect(in,multitext::name);
        multitext::load_derived(in);
        load_attribute(type,in);
        load_attribute(datecreated,in);
        load_attribute(datemodified,in);
        //load_element(trait,in);
        load_element(form,in);
        load_element(annotation,in);
    };
    Element * store() {
        Element * out = new Element(lift_base::name);
        multitext::store_derived(out);
        store_attribute(type,out);
        store_attribute(datecreated,out);
        store_attribute(datemodified,out);
        //store_element(trait,out);
        store_element(form,out);
        store_element(annotation,out);
        return out;
    };

    // attributes
    required<key> type;
    optional<datetime> datecreated;
    optional<datetime> datemodified;
    // elements
    //zero_more<trait> trait;
    zero_more<Lift14::form> form;
    zero_more<annotation> annotation;
};

class extensible : public lift_base {
public:
    extensible(LPCTSTR name) :
        lift_base(name),
        datecreated(DATE_CREATED),
        datemodified(DATE_MODIFIED),
        field(L"field"),
        trait(L"trait"),
        annotation(ANNOTATION) {
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
        expect(in,name);
    };

    void load_derived(Element * in) {
        load_attribute(datecreated,in);
        load_attribute(datemodified,in);
        load_element(field,in);
        load_element(trait,in);
        load_element(annotation,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        return out;
    };

    void store_derived(Element * out) {
        store_attribute(datecreated,out);
        store_attribute(datemodified,out);
        store_element(field,out);
        store_element(trait,out);
        store_element(annotation,out);
    }

    // attributes
    optional<datetime> datecreated;
    optional<datetime> datemodified;
    // elements
    zero_more<field> field;
    zero_more<trait> trait;
    zero_more<annotation> annotation;
};

class multitext_extensible : public lift_base {
public:
    multitext_extensible(LPCTSTR name) :
        lift_base(name),
        datecreated(DATE_CREATED),
        datemodified(DATE_MODIFIED),
        form(L"form"),
        text(L"text"),
        field(L"field"),
        trait(L"trait"),
        annotation(ANNOTATION) {
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
    }

    void load_derived(Element * in) {
        load_attribute(datecreated,in);
        load_attribute(datemodified,in);
        load_element(form,in);
        load_element(text,in);
        load_element(field,in);
        load_element(trait,in);
        load_element(annotation,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        return out;
    }

    void store_derived(Element * out) {
        store_attribute(datecreated,out);
        store_attribute(datemodified,out);
        store_element(form,out);
        store_element(text,out);
        store_element(field,out);
        store_element(trait,out);
        store_element(annotation,out);
    }

    // attributes
    optional<datetime> datecreated;
    optional<datetime> datemodified;
    // elements
    zero_more<form> form;
    optional<wstring> text;
    zero_more<field> field;
    zero_more<trait> trait;
    zero_more<annotation> annotation;
};

class range_element : public extensible {
public:
    range_element(LPCTSTR name) :
        extensible(name),
        id(L"id"),
        parent(L"parent"),
        guid(L"guid"),
        description(L"description"),
        label(L"label"),
        abbrev(L"abbrev") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(id,in);
        load_attribute(parent,in);
        load_attribute(guid,in);
        load_element(description,in);
        load_element(label,in);
        load_element(abbrev,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(id,out);
        store_attribute(parent,out);
        store_attribute(guid,out);
        store_element(description,out);
        store_element(label,out);
        store_element(abbrev,out);
        return out;
    };

    // attributes
    required<key> id;
    optional<key> parent;
    optional<wstring> guid;
    // elements
    zero_more<multitext> description;
    zero_more<multitext> label;
    zero_more<multitext> abbrev;
};

class range : public extensible {
public:
    range(LPCTSTR name) :
        extensible(name),
        id(L"id"),
        guid(L"guid"),
        href(L"href"),
        description(L"description"),
        range_element(L"range-element"),
        label(L"label"),
        abbrev(L"abbrev") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(id,in);
        load_attribute(guid,in);
        load_attribute(href,in);
        load_element(description,in);
        load_element(range_element,in);
        load_element(label,in);
        load_element(abbrev,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(id,out);
        store_attribute(guid,out);
        store_attribute(href,out);
        store_element(description,out);
        store_element(range_element,out);
        store_element(label,out);
        store_element(abbrev,out);
        return out;
    };

    // attributes
    required<key> id;
    optional<wstring> guid;
    optional<url> href;
    // elements
    zero_more<multitext> description;
    zero_more<range_element> range_element;
    zero_more<multitext> label;
    zero_more<multitext> abbrev;
};

class lift_ranges : public lift_base {
public:
    lift_ranges() :
        lift_base(L"lift-ranges"),
        range(L"range") {
    };

    lift_ranges(LPCTSTR name) :
        lift_base(name),
        range(L"range") {
    };

    void load(Element * in) {
        expect(in,name);
        load_element(range,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_element(range,out);
        return out;
    };

    // attributes
    zero_more<range> range;
};

class gloss : public form {
public:
    gloss(LPCTSTR _name) :
        form(_name),
        trait(L"trait") {
    };

    void load(Element * in) {
        expect(in,name);
        form::load_derived(in);
        load_element(trait,in);
    };

    Element * store() {
        Element * out = new Element(name);
        form::store_derived(out);
        store_element(trait,out);
        return out;
    };

    zero_more<trait> trait;
};

class urlref : public lift_base {
public:
    urlref(LPCTSTR name) :
        lift_base(name),
        href(L"href"),
        label(L"label") {
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(href,in);
        load_element(label,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_attribute(href,out);
        store_element(label,out);
        return out;
    };

    // attributes
    required<url> href;
    // element
    optional<multitext> label;
};

class field_defns;

class ranges : public lift_base {
public:
    ranges(LPCTSTR name) :
        lift_base(name),
        range(L"range") {
    };

    void load(Element * in) {
        expect(in,name);
        load_element(range,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_element(range,out);
        return out;
    };

    // attributes
    // elements
    zero_more<range> range;
};

class header : lift_base {
public:
    header(LPCTSTR varname) :
        lift_base(varname),
        description(L"description"),
        ranges(L"ranges"),
        fields(L"fields") {
    }

    void load(Element * in) {
        expect(in,name);
        load_element(description,in);
        load_element(ranges,in);
        load_element(fields,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_element(description,out);
        store_element(ranges,out);
        store_element(fields,out);
        return out;
    };

    // attributes
    // elements
    optional<multitext> description;
    optional<ranges> ranges;
    optional<field_defns> fields;
};

class note : public multitext_extensible {
public:
    note(LPCTSTR name) :
        multitext_extensible(name),
        type(L"type") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext_extensible::load_derived(in);
        load_attribute(type,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext_extensible::store_derived(out);
        store_attribute(type,out);
        return out;
    };

    // attributes
    optional<key> type;
};

class phonetic : public multitext_extensible {
public:
    phonetic(LPCTSTR name) :
        multitext_extensible(name),
        media(L"media"),
        form(L"form") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext_extensible::load_derived(in);
        load_element(media,in);
        load_element(form,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext_extensible::store_derived(out);
        store_element(media,out);
        store_element(form,out);
        return out;
    };

    // attributes
    // elements
    zero_more<urlref> media;
    zero_more<span> form;
};

class etymology : public extensible {
public:
    etymology(LPCTSTR name) :
        extensible(name),
        type(L"type"),
        source(SOURCE),
        gloss(L"gloss"),
        form(L"form") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(type,in);
        load_attribute(source,in);
        load_element(gloss,in);
        load_element(form,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(type,out);
        store_attribute(source,out);
        store_element(gloss,out);
        store_element(form,out);
        return out;
    };

    // attributes
    required<key> type;
    required<wstring> source;
    // elements
    zero_more<form> gloss;
    optional<form> form;
};

class grammatical_info : public lift_base {
public:
    grammatical_info(LPCTSTR _name) :
        lift_base(_name),
        value(L"value"),
        trait(L"trait") {
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(value,in);
        load_element(trait,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_attribute(value,out);
        store_element(trait,out);
        return out;
    }

    // attributes
    required<key> value;
    // elements
    zero_more<trait> trait;
};

class reversal : public multitext {
public:
    reversal(LPCTSTR varname) :
        multitext(varname),
        type(L"type"),
        main(L"main"),
        grammatical_info(L"grammatical-info") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext::load_derived(in);
        load_attribute(type,in);
        load_element(main,in);
        load_element(grammatical_info,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext::store_derived(out);
        store_attribute(type,out);
        store_element(main,out);
        store_element(grammatical_info,out);
        return out;
    };

    // attributes
    optional<key> type;
    // elements
    optional<reversal> main;
    optional<grammatical_info> grammatical_info;
};

class translation : public multitext {
public:
    translation(LPCTSTR name) :
        multitext(name),
        type(L"type") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext::load_derived(in);
        load_attribute(type,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext::store_derived(out);
        store_attribute(type,out);
        return out;
    };

    // attributes
    optional<key> type;
};

class example : public multitext_extensible {
public:
    example(LPCTSTR varname) :
        multitext_extensible(varname),
        source(SOURCE),
        translation(L"translation") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext_extensible::load_derived(in);
        load_attribute(source,in);
        load_element(translation,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext_extensible::store_derived(out);
        store_attribute(source,out);
        store_element(translation,out);
        return out;
    };

    // attributes
    optional<key> source;
    // elements
    zero_more<translation> translation;
};

class relation : public extensible {
public:
    relation(LPCTSTR varname) :
        extensible(varname),
        type(L"type"),
        ref(L"ref"),
        order(L"order"),
        usage(L"usage") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(type,in);
        load_attribute(ref,in);
        load_attribute(order,in);
        load_element(usage,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(type,out);
        store_attribute(ref,out);
        store_attribute(order,out);
        store_element(usage,out);
        return out;
    };

    // attributes
    required<key> type;
    required<refid> ref;
    optional<int> order;
    optional<multitext> usage;

    // elements
};

class variant : public multitext_extensible {
public:
    variant(LPCTSTR varname) :
        multitext_extensible(varname),
        ref(L"ref"),
        pronunciation(L"pronunciation"),
        relation(L"relation") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext_extensible::load_derived(in);
        load_attribute(ref,in);
        load_element(pronunciation,in);
        load_element(relation,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext_extensible::store_derived(out);
        store_attribute(ref,out);
        store_element(pronunciation,out);
        store_element(relation,out);
        return out;
    };

    // attributes
    optional<refid> ref;
    // elements
    zero_more<phonetic> pronunciation;
    zero_more<relation> relation;
};

class sense : public extensible {
public:
    sense(LPCTSTR varname) :
        extensible(varname),
        id(L"id"),
        order(L"order"),
        grammatical_info(L"grammatical-info"),
        gloss(L"gloss"),
        definition(L"definition"),
        relation(L"relation"),
        note(L"note"),
        example(L"example"),
        reversal(L"reversal"),
        illustration(L"illustration"),
        subsense(L"subsense") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(id,in);
        load_attribute(order,in);
        load_element(grammatical_info,in);
        load_element(gloss,in);
        load_element(definition,in);
        load_element(relation,in);
        load_element(note,in);
        load_element(example,in);
        load_element(reversal,in);
        load_element(illustration,in);
        load_element(subsense,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(id,out);
        store_attribute(order,out);
        store_element(grammatical_info,out);
        store_element(gloss,out);
        store_element(definition,out);
        store_element(relation,out);
        store_element(note,out);
        store_element(example,out);
        store_element(reversal,out);
        store_element(illustration,out);
        store_element(subsense,out);
        return out;
    };

    // attributes
    optional<refid> id;
    optional<int> order;
    // elements
    optional<grammatical_info> grammatical_info;
    zero_more<gloss> gloss;
    optional<multitext> definition;
    zero_more<relation> relation;
    zero_more<note> note;
    zero_more<example> example;
    zero_more<reversal> reversal;
    zero_more<urlref> illustration;
    zero_more<sense> subsense;
};

class entry : public extensible {
public:
    entry(LPCTSTR varname) :
        extensible(varname),
        id(L"id"),
        guid(L"guid"),
        order(L"order"),
        dateDeleted(L"dateDeleted"),
        lexical_unit(L"lexical-unit"),
        citation(L"citation"),
        pronunciation(L"pronunciation"),
        variant(L"variant"),
        sense(L"sense"),
        note(L"note"),
        relation(L"relation"),
        etymology(L"etymology"),
        literal_meaning(L"literal-meaning") {
    };

    void load(Element * in) {
        expect(in,name);
        extensible::load_derived(in);
        load_attribute(id,in);
        load_attribute(guid,in);
        load_attribute(order,in);
        load_attribute(dateDeleted,in);
        load_element(lexical_unit,in);
        load_element(citation,in);
        load_element(pronunciation,in);
        load_element(variant,in);
        load_element(sense,in);
        load_element(note,in);
        load_element(relation,in);
        load_element(etymology,in);
        load_element(literal_meaning,in);
    };

    Element * store() {
        Element * out = new Element(name);
        extensible::store_derived(out);
        store_attribute(id,out);
        store_attribute(guid,out);
        store_attribute(order,out);
        store_attribute(dateDeleted,out);
        store_element(lexical_unit,out);
        store_element(citation,out);
        store_element(pronunciation,out);
        store_element(variant,out);
        store_element(sense,out);
        store_element(note,out);
        store_element(relation,out);
        store_element(etymology,out);
        store_element(literal_meaning,out);
        return out;
    };

    // attributes
    optional<refid> id;
    optional<int> order;
    optional<wstring> guid;
    optional<datetime> dateDeleted;
    // elements
    optional<multitext> lexical_unit;
    optional<multitext> citation;
    zero_more<phonetic> pronunciation;
    zero_more<variant> variant;
    zero_more<sense> sense;
    zero_more<note> note;
    zero_more<relation> relation;
    zero_more<etymology> etymology;
    optional<multitext> literal_meaning;
};

class multitext_extensible;

class field_defn : multitext_extensible {
public:
    field_defn(LPCTSTR name) :
        multitext_extensible(name),
        tag(L"tag") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext_extensible::load_derived(in);
        load_attribute(tag,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext_extensible::store_derived(out);
        store_attribute(tag,out);
        return out;
    };

    required<key> tag;
};

class field_defns : lift_base {
public:
    field_defns(LPCTSTR name) :
        lift_base(name),
        field(L"field") {
    };

    void load(Element * in) {
        expect(in,name);
        load_element(field,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_element(field,out);
        return out;
    };

    zero_more<field_defn> field;
};

class lift : lift_base {
public:
    lift() :
        lift_base(L"lift"),
        version(L"version"),
        producer(L"producer"),
        header(L"header"),
        entry(L"entry") {
        version = wstring(L"0.14");
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(version,in);
        load_attribute(producer,in);
        load_element(header,in);
        load_element(entry,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_attribute(version,out);
        store_attribute(producer,out);
        store_element(header,out);
        store_element(entry,out);
        return out;
    };

    // atributes
    required<wstring> version;
    optional<wstring> producer;
    // elements
    optional<header> header;
    zero_more<entry> entry;
};

lift load(LPCTSTR filename);
lift_ranges load_ranges(LPCTSTR filename);
void store(lift & root, LPCTSTR filename);

}
#endif
