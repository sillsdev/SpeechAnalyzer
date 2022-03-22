#ifndef LIFT13_H
#define LIFT13_H

#include <string>
#include <list>
#include <XMLUtils.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <assert.h>
#include "Lift.h"
#include <memory>
#include <map>

using std::map;
using std::stringstream;
using std::vector;
using std::list;
using std::wstring;
using std::exception;
using namespace Lift;

namespace Lift13 {

class multitext;
class lift_ranges;

typedef map<wstring,lift_ranges> lift_range_map;

class span : public lift_base {
public:
    span(LPCTSTR _name) :
        lift_base(_name),
        lang(L"lang"),
        href(L"href"),
        clazz(CLAZZ),
		zmspan(SPAN) {
    };

    span(LPCTSTR _name, LPCTSTR _data) :
        lift_base(_name),
        lang(L"lang"),
        href(L"href"),
        clazz(CLAZZ),
		zmspan(SPAN) {
        pcdata = _data;
    };

    span(LPCTSTR _name, LPCTSTR _lang, LPCTSTR _data) :
        lift_base(_name),
        lang(L"lang"),
        href(L"href"),
        clazz(CLAZZ),
		zmspan(SPAN) {
        lang = wstring(_lang);
		pcdata = _data;
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(lang,in);
        load_attribute(href,in);
        load_attribute(clazz,in);
        load_value(pcdata,in);
        load_element(zmspan,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_attribute(lang,out);
        store_attribute(href,out);
        store_attribute(clazz,out);
        store_value(pcdata,out);
        store_element(zmspan,out);
        return out;
    };

   bool operator==(const span & right) const {
		if (lang!=right.lang) return false;
		if (href!=right.href) return false;
		if (clazz!=right.clazz) return false;
		if (zmspan !=right.zmspan) return false;
		if (pcdata.compare(right.pcdata)!=0) return false;
		return true;
    }

    // attributes
    optional<lang> lang;
    optional<href> href;
    optional<wstring> clazz;
    // elements
    zero_more<span> zmspan;
    // data
    wstring pcdata;
};

class annotation;

class text : public lift_base {
public:
    text(LPCTSTR _name, LPCTSTR data) :
        lift_base(_name),
        zmspan(SPAN) {
        pcdata = data;
    };

    text(LPCTSTR _name) :
        lift_base(_name),
        zmspan(SPAN) {
    };

    text(LPCTSTR _name, span _span) :
        lift_base(_name),
        zmspan(SPAN) {
        zmspan.append(_span);
    };

    void load(Element * in) {
        expect(in,name);
        load_derived(in);
        load_value(pcdata, in);
    }

    void load_derived(Element * in) {
        load_element(zmspan,in);
    }

    Element * store() {
        Element * out = new Element(name);
        store_derived(out);
        store_value(pcdata, out);
        return out;
    }

    void store_derived(Element * out) {
        store_element(zmspan,out);
    }

    bool operator!=(const text & right) const {
        if (zmspan !=right.zmspan) {
            return true;
        }
        return false;
    }

    bool operator==(const text & right) const {
		if (zmspan !=right.zmspan) {
			return false;
		}
		return true;
    }

    // elements
    zero_more<span> zmspan;
    // data
    wstring pcdata;
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

    bool operator==(const multitext & right) {
        if (form!=right.form) {
            return false;
        }
        if (trait!=right.trait) {
            return false;
        }
        return true;
    }

    // elements
    zero_more<form> form;
    zero_more<trait> trait;
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

    bool operator==(const annotation & right) const {
        if (name!=right.name) {
            return false;
        }
        if (value!=right.value) {
            return false;
        }
        if (who!=right.who) {
            return false;
        }
        if (when!=right.when) {
            return false;
        }
        return true;
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
        text(LTEXT),
        annotation(ANNOTATION) {
    };

    form(LPCTSTR _name, LPCTSTR _lang, text _text) :
        lift_base(_name),
        lang(L"lang"),
        text(LTEXT),
        annotation(ANNOTATION) {
        lang = wstring(_lang);
        text = _text;
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

    bool operator==(const form & right) const {
        if (lang!=right.lang) {
            return false;
        }
        if (text!=right.text) {
            return false;
        }
        if (annotation!=right.annotation) {
            return false;
        }
        return true;
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

    bool operator==(const trait & right) const {
        if (name!=right.name) {
            return false;
        }
        if (value!=right.value) {
            return false;
        }
        if (id!=right.id) {
            return false;
        }
        if (annotation!=right.annotation) {
            return false;
        }
        return true;
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

    field(LPCTSTR _name, key _type) :
        multitext(_name),
        type(L"type"),
        datecreated(DATE_CREATED),
        datemodified(DATE_MODIFIED),
        //trait(L"trait"),
        form(L"form"),
        annotation(ANNOTATION)  {
        type = _type;
        datecreated = createDate();
        datemodified = createDate();
    }

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
    zero_more<Lift13::form> form;
    zero_more<annotation> annotation;
};

class extensible : public lift_base {
public:
    extensible(LPCTSTR name) :
        lift_base(name),
        date_created(DATE_CREATED),
        date_modified(DATE_MODIFIED),
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
        load_attribute(date_created,in);
        load_attribute(date_modified,in);
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
        store_attribute(date_created,out);
        store_attribute(date_modified,out);
        store_element(field,out);
        store_element(trait,out);
        store_element(annotation,out);
    }

    // attributes
    optional<datetime> date_created;
    optional<datetime> date_modified;
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
        text(LTEXT),
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

class range_element : public lift_base {
public:
    range_element(LPCTSTR name) :
        lift_base(name),
        id(L"id"),
        parent(L"parent"),
        guid(L"guid"),
        description(L"description"),
        label(L"label"),
        abbrev(L"abbrev") {
    };

    void load(Element * in) {
        expect(in,name);
        load_attribute(id,in);
        load_attribute(parent,in);
        load_attribute(guid,in);
        load_element(description,in);
        load_element(label,in);
        load_element(abbrev,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_attribute(id,out);
        store_attribute(parent,out);
        store_attribute(guid,out);
        store_element(description,out);
        store_element(label,out);
        store_element(abbrev,out);
        return out;
    };

    /*
    * Accept external data from an external
    * lift_ranges file.  our data overrides the incoming data.
    * we've already compared ids
    */
    void add(range_element & right) {
        parent.add(right.parent);
        guid.add(right.guid);
        description.add(right.description);
        label.add(right.label);
        abbrev.add(right.abbrev);
    }

    // attributes
    required<key> id;
    optional<key> parent;
    optional<wstring> guid;
    // elements
    zero_more<multitext> description;
    zero_more<multitext> label;
    zero_more<multitext> abbrev;
};

class range;

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

class range : public lift_base {
public:
    range(LPCTSTR name) :
        lift_base(name),
        id(L"id"),
        guid(L"guid"),
        href(L"href"),
        description(L"description"),
        range_element(L"range-element"),
        label(L"label"),
        abbrev(L"abbrev") {
    };

    void load(Element * in) {
        printf("loading range %p\n",in);
        expect(in,name);
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
        store_attribute(id,out);
        store_attribute(guid,out);
        store_attribute(href,out);
        store_element(description,out);
        store_element(range_element,out);
        store_element(label,out);
        store_element(abbrev,out);
        return out;
    };

    void apply_lift_ranges(lift_range_map & range_map) {

        // no href? nothing to do...
        if (href.size()==0) {
            return;
        }
        // this href is not in this map
        if (range_map.find(href.get()) == range_map.end()) {
            return;
        }
        lift_ranges & lift_ranges = range_map[href.get()];
        // there is no range data available
        if (lift_ranges.range.size()==0) {
            return;
        }
        for (size_t i=0; i<lift_ranges.range.size(); i++) {

            range & right = lift_ranges.range[i];
            // this this the correct ID?
            if (id != right.id) {
                continue;
            }
            // we if don't have anything, just append the external data
            if (range_element.size()==0) {
                for (size_t j=0; j<right.range_element.size(); j++) {
                    range_element.append(right.range_element[j]);
                }
                continue;
            }
            // we already have something?
            for (size_t j=0; j<right.range_element.size(); j++) {
                bool found = false;
                Lift13::range_element & rre = right.range_element[j];
                for (size_t k=0; k<range_element.size(); k++) {
                    Lift13::range_element & re = range_element[k];
                    if (re.id == rre.id) {
                        // carefully merge the item
                        re.add(rre);
                        // we are done with this loop
                        found=true;
                        break;
                    }
                }
                if (!found) {
                    // just append it
                    range_element.append(rre);
                }
            }
        }
    }

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

class gloss : public form {
public:
    gloss(LPCTSTR _name) :
        form(_name),
        trait(L"trait") {
    };

    gloss(LPCTSTR _name, LPCTSTR _lang, LPCTSTR _data) : //text _text) : //LPCTSTR _lang, LPCTSTR _data) :
        form(_name, _lang, Lift13::text(LTEXT, _data)),
        trait(L"trait") {
    };

    gloss(LPCTSTR _name, LPCTSTR _lang, span _span) :
        form(_name, _lang, Lift13::text(LTEXT,_span)),
        trait(L"trait") {
    };

    void load(Element * in) {
        form::load_derived(in);
        expect(in,name);
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
        printf("loading ranges %p\n",in);
        expect(in,name);
        load_element(range,in);
    };

    Element * store() {
        Element * out = new Element(name);
        store_element(range,out);
        return out;
    };

    lift_ranges export_lift_ranges(LPCTSTR filename) {
        lift_ranges result;
        if (range.size()==0) {
            return result;
        }
        for (size_t i=0; i<range.size(); i++) {
            range[i].href.remove();
            result.range.append(range[i]);
            wstring href;
            href.append(L"file:\\");
            href.append(filename);
            range[i].href = href;
            range[i].range_element.remove();
        }
        return result;
    }

    void apply_lift_ranges(lift_range_map & range_map) {
        if (range.size()==0) {
            return;
        }
        for (size_t i=0; i<range.size(); i++) {
            printf("applying range %d\n",i);
            range[i].apply_lift_ranges(range_map);
            range[i].href.get()=L"";
        }
    }

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

    lift_ranges export_lift_ranges(LPCTSTR filename) {
        if (ranges.size()==0) {
            return lift_ranges();
        }
        return ranges.get().export_lift_ranges(filename);
    }

    void apply_lift_ranges(lift_range_map & range_map) {
        if (ranges.size()==0) {
            return;
        }
        ranges.get().apply_lift_ranges(range_map);
    }

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
    zero_more<Lift13::form> form;
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
    required<form> form;
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
    sense(LPCTSTR _name) :
        extensible(_name),
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

    sense(LPCTSTR _name, LPCTSTR _id, int _order) :
        extensible(_name),
        id(L"id",wstring(_id)),
        order(L"order",_order),
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
        date_deleted(L"dateDeleted"),
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
        load_attribute(date_deleted,in);
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
        store_attribute(date_deleted,out);
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
    optional<datetime> date_deleted;
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

class field_defn : public multitext {
public:
    field_defn(LPCTSTR name) :
        multitext(name),
        tag(L"tag") {
    };

    void load(Element * in) {
        expect(in,name);
        multitext::load_derived(in);
        load_attribute(tag,in);
    };

    Element * store() {
        Element * out = new Element(name);
        multitext::store_derived(out);
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
        version = wstring(L"0.13");
    };

    lift(LPCTSTR _producer) :
        lift_base(L"lift"),
        version(L"version"),
        producer(L"producer"),
        header(L"header"),
        entry(L"entry") {
        version = wstring(L"0.13");
        producer = wstring(_producer);
    };


    void load(Element * in) {
        expect(in,name);
        load_attribute(version,in);
        load_attribute(producer,in);
        load_element(header,in);
        load_element(entry,in);
    };

    Element * store() {
        Element * element = new Element(name);
        store_attribute(version,element);
        store_attribute(producer,element);
        store_element(header,element);
        store_element(entry,element);
        return element;
    };

    lift_range_map get_external_range_refs() {

        lift_range_map hrefs;
        if (header.size()==0) {
            return hrefs;
        }
        if (header.get().ranges.size()==0) {
            return hrefs;
        }
        if (header.get().ranges.get().range.size()>0) {
            for (size_t i=0; i<header.get().ranges.get().range.size(); i++) {
                if (header.get().ranges.get().range[i].href.size()>0) {
                    wstring key = header.get().ranges.get().range[i].href.get();
                    hrefs[key] = lift_ranges();
                }
            }
        }
        return hrefs;
    }

    lift_ranges export_lift_ranges(LPCTSTR filename) {
        if (header.size()==0) {
            return lift_ranges();
        }
        return header.get().export_lift_ranges(filename);
    }

    void apply_lift_ranges(lift_range_map & range_map) {
        if (header.size()==0) {
            return;
        }
        header.get().apply_lift_ranges(range_map);
    }

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

void write_document(Document & doc, LPCTSTR filename);

}
#endif
