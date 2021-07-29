(:*******************************************************:)
(: Test: K-DateTimeFunc-13                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with timezones being canonical UTC('Z'). :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04Z"),
						    xs:time("08:05:23Z"))
				   eq
				   xs:dateTime("2004-03-04T08:05:23Z")