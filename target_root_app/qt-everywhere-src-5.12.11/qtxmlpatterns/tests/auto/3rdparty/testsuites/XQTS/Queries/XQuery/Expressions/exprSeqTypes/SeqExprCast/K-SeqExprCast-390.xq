(:*******************************************************:)
(: Test: K-SeqExprCast-390                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:date with UTC offset to xs:gYear. :)
(:*******************************************************:)

		   xs:gYear(xs:date("2002-11-23-13:37")) eq xs:gYear("2002-13:37")
	