(:*******************************************************:)
(: Test: K-SeqMAXFunc-33                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(max(xs:float("NaN"))) eq "NaN"`. :)
(:*******************************************************:)
string(max(xs:float("NaN"))) eq "NaN"