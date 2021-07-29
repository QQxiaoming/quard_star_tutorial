(:*******************************************************:)
(: Test: K-YearMonthDurationMultiply-8                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The multiplication operator is not available between xs:duration and xs:integer. :)
(:*******************************************************:)
xs:duration("P1Y3M") * 3