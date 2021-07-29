(:*******************************************************:)
(: Test: K-SeqExprCast-326                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC timezone to xs:gYear. :)
(:*******************************************************:)

		   xs:gYear(xs:dateTime("2002-11-23T23:12:23.867-00:00")) eq xs:gYear("2002Z")
	