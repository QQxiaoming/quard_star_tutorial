(:*******************************************************:)
(: Test: K-DateTimeFunc-12                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:dateTime() with the time value being 24:00:00. :)
(:*******************************************************:)
xs:string(dateTime(xs:date("1999-12-31"), xs:time("24:00:00")))
						eq "1999-12-31T00:00:00"