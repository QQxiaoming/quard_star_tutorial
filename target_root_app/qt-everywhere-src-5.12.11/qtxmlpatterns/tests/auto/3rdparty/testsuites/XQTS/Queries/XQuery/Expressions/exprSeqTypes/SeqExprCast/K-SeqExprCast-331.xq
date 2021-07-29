(:*******************************************************:)
(: Test: K-SeqExprCast-331                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC offset to xs:gMonth. :)
(:*******************************************************:)

		   xs:gMonth(xs:dateTime("2002-11-23T22:12:23.867-13:37")) eq xs:gMonth("--11-13:37")
	