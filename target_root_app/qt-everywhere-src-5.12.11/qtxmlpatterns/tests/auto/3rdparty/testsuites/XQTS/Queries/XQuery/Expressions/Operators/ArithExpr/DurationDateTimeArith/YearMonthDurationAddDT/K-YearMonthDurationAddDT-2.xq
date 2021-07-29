(:*******************************************************:)
(: Test: K-YearMonthDurationAddDT-2                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple testing involving operator '+' between xs:yearMonthDuration and xs:dateTime. :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y35M") +
		xs:dateTime("1999-07-19T08:23:01.765") eq xs:dateTime("2005-06-19T08:23:01.765")