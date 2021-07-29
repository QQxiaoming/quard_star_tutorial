(:*******************************************************:)
(: Test: K-DateTimeFunc-6                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing different timezones to fn:dateTime() is an error. :)
(:*******************************************************:)
dateTime(xs:date("2004-03-04-00:01"),
							xs:time("08:05:23+00:01"))