(:*******************************************************:)
(: Test: K-SeqExprCast-1160                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:gMonth. :)
(:*******************************************************:)
xs:gMonth("
	 --11
	 ")
        eq
        xs:gMonth("
	 --11
	 ")
      