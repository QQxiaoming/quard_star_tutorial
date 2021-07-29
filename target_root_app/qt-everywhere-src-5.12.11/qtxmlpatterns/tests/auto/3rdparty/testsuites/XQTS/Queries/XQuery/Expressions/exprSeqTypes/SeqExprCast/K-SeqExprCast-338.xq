(:*******************************************************:)
(: Test: K-SeqExprCast-338                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC timezone to xs:time. :)
(:*******************************************************:)

		   xs:time(xs:dateTime("2002-11-23T22:12:23.867-00:00")) eq xs:time("22:12:23.867Z")
	