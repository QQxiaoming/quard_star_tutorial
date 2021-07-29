(:*******************************************************:)
(: Test: K-SeqAVGFunc-16                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((xs:double("NaN"), 3, 3))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((xs:double("NaN"), 3, 3))) eq "NaN"