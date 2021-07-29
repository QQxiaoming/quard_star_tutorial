(:*******************************************************:)
(: Test: K-DateTimeFunc-15                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with the first value's timezone being an arbitrary value. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04+13:07"),
			    xs:time("08:05:23"))
				eq
				xs:dateTime("2004-03-04T08:05:23+13:07")