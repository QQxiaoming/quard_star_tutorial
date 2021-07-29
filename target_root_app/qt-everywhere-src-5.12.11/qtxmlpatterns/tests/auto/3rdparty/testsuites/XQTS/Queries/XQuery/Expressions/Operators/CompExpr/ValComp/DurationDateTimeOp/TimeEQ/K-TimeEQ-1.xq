(:*******************************************************:)
(: Test: K-TimeEQ-1                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:time, returning positive. :)
(:*******************************************************:)
xs:time("23:01:04.12") eq
		              xs:time("23:01:04.12")