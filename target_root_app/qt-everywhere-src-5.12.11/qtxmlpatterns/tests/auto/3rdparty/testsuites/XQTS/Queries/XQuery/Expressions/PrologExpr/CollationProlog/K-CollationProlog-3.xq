(:*******************************************************:)
(: Test: K-CollationProlog-3                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Any implementation must support setting the default collation to the Unicode Codepoint collation with 'declare default collation'. :)
(:*******************************************************:)

	declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";
	default-collation() eq "http://www.w3.org/2005/xpath-functions/collation/codepoint"