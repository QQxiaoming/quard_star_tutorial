(:*******************************************************:)
(: Test: K-TimezoneFromDateFunc-5                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `timezone-from-date(xs:date("2004-10-12Z")) eq xs:dayTimeDuration("PT0S")`. :)
(:*******************************************************:)
timezone-from-date(xs:date("2004-10-12Z")) eq
			xs:dayTimeDuration("PT0S")