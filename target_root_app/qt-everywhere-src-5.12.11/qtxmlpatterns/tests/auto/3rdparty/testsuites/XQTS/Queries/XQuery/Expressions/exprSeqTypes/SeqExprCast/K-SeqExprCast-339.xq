(:*******************************************************:)
(: Test: K-SeqExprCast-339                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC offset to xs:time. :)
(:*******************************************************:)

		   xs:time(xs:dateTime("2002-11-23T22:12:23.867-13:37")) eq xs:time("22:12:23.867-13:37")
	