(:*******************************************************:)
(: Test: K-CollationProlog-4                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A 'declare default collation' expression may occur only once. :)
(:*******************************************************:)

	declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";
	declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";
	default-collation() eq "http://www.w3.org/2005/xpath-functions/collation/codepoint"