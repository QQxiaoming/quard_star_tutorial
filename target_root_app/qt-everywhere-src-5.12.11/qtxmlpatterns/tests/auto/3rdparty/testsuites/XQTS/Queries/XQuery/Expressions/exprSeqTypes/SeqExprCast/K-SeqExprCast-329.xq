(:*******************************************************:)
(: Test: K-SeqExprCast-329                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC offset to xs:gYearMonth. :)
(:*******************************************************:)

		   xs:gYearMonth(xs:dateTime("2002-11-23T23:12:23.867-13:37"))
		   eq xs:gYearMonth("2002-11-13:37")
	