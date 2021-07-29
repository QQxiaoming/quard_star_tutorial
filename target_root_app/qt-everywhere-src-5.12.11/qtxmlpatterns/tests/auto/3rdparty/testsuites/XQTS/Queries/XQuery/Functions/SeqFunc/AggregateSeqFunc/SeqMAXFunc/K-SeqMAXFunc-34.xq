(:*******************************************************:)
(: Test: K-SeqMAXFunc-34                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(max(xs:double("NaN"))) eq "NaN"`. :)
(:*******************************************************:)
string(max(xs:double("NaN"))) eq "NaN"