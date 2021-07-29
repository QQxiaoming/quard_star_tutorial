(:*******************************************************:)
(: Test: K-SeqAVGFunc-15                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((3, xs:double("NaN"), 3))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((3, xs:double("NaN"), 3))) eq "NaN"