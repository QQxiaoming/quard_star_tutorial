(:*******************************************************:)
(: Test: K-SeqExprCast-1056                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:gMonthDay. :)
(:*******************************************************:)
xs:gMonthDay("
	 --11-13
	 ")
        eq
        xs:gMonthDay("
	 --11-13
	 ")
      