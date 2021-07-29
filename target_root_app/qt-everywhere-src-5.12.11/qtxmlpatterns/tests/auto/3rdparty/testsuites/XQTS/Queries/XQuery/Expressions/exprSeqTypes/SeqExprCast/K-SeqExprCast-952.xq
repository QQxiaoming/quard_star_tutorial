(:*******************************************************:)
(: Test: K-SeqExprCast-952                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:gYearMonth. :)
(:*******************************************************:)
xs:gYearMonth("
	 1999-11
	 ")
        eq
        xs:gYearMonth("
	 1999-11
	 ")
      