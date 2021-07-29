(:*******************************************************:)
(: Test: K-SubstringBeforeFunc-5                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `substring-before("foo", "oo", "http://www.w3.org/2005/xpath-functions/collation/codepoint") eq "f"`. :)
(:*******************************************************:)
substring-before("foo", "oo",
			"http://www.w3.org/2005/xpath-functions/collation/codepoint") eq "f"