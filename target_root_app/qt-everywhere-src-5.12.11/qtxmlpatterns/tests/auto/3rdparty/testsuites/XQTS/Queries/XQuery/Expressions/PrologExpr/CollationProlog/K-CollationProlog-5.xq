(:*******************************************************:)
(: Test: K-CollationProlog-5                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Two 'declare default collation' declarations where the collations differs. :)
(:*******************************************************:)

	declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";
	declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint2";
	default-collation() eq "http://www.w3.org/2005/xpath-functions/collation/codepoint"