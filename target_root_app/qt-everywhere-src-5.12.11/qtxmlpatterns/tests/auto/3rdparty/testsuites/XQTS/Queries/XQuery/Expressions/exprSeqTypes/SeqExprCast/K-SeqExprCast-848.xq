(:*******************************************************:)
(: Test: K-SeqExprCast-848                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:time. :)
(:*******************************************************:)
xs:time("
	 03:20:00-05:00
	 ")
        eq
        xs:time("
	 03:20:00-05:00
	 ")
      