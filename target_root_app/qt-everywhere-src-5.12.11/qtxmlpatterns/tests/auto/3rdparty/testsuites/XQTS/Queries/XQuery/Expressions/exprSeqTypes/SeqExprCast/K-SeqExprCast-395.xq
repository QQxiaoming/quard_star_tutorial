(:*******************************************************:)
(: Test: K-SeqExprCast-395                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:date with UTC timezone to xs:gMonthDay. :)
(:*******************************************************:)

		   xs:gMonthDay(xs:date("2002-11-23-00:00")) eq xs:gMonthDay("--11-23Z")
	