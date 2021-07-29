(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-6                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Test that the implicit timezone in the dynamic context is used if $timezone is empty; indirectly also tests context stability. :)
(:*******************************************************:)
timezone-from-dateTime(adjust-dateTime-to-timezone(xs:dateTime("2001-02-03T00:00:00")))
			 eq
			 implicit-timezone()