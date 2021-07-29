(:*******************************************************:)
(: Test: K-DateTimeFunc-16                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with the second value's timezone being an arbitrary value. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04"),
			    xs:time("08:05:23+13:07"))
				eq
				xs:dateTime("2004-03-04T08:05:23+13:07")