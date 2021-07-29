(:*******************************************************:)
(: Test: K-DateTimeFunc-8                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with timezones +00:00 and -00:00. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04-00:00"),
						    xs:time("08:05:23+00:00"))
				   eq
				   xs:dateTime("2004-03-04T08:05:23Z")