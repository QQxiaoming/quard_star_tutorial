(:*******************************************************:)
(: Test: K-ContainsFunc-3                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `contains((), (), "http://www.w3.org/2005/xpath-functions/collation/codepoint", "wrong param")`. :)
(:*******************************************************:)
contains((), (),
			"http://www.w3.org/2005/xpath-functions/collation/codepoint",
			"wrong param")