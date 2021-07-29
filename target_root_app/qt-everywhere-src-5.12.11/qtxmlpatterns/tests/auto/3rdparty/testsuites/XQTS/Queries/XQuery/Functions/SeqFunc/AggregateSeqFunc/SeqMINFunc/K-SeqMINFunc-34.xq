(:*******************************************************:)
(: Test: K-SeqMINFunc-34                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(min(xs:double("NaN"))) eq "NaN"`. :)
(:*******************************************************:)
string(min(xs:double("NaN"))) eq "NaN"