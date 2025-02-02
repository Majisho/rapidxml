#include "../test_utils.hpp"
#include "../../rapidxml_utils.hpp"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4127)   // Conditional expression is constant
#endif

using namespace std;
using namespace rapidxml;

template<int Flags>
void test_document_node()
{
    cout << "Test document node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_element.xml");
    doc.parse<Flags>(xml.data());

    CHECK(doc.type() == node_document);
    CHECK(doc.parent() == 0);
    CHECK(*doc.name() == 0);
    CHECK(*doc.value() == 0);
}

template<int Flags>
void test_element_node()
{
    cout << "Test element node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_element.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *element = doc.first_node();
    REQUIRE(element);
    REQUIRE(element->type() == node_element);
    CHECK(element->parent() == &doc);
    CHECK(name<Flags>(element) == "root");
    CHECK(element->name_size() == 4);
    CHECK(value<Flags>(element) == "foobar");
    CHECK(element->value_size() == 6);
    if (Flags & parse_no_data_nodes)
    {
        CHECK(count_children(element) == 1);
    }
    else
    {
        CHECK(count_children(element) == 2);
    }
    CHECK(count_children(element, "interior") == 1);
    CHECK(count_children(element, "invalid") == 0);
    CHECK(count_attributes(element) == 0);
    CHECK(element->next_sibling() == 0);
    CHECK(element->previous_sibling() == 0);
}

template<int Flags>
void test_attribute_node()
{
    cout << "Test attribute node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_attributes.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *element = doc.first_node();
    REQUIRE(element);
    CHECK(count_attributes(element) == 7);
    xml_attribute<char> *attr = element->first_attribute();
    CHECK(attr->parent() == element);
    CHECK(name<Flags>(attr) == "att1");
    CHECK(attr->name_size() == 4);
    CHECK(value<Flags>(attr) == "value1");
    CHECK(attr->value_size() == 6);
    attr = element->last_attribute();
    CHECK(name<Flags>(attr) == "att7");
    CHECK(attr->name_size() == 4);
    if (Flags & parse_normalize_whitespace)
    {
        CHECK(value<Flags>(attr) == " \r\nfoo  bar\t");  // Whitespace is not normalized in attribute values
        CHECK(attr->value_size() == 12);
    }
    else
    {
        CHECK(value<Flags>(attr) == " \r\nfoo  bar\t");
        CHECK(attr->value_size() == 12);
    }
}

template<int Flags>
void test_data_node()
{
    cout << "Test data node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_data.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *element = doc.first_node();
    REQUIRE(element);
    xml_node<char> *data = element->first_node();

    if (Flags & parse_no_data_nodes)
    {
        CHECK(data == 0);
    }
    else
    {
        REQUIRE(data);
        CHECK(*data->name() == 0);
        CHECK(data->value() == element->value());
        if (Flags & parse_normalize_whitespace)
        {
            if (Flags & parse_trim_whitespace)
            {
                CHECK(value<Flags>(data) == "foo bar");
                CHECK(data->value_size() == 7);
            }
            else
            {
                CHECK(value<Flags>(data) == " foo bar ");
                CHECK(data->value_size() == 9);
            }
        }
        else
        {
            if (Flags & parse_trim_whitespace)
            {
                CHECK(value<Flags>(data) == "foo  bar");
                CHECK(data->value_size() == 8);
            }
            else
            {
                CHECK(value<Flags>(data) == "  foo  bar  ");
                CHECK(data->value_size() == 12);
            }
        }
    }
}

template<int Flags>
void test_cdata_node()
{
    cout << "Test cdata node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_cdata.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *element = doc.first_node();
    REQUIRE(element);
    xml_node<char> *cdata = element->first_node();

    if (Flags & parse_no_data_nodes)
    {
        CHECK(cdata == 0);
    }
    else
    {
        REQUIRE(cdata);
        CHECK(*cdata->name() == 0);
        //CHECK(cdata->value() == element->value());  -> This is not yet implemented
        CHECK(value<Flags>(cdata) == "data<fake>&amp;&#20;</fake>  ");
        CHECK(cdata->value_size() == 29);
    }
}

template<int Flags>
void test_declaration_node()
{
    cout << "Test declaration node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_header.xml");
    doc.parse<Flags>(xml.data());

    if (!(Flags & parse_declaration_node))
    {
        xml_node<char> *node = doc.first_node();
        REQUIRE(node);
        CHECK(node->type() == node_element);
    }
    else
    {
        xml_node<char> *declaration = doc.first_node();
        REQUIRE(declaration);
        CHECK(declaration->type() == node_declaration);
        CHECK(count_attributes(declaration) == 3);
        xml_attribute<char> *attr = declaration->first_attribute("version");
        REQUIRE(attr);
        CHECK(value<Flags>(attr) == "1.0");
        attr = declaration->first_attribute("encoding");
        REQUIRE(attr);
        CHECK(value<Flags>(attr) == "utf-8");
        attr = declaration->first_attribute("standalone");
        REQUIRE(attr);
        CHECK(value<Flags>(attr) == "yes");
    }
}

template<int Flags>
void test_comment_node()
{
    cout << "Test comment node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_comments.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *element = doc.first_node();
    REQUIRE(element);

    if (!(Flags & parse_comment_nodes))
    {
        REQUIRE(element->first_node() == 0);
    }
    else
    {
        xml_node<char> *comment = element->first_node();
        REQUIRE(comment);
        CHECK(comment->type() == node_comment);
        CHECK(*comment->name() == 0);
        CHECK(value<Flags>(comment) == "");
        comment = comment->next_sibling();
        REQUIRE(comment);
        CHECK(comment->type() == node_comment);
        CHECK(*comment->name() == 0);
        CHECK(value<Flags>(comment) == "comment1");
    }

}

template<int Flags>
void test_doctype_node()
{
    cout << "Test doctype node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_doctype.xml");
    doc.parse<Flags>(xml.data());

    if (!(Flags & parse_doctype_node))
    {
        xml_node<char> *node = doc.first_node();
        REQUIRE(node);
        CHECK(node->type() == node_element);
    }
    else
    {
        xml_node<char> *doctype = doc.first_node();
        REQUIRE(doctype);
        CHECK(doctype->type() == node_doctype);
        CHECK(*doctype->name() == 0);
        CHECK(value<Flags>(doctype) == "el1 [\r\n\t<!ELEMENT el1 EMPTY>\r\n]");
    }

}

template<int Flags>
void test_pi_node()
{
    cout << "Test pi node...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_pi.xml");
    doc.parse<Flags>(xml.data());

    if (!(Flags & parse_pi_nodes))
    {
        xml_node<char> *node = doc.first_node();
        REQUIRE(node);
        CHECK(node->type() == node_element);
    }
    else
    {
        xml_node<char> *pi = doc.first_node();
        REQUIRE(pi);
        CHECK(pi->type() == node_pi);
        CHECK(name<Flags>(pi) == "target");
        CHECK(value<Flags>(pi) == "<some><instructions>");
    }
}

template<int Flags>
void test_node_with_char_refs()
{
    cout << "Test node with character references...\n";

    xml_document<char> doc;
    file<char> xml("../xml_files/simple_char_references.xml");
    doc.parse<Flags>(xml.data());

    xml_node<char> *r = doc.first_node();
    REQUIRE(r);
    CHECK(r->type() == node_element);
    CHECK(name<Flags>(r) == "root");
    if (Flags & parse_no_entity_translation)
    {
        if (Flags & parse_normalize_whitespace)
        {
            CHECK(value<Flags>(r) == "&lt;&gt;&amp;&quot;&apos;&#32;&#x2A;&#x2a;");
            CHECK(r->value_size() == 42);
        }
        else
        {
            CHECK(value<Flags>(r) == "\r\n\t&lt;&gt;&amp;&quot;&apos;&#32;&#x2A;&#x2a;\r\n");
            CHECK(r->value_size() == 42+5);
        }
    }
    else if (Flags & parse_trim_whitespace)
    {
        CHECK(value<Flags>(r) == "<>&\"' **");
        CHECK(r->value_size() == 8);
    }
    else if (Flags & parse_normalize_whitespace)
    {
        CHECK(value<Flags>(r) == " <>&\"' ** ");
        CHECK(r->value_size() == 10);
    }
    else
    {
        CHECK(value<Flags>(r) == "\r\n\t<>&\"' **\r\n");
        CHECK(r->value_size() == 8+5);
    }

    xml_attribute<char> *attr = r->first_attribute("attr");
    if (Flags & parse_no_entity_translation)
    {
        CHECK(value<Flags>(attr) == "&lt;&gt;&amp;&quot;&apos;&#32;&#x2A;&#x2a;");
        CHECK(attr->value_size() == 42);
    }
    else
    {
        CHECK(value<Flags>(attr) == "<>&\"' **");
        CHECK(attr->value_size() == 8);
    }

    xml_attribute<char> *attr_bug1 = r->first_attribute("bug1");
    if (Flags & parse_no_entity_translation)
    {
        CHECK(value<Flags>(attr_bug1) == "Abcd, efgh-&gt;LMN");
        CHECK(attr_bug1->value_size() == 18);
    }
    else
    {
        CHECK(value<Flags>(attr_bug1) == "Abcd, efgh->LMN");
        CHECK(attr_bug1->value_size() == 15);
    }
}

template<int Flags>
void test_all()
{
    test_document_node<Flags>();
    test_element_node<Flags>();
    test_attribute_node<Flags>();
    test_data_node<Flags>();
    test_cdata_node<Flags>();
    test_declaration_node<Flags>();
    test_comment_node<Flags>();
    test_doctype_node<Flags>();
    test_pi_node<Flags>();
    test_node_with_char_refs<Flags>();
}

int main()
{
    test_all<parse_fastest>();
    test_all<0>();
    test_all<parse_full>();
    test_all<parse_normalize_whitespace>();
    test_all<parse_trim_whitespace>();
    test_all<parse_trim_whitespace | parse_normalize_whitespace>();
    return test::final_results();
}
