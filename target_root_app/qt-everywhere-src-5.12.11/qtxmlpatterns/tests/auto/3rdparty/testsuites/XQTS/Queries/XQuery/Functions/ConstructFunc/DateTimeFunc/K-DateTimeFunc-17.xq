(:*******************************************************:)
(: Test: K-DateTimeFunc-17                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() where none of its values has a timezone. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04"),
			    xs:time("08:05:23"))
				eq
				xs:dateTime("2004-03-04T08:05:23")