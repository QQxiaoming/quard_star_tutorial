(:*******************************************************:)
(: Test: K-NodeNumberFunc-13                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `string(number(xs:anyURI("1"))) eq "NaN"`. :)
(:*******************************************************:)
string(number(xs:anyURI("1"))) eq "NaN"