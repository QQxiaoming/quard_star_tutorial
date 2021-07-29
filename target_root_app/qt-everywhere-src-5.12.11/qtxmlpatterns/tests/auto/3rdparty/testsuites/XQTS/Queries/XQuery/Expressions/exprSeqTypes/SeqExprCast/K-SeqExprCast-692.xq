(:*******************************************************:)
(: Test: K-SeqExprCast-692                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:yearMonthDuration. :)
(:*******************************************************:)
xs:yearMonthDuration("
	 P1Y12M
	 ")
        eq
        xs:yearMonthDuration("
	 P1Y12M
	 ")
      