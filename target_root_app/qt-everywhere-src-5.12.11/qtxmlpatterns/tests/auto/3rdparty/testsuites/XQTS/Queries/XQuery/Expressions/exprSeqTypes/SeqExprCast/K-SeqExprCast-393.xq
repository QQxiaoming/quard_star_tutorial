(:*******************************************************:)
(: Test: K-SeqExprCast-393                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:date with UTC timezone to xs:gMonth. :)
(:*******************************************************:)

		   xs:gMonth(xs:date("2002-11-23-00:00")) eq xs:gMonth("--11Z")
	