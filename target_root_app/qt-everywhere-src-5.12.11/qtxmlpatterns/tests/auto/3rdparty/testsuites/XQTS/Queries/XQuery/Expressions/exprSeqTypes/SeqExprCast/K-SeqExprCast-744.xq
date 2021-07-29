(:*******************************************************:)
(: Test: K-SeqExprCast-744                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:dayTimeDuration. :)
(:*******************************************************:)
xs:dayTimeDuration("
	 P3DT2H
	 ")
        eq
        xs:dayTimeDuration("
	 P3DT2H
	 ")
      