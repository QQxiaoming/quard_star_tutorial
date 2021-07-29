(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-15                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T00:00:00+01:00"),
						    xs:dayTimeDuration("-PT8H"))
			eq xs:dateTime("2002-03-06T15:00:00-08:00")
		