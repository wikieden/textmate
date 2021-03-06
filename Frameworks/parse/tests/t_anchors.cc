#include "support.cc"
#include <test/bundle_index.h>

static bundles::item_ptr AnchorTestGrammarItem;
static bundles::item_ptr AnchorInCapturesTestGrammarItem;

static class AnchorGrammarFixture : public CxxTest::GlobalFixture
{
public:
	bool setUpWorld()
	{
		static std::string AnchorTestLanguageGrammar =
			"{ name           = 'Test';"
			"  patterns       = ("
			"    { name = 'bof'; match = '\\Axy'; },"
			"    { name = 'bom'; match = '\\Gxy'; },"
			"    { name = 'eof'; match = 'xy\\z'; },"
			"    { begin = '\\['; end = '\\]';"
			"      patterns = ("
			"        { name = 'bom'; match = '\\Axy'; },"
			"        { name = 'bom'; match = '\\Gxy'; },"
			"        { name = 'bom'; match = 'xy\\z'; },"
			"      );"
			"    },"
			"  );"
			"  scopeName      = 'test';"
			"  uuid           = 'F159C30D-9EE7-4A30-AE30-E26EF3BCCC17';"
			"}";

		static std::string AnchorInCapturesTestLanguageGrammar =
			"{ name     = 'Test';"
			"  patterns = ("
			"    { match = '> (.+)';"
			"      name = 'gt';"
			"      captures = { 1 = { patterns = ( { include = '#captures'; } ); }; };"
			"    },"
			"    { match = '(.+) <';"
			"      name = 'lt';"
			"      captures = { 1 = { patterns = ( { include = '#captures'; } ); }; };"
			"    },"
			"    { match = '.+\\z';"
			"      name = 'tail';"
			"      captures = { 0 = { patterns = ( { include = '#captures'; } ); }; };"
			"    },"
			"    { match = '\\A.+';"
			"      name = 'head';"
			"      captures = { 0 = { patterns = ( { include = '#captures'; } ); }; };"
			"    },"
			"    { match = '.+';"
			"      name = 'line';"
			"      captures = { 0 = { patterns = ( { include = '#captures'; } ); }; };"
			"    },"
			"  );"
			"  repository = {"
			"    captures = {"
			"      patterns = ("
			"        { match = '\\A\\w+';"
			"          name = 'b-buf';"
			"        },"
			"        { match = '^\\w+';"
			"          name = 'b-line';"
			"        },"
			"        { match = '\\G\\w+';"
			"          name = 'b-cap';"
			"        },"
			"        { match = '\\w+\\z';"
			"          name = 'e-buf';"
			"        },"
			"        { match = '\\w+$';"
			"          name = 'e-line';"
			"        },"
			"        { match = '\\w+\\Z';"
			"          name = 'e-cap';"
			"        },"
			"      );"
			"    };"
			"  };"
			"  scopeName      = 'test';"
			"  uuid           = '5C80DB53-F519-494F-BA8D-C8D80540E9E4';"
			"}";

		test::bundle_index_t bundleIndex;
		AnchorTestGrammarItem           = bundleIndex.add(bundles::kItemTypeGrammar, AnchorTestLanguageGrammar);
		AnchorInCapturesTestGrammarItem = bundleIndex.add(bundles::kItemTypeGrammar, AnchorInCapturesTestLanguageGrammar);
		return bundleIndex.commit();
	}

} anchor_grammar_fixture;

class AnchorsTests : public CxxTest::TestSuite
{
public:
	void test_anchors ()
	{
		auto grammar = parse::parse_grammar(AnchorTestGrammarItem);

		TS_ASSERT_EQUALS(markup(grammar, "xy xy\nxy xy\n[xy xy\nxy xy]\nxy xy"), "«test»«bof»xy«/bof» xy\nxy xy\n[«bom»xy«/bom» xy\nxy xy]\nxy «eof»xy«/eof»«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "xy xy"),                               "«test»«bof»xy«/bof» «eof»xy«/eof»«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "xy xy\n"),                             "«test»«bof»xy«/bof» xy\n«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "[xy xy]"),                             "«test»[«bom»xy«/bom» xy]«/test»");
	}

	void test_anchor_in_captures ()
	{
		auto grammar = parse::parse_grammar(AnchorInCapturesTestGrammarItem);
		TS_ASSERT_EQUALS(markup(grammar, "foo\n"),        "«test»«head»«b-buf»foo«/b-buf»«/head»\n«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "> foo\n"),      "«test»«gt»> «b-cap»foo«/b-cap»«/gt»\n«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "foo <\n"),      "«test»«lt»«b-buf»foo«/b-buf» <«/lt»\n«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "\nfoo\n"),      "«test»\n«line»«b-line»foo«/b-line»«/line»\n«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "\nfoo"),        "«test»\n«tail»«b-line»foo«/b-line»«/tail»«/test»");
		TS_ASSERT_EQUALS(markup(grammar, "\nfoo bar"),    "«test»\n«tail»«b-line»foo«/b-line» «e-buf»bar«/e-buf»«/tail»«/test»");

		// TS_ASSERT_EQUALS(markup(grammar, "foo bar\n"),    "«test»«head»«b-buf»foo«/b-buf» «e-line»bar«/e-line»«/head»\n«/test»");
		// TS_ASSERT_EQUALS(markup(grammar, "> foo bar\n"),  "«test»«gt»> «b-cap»foo«/b-cap» «e-line»bar«/e-line»«/gt»\n«/test»");
		// TS_ASSERT_EQUALS(markup(grammar, "foo bar <\n"),  "«test»«lt»«b-buf»foo«/b-buf» «e-cap»bar«/e-cap» <«/lt»\n«/test»");
		// TS_ASSERT_EQUALS(markup(grammar, "\nfoo bar\n"),  "«test»\n«line»«b-line»foo«/b-line» «e-line»bar«/e-line»«/line»\n«/test»");
	}
};
