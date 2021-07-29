(:*******************************************************:)
(: Test: K-SeqExprCast-640                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:duration. :)
(:*******************************************************:)
xs:duration("
	 P1Y2M3DT10H30M
	 ")
        eq
        xs:duration("
	 P1Y2M3DT10H30M
	 ")
      