(:*******************************************************:)
(: Test: K-TimezoneFromDateTimeFunc-5                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `timezone-from-dateTime(xs:dateTime("2004-10-12T23:43:12Z")) eq xs:dayTimeDuration("PT0S")`. :)
(:*******************************************************:)
timezone-from-dateTime(xs:dateTime("2004-10-12T23:43:12Z")) eq
			xs:dayTimeDuration("PT0S")