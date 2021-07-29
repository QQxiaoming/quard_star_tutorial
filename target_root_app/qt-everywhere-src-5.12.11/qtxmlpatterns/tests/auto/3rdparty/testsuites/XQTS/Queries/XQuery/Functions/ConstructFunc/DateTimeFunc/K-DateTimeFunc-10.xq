(:*******************************************************:)
(: Test: K-DateTimeFunc-10                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with the first value having no timezone. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04"),
						    xs:time("08:05:23-05:00"))
				   eq
				   xs:dateTime("2004-03-04T08:05:23-05:00")