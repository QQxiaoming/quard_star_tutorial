(:*******************************************************:)
(: Test: K-SeqExprCast-796                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:dateTime. :)
(:*******************************************************:)
xs:dateTime("
	 2002-10-10T12:00:00-05:00
	 ")
        eq
        xs:dateTime("
	 2002-10-10T12:00:00-05:00
	 ")
      