(:*******************************************************:)
(: Test: K-NumericSubtract-22                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: A test whose essence is: `string(xs:double("NaN") - 3) eq "NaN"`. :)
(:*******************************************************:)
string(xs:double("NaN") - 3) eq "NaN"