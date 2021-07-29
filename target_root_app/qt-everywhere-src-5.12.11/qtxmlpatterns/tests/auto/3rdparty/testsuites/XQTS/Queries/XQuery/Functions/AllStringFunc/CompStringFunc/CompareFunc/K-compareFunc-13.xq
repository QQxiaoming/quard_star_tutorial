(:*******************************************************:)
(: Test: K-compareFunc-13                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `empty(compare("a string", (), "http://www.w3.org/2005/xpath-functions/collation/codepoint"))`. :)
(:*******************************************************:)
empty(compare("a string", (),
			"http://www.w3.org/2005/xpath-functions/collation/codepoint"))