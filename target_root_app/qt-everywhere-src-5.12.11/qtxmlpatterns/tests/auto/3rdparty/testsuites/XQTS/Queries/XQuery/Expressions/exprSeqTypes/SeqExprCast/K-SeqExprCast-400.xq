(:*******************************************************:)
(: Test: K-SeqExprCast-400                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:date with UTC offset to xs:dateTime. :)
(:*******************************************************:)

		   xs:dateTime(xs:date("2002-11-23-13:37"))
		   eq xs:dateTime("2002-11-23T00:00:00.000-13:37")
	