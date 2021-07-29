(:*******************************************************:)
(: Test: K-SeqExprCast-392                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:date with UTC offset to xs:gYearMonth. :)
(:*******************************************************:)

		   xs:gYearMonth(xs:date("2002-11-23-13:37"))
		   eq xs:gYearMonth("2002-11-13:37")
	